#include <VolumeViz/misc/CvrVoxelChunk.h>
#include <VolumeViz/render/2D/CvrRGBATexture.h>
#include <VolumeViz/render/2D/CvrPaletteTexture.h>
#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <VolumeViz/elements/SoVolumeDataElement.h>
#include <VolumeViz/nodes/SoTransferFunction.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/misc/CvrGIMPGradient.h>
#include <VolumeViz/misc/CvrCLUT.h>
#include <VolumeViz/misc/CvrUtil.h>

#include <../nodes/gradients/GREY.h>
#include <../nodes/gradients/TEMPERATURE.h>
#include <../nodes/gradients/PHYSICS.h>
#include <../nodes/gradients/STANDARD.h>
#include <../nodes/gradients/GLOW.h>
#include <../nodes/gradients/BLUE_RED.h>
#include <../nodes/gradients/SEISMIC.h>

#include <Inventor/C/tidbits.h>
#include <Inventor/C/glue/gl.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/misc/SoState.h>
#include <Inventor/errors/SoDebugError.h>

#include <assert.h>
#include <string.h> // memcpy()

const unsigned int COLOR_TABLE_PREDEF_SIZE = 256;
uint8_t CvrVoxelChunk::PREDEFGRADIENTS[SoTransferFunction::SEISMIC + 1][COLOR_TABLE_PREDEF_SIZE][4];

SbDict * CvrVoxelChunk::CLUTdict = NULL;


// Allocates an uninitialized buffer for storing enough voxel data to
// fit into the given dimensions with space per voxel allocated
// according to the UnitSize type.
//
// If the "buffer" argument is non-NULL, will nto allocate a buffer,
// but rather just use that pointer. It is then the caller's
// responsibility to a) not destruct that buffer before this instance
// is destructed, and b) to deallocate the buffer data.
CvrVoxelChunk::CvrVoxelChunk(const SbVec3s & dimensions, UnitSize type,
                             void * buffer)
{
  assert(dimensions[0] > 0);
  assert(dimensions[1] > 0);
  assert(dimensions[2] > 0);
  assert(type == UINT_8 || type == UINT_16 || type == UINT_32);

  this->dimensions = dimensions;
  this->unitsize = type;

  if (buffer == NULL) {
    this->voxelbuffer = new uint8_t[this->bufferSize()];
    this->destructbuffer = TRUE;
  }
  else {
    this->voxelbuffer = buffer;
    this->destructbuffer = FALSE;
  }

  static SbBool init_static = TRUE;
  if (init_static) {
    init_static = FALSE;
    CvrVoxelChunk::initPredefGradients();
  }
}

CvrVoxelChunk::~CvrVoxelChunk()
{
  if (this->destructbuffer) { delete[] (uint8_t *)this->voxelbuffer; }
}

// Number of bytes in buffer.
unsigned int
CvrVoxelChunk::bufferSize(void) const
{
  return
    this->dimensions[0] * this->dimensions[1] * this->dimensions[2] *
    this->unitsize;
}

// Returns the buffer start pointer.
void *
CvrVoxelChunk::getBuffer(void) const
{
  return this->voxelbuffer;
}

// Returns the buffer start pointer. Convenience method to return the
// pointer casted to the correct size. Don't use this method unless
// the unitsize type is UINT_8.
uint8_t *
CvrVoxelChunk::getBuffer8(void) const
{
  assert(this->unitsize == UINT_8);
  return (uint8_t *)this->voxelbuffer;
}

// Returns the buffer start pointer. Convenience method to return the
// pointer casted to the correct size. Don't use this method unless
// the unitsize type is UINT_16.
uint16_t *
CvrVoxelChunk::getBuffer16(void) const
{
  assert(this->unitsize == UINT_16);
  return (uint16_t *)this->voxelbuffer;
}

// Returns the buffer start pointer. Convenience method to return the
// pointer casted to the correct size. Don't use this method unless
// the unitsize type is UINT_32.
uint32_t *
CvrVoxelChunk::getBuffer32(void) const
{
  assert(this->unitsize == UINT_32);
  return (uint32_t *)this->voxelbuffer;
}

const SbVec3s &
CvrVoxelChunk::getDimensions(void) const
{
  return this->dimensions;
}

CvrVoxelChunk::UnitSize
CvrVoxelChunk::getUnitSize(void) const
{
  return this->unitsize;
}

// Converts the transferfunction's colormap into a CvrCLUT object.
CvrCLUT *
CvrVoxelChunk::makeCLUT(const SoTransferFunctionElement * tfelement)
{
  SoTransferFunction * transferfunc = tfelement->getTransferFunction();
  assert(transferfunc != NULL);

  const int predefmapidx = transferfunc->predefColorMap.getValue();
  assert(predefmapidx >= SoTransferFunction::NONE);
  assert(predefmapidx <= SoTransferFunction::SEISMIC);

  CvrCLUT * clut = NULL;
  unsigned int nrcols = 0;

  if (predefmapidx != SoTransferFunction::NONE) {
    uint8_t * predefmap = &(CvrVoxelChunk::PREDEFGRADIENTS[predefmapidx][0][0]);
    nrcols = COLOR_TABLE_PREDEF_SIZE;
    clut = new CvrCLUT(nrcols, predefmap);
  }
  else {
    const float * colormap = transferfunc->colorMap.getValues(0);
    const int colormaptype = transferfunc->colorMapType.getValue();

    unsigned int nrcomponents = 0;
    switch (colormaptype) {
    case SoTransferFunction::ALPHA: nrcomponents = 1; break;
    case SoTransferFunction::LUM_ALPHA: nrcomponents = 2; break;
    case SoTransferFunction::RGBA: nrcomponents = 4; break;
    default: assert(FALSE && "invalid SoTransferFunction::colorMapType value"); break;
    }

    nrcols = transferfunc->colorMap.getNum() / nrcomponents;
    assert((transferfunc->colorMap.getNum() % nrcomponents) == 0);

    clut = new CvrCLUT(nrcols, nrcomponents, colormap);
  }

  uint32_t transparencythresholds[2];
  tfelement->getTransparencyThresholds(transparencythresholds[0],
                                       transparencythresholds[1]);
  clut->setTransparencyThresholds(transparencythresholds[0],
                                  SbMin(nrcols - 1, transparencythresholds[1]));
  return clut;
}

// Fetch a CLUT that represents the current
// SoTransferFunction. Facilitates sharing of palettes.
CvrCLUT *
CvrVoxelChunk::getCLUT(const SoTransferFunctionElement * tfelement)
{
  if (!CvrVoxelChunk::CLUTdict) {
    // FIXME: dealloc at exit
    CvrVoxelChunk::CLUTdict = new SbDict;
  }

  SoTransferFunction * transferfunc = tfelement->getTransferFunction();
  assert(transferfunc != NULL);

  void * clutvoidptr;
  CvrCLUT * clut;
  if (CvrVoxelChunk::CLUTdict->find(transferfunc->getNodeId(), clutvoidptr)) {
    clut = (CvrCLUT *)clutvoidptr;
  }
  else {
    clut = CvrVoxelChunk::makeCLUT(tfelement);
    // FIXME: ref(), or else we'd get dangling pointers to destructed
    // CvrCLUT entries in the dict. Should provide a "destructing now"
    // callback on the CvrCLUT class to clean up the design, as this
    // is a resource leak as it now stands.
    clut->ref();
    SbBool r = CvrVoxelChunk::CLUTdict->enter(transferfunc->getNodeId(), clut);
    assert(r == TRUE && "clut should not exist on nodeid");
  }

  return clut;
}

SbBool
CvrVoxelChunk::usePaletteTextures(SoGLRenderAction * action)
{
  SoState * state = action->getState();
  const SoVolumeDataElement * vdelement = SoVolumeDataElement::getInstance(state);
  assert(vdelement != NULL);
  const SoVolumeData * volumedatanode = vdelement->getVolumeData();
  assert(volumedatanode != NULL);

  const cc_glglue * glw = cc_glglue_instance(action->getCacheContext());

  // Check if paletted textures is wanted by the app programmer.
  SbBool usepalettetex = volumedatanode->usePalettedTexture.getValue();

  const char * env;
  static int force_paletted = -1; // "-1" means "undecided"
  static int force_rgba = -1;
  if (force_paletted == -1) { force_paletted = (env = coin_getenv("CVR_FORCE_PALETTED_TEXTURES")) && (atoi(env) > 0); }
  if (force_rgba == -1) { force_rgba = (env = coin_getenv("CVR_FORCE_RGBA_TEXTURES")) && (atoi(env) > 0); }
  assert(!(force_paletted && force_rgba)); // both at the same time can't be done, silly

  if (force_paletted) usepalettetex = TRUE;
  if (force_rgba) usepalettetex = FALSE;

  // If we requested paletted textures, does OpenGL support them?
  // (FIXME: one more thing to check versus OpenGL is that the
  // palette size can fit.)
  usepalettetex = usepalettetex && cc_glglue_has_paletted_textures(glw);

  static SbBool first = TRUE;
  if (first && CvrUtil::doDebugging()) {
    SoDebugError::postInfo("CvrVoxelChunk::usePaletteTextures",
                           "returns %s", usepalettetex ? "TRUE" : "FALSE");
    first = FALSE;
  }

  return usepalettetex;
}

/*!
  Transfers voxel data to a texture.

  The "invisible" flag will be set according to whether or not there's
  at least one texel that's not fully transparent.
*/
CvrTextureObject *
CvrVoxelChunk::transfer(SoGLRenderAction * action, SbBool & invisible) const
{
  // FIXME: about the "invisible" flag: this should really be an
  // SbBox2s that indicates which part of the output buffer is
  // non-fully-transparent. This could be used to optimize texture
  // rendering. 20021201 mortene.

  // FIXME: in addition to the "invisible" area, we could keep an
  // "opaqueness" area, to make it possible to optimize rendering by
  // occlusion culling. 20021201 mortene.


  SoState * state = action->getState();
  const SoTransferFunctionElement * tfelement = SoTransferFunctionElement::getInstance(state);
  assert(tfelement != NULL);
  const SoTransferFunction * transferfunc = tfelement->getTransferFunction();
  assert(transferfunc != NULL);

  // FIXME: only handles 2D textures yet. 20021203 mortene.
  assert(this->getDimensions()[2] == 1);

  const SbVec2s size(this->getDimensions()[0], this->getDimensions()[1]);

  const SbVec2s texsize(coin_next_power_of_two(size[0] - 1),
                        coin_next_power_of_two(size[1] - 1));

  invisible = TRUE;

  CvrTextureObject * texobj = NULL;

  // Handling RGBA inputdata. Just forwarding to output.
  if (this->getUnitSize() == CvrVoxelChunk::UINT_32) {
    CvrRGBATexture * rgbatex = new CvrRGBATexture(texsize);
    texobj = rgbatex;
    uint32_t * output = rgbatex->getRGBABuffer();

    // FIXME: we should really have support routines for converting
    // "raw" RGBA inputdata into paletted data

    for (unsigned int y = 0; y < (unsigned int)size[1]; y++) {
      (void)memcpy(&output[texsize[0] * y],
                   &(this->getBuffer32()[size[0] * y]),
                   size[0] * sizeof(uint32_t));
    }

    // FIXME: set the "invisible" flag correctly according to actual
    // input. 20021129 mortene.
    invisible = FALSE;
  }

  else if (this->getUnitSize() == CvrVoxelChunk::UINT_8) {
    CvrRGBATexture * rgbatex = NULL;
    CvrPaletteTexture * palettetex = NULL;

    const CvrCLUT * clut = CvrVoxelChunk::getCLUT(tfelement);
    clut->ref();

    SbBool usepalettetex = CvrVoxelChunk::usePaletteTextures(action);

    if (usepalettetex) {
      palettetex = new CvrPaletteTexture(texsize);
      palettetex->setCLUT(clut);
      texobj = palettetex;
    }
    else {
      rgbatex = new CvrRGBATexture(texsize);
      texobj = rgbatex;
    }

    const int32_t shiftval = transferfunc->shift.getValue();
    const int32_t offsetval = transferfunc->offset.getValue();

    const uint8_t * inputbytebuffer = this->getBuffer8();

    if (palettetex) { // paletted texture
      uint8_t * output = palettetex->getIndex8Buffer();

      for (unsigned int y=0; y < (unsigned int)size[1]; y++) {
        for (unsigned int x=0; x < (unsigned int)size[0]; x++) {
          const int voxelidx = y * size[0] + x;
          const int texelidx = y * texsize[0] + x;
          const uint8_t voldataidx = inputbytebuffer[voxelidx];
          const uint8_t colidx = (voldataidx << shiftval) + offsetval;

          output[texelidx] = colidx;
        }
      }

      // FIXME: should set the ''invisible'' flag correctly to
      // optimize the amount of the available fill-rate of the gfx
      // card we're using.
      //
      // Note that it's not straightforward to use this optimalization
      // for paletted textures, because we want to be able to change
      // the palette on the fly without having to regenerate texture
      // blocks / slices (i.e.: something that _was_ invisible could
      // become visible upon changing the palette, and vice versa).
      //
      // Should still fix it, though, as it can have a _major_
      // impact. Try for instance the 3DHEAD.VOL set in RGBA texture
      // mode versus palette texture mode -- the former has ~ 2X-3X
      // better framerate.
      invisible = FALSE;
    }
    else { // RGBA texture
      uint32_t * rgbaquadruplets = rgbatex->getRGBABuffer();
      uint8_t * output = (uint8_t *)rgbaquadruplets;

      for (unsigned int y=0; y < (unsigned int)size[1]; y++) {
        for (unsigned int x=0; x < (unsigned int)size[0]; x++) {
          const int texelidx = y * texsize[0] + x;
          const int voxelidx = y * size[0] + x;
          const uint8_t voldataidx = inputbytebuffer[voxelidx];
          const uint8_t colidx = (voldataidx << shiftval) + offsetval;

          clut->lookupRGBA(colidx, &output[texelidx * 4]);
          invisible = invisible && (output[texelidx * 4 + 3] == 0x00);
        }
      }
    }
    clut->unref();
  }

  else if (this->getUnitSize() == CvrVoxelChunk::UINT_16) {
    assert(FALSE && "not yet implemented");
  }

  else {
    assert(FALSE && "unknown unit size");
  }

  return texobj;
}

// Initialize all predefined colormaps.
void
CvrVoxelChunk::initPredefGradients(void)
{
  const char * gradientbufs[] = {
    GREY_gradient, TEMPERATURE_gradient, PHYSICS_gradient,
    STANDARD_gradient, GLOW_gradient, BLUE_RED_gradient, SEISMIC_gradient
  };

  for (unsigned int j=0; j < sizeof(gradientbufs)/sizeof(gradientbufs[0]); j++) {
    CvrGIMPGradient * gg = CvrGIMPGradient::read(gradientbufs[j]);
    gg->convertToIntArray(CvrVoxelChunk::PREDEFGRADIENTS[j+1]);

#if 0 // DEBUG: spits out a COLOR_TABLE_PREDEF_SIZEx1 image of the colormap.
    SbString s;
    s.sprintf("/tmp/gradient-%d.ppm", j + 1);
    FILE * f = fopen(s.getString(), "w");
    assert(f);
    // width height maxcolval
    (void)fprintf(f, "P3\n%d 1 255\n", COLOR_TABLE_PREDEF_SIZE);

    for (unsigned int i=0; i < COLOR_TABLE_PREDEF_SIZE; i++) {
      fprintf(f, "%d %d %d\n",
              CvrVoxelChunk::PREDEFGRADIENTS[TEMPERATURE][i][0],
              CvrVoxelChunk::PREDEFGRADIENTS[TEMPERATURE][i][1],
              CvrVoxelChunk::PREDEFGRADIENTS[TEMPERATURE][i][2]);
    }
    fclose(f);
#endif // DEBUG
  }
}

// For debugging purposes.
void
CvrVoxelChunk::dumpToPPM(const char * filename) const
{
  // FIXME: only handles UINT_8 type chunks yet.

  uint8_t * slicebuf = (uint8_t *)this->getBuffer();
  FILE * f = fopen(filename, "w");
  assert(f);
  (void)fprintf(f, "P2\n%d %d 255\n",  // width height maxcolval
                this->getDimensions()[0], this->getDimensions()[1]);

  const int nrvoxels = this->getDimensions()[0] * this->getDimensions()[1];
  for (int i=0; i < nrvoxels; i++) {
    (void)fprintf(f, "%d\n", slicebuf[i]);
  }
  (void)fclose(f);
}


// Cut a slice along any principal axis, of a single image depth.
CvrVoxelChunk *
CvrVoxelChunk::buildSubPage(const unsigned int axisidx, const int pageidx,
                            const SbBox2s & cutslice)
{
  CvrVoxelChunk * output = NULL;
  switch (axisidx) {
  case 0:
    output = this->buildSubPageX(pageidx, cutslice);
    break;

  case 1:
    output = this->buildSubPageY(pageidx, cutslice);
    break;

  case 2:
    output = this->buildSubPageZ(pageidx, cutslice);
    break;

  default:
    assert(FALSE);
    break;
  }
  return output;
}


// Copies rows of z-axis data down the y-axis.
CvrVoxelChunk *
CvrVoxelChunk::buildSubPageX(const int pageidx, // FIXME: get rid of this by using an SbBox3s for cutslice. 20021203 mortene.
                             const SbBox2s & cutslice)
{
  assert(pageidx >= 0);
  assert(pageidx < this->getDimensions()[0]);

  SbVec2s ssmin, ssmax;
  cutslice.getBounds(ssmin, ssmax);

  const SbVec3s dim = this->getDimensions();

  const int zAdd = dim[0] * dim[1];

  const unsigned int nrhorizvoxels = ssmax[0] - ssmin[0];
  assert(nrhorizvoxels > 0);
  const unsigned int nrvertvoxels = ssmax[1] - ssmin[1];
  assert(nrvertvoxels > 0);

  const SbVec3s outputdims(nrhorizvoxels, nrvertvoxels, 1);
  CvrVoxelChunk * output = new CvrVoxelChunk(outputdims, this->getUnitSize());

  const unsigned int staticoffset =
    pageidx + ssmin[1] * dim[0] + ssmin[0] * zAdd;

  const unsigned int voxelsize = this->getUnitSize();
  uint8_t * inputbytebuffer = (uint8_t *)this->getBuffer();
  uint8_t * outputbytebuffer = (uint8_t *)output->getBuffer();

  for (unsigned int rowidx = 0; rowidx < nrvertvoxels; rowidx++) {
    const unsigned int inoffset = staticoffset + (rowidx * dim[0]);
    const uint8_t * srcptr = &(inputbytebuffer[inoffset * voxelsize]);
    uint8_t * dstptr = &(outputbytebuffer[nrhorizvoxels * rowidx * voxelsize]);

    // FIXME: should optimize this loop. 20021125 mortene.
    for (unsigned int horizidx = 0; horizidx < nrhorizvoxels; horizidx++) {
      *dstptr++ = *srcptr++;
      if (voxelsize > 1) *dstptr++ = *srcptr++;
      if (voxelsize == 4) { *dstptr++ = *srcptr++; *dstptr++ = *srcptr++; }

      srcptr += zAdd * voxelsize - voxelsize;
    }
  }

  return output;
}

// Copies rows of x-axis data along the z-axis.
/*
  Here's how a 4x3 slice would be cut, from the memory layout:

  +----------------+
  |                |
  |    xxxx        |
  |                |
  |                |
  |                |
  +----------------+
  |                |
  |    xxxx        |
  |                |
  |                |
  |                |
  +----------------+
  |                |
  |    xxxx        |
  |                |
  |                |
  |                |
  +----------------+

  Cutting is done top-to-bottom, and is placed in the output slice
  buffer left-to-right, top-to-bottom.
 */
CvrVoxelChunk *
CvrVoxelChunk::buildSubPageY(const int pageidx, // FIXME: get rid of this by using an SbBox3s for cutslice. 20021203 mortene.
                             const SbBox2s & cutslice)
{
  assert(pageidx >= 0);
  assert(pageidx < this->getDimensions()[1]);

  SbVec2s ssmin, ssmax;
  cutslice.getBounds(ssmin, ssmax);

  const SbVec3s dim = this->getDimensions();

  const unsigned int nrhorizvoxels = ssmax[0] - ssmin[0];
  assert(nrhorizvoxels > 0);
  const unsigned int nrvertvoxels = ssmax[1] - ssmin[1];
  assert(nrvertvoxels > 0);

  const SbVec3s outputdims(nrhorizvoxels, nrvertvoxels, 1);
  CvrVoxelChunk * output = new CvrVoxelChunk(outputdims, this->getUnitSize());

  const unsigned int staticoffset =
    (ssmin[1] * dim[0] * dim[1]) + (pageidx * dim[0]) + ssmin[0];

  const unsigned int voxelsize = this->getUnitSize();
  uint8_t * inputbytebuffer = (uint8_t *)this->getBuffer();
  uint8_t * outputbytebuffer = (uint8_t *)output->getBuffer();

  for (unsigned int rowidx = 0; rowidx < nrvertvoxels; rowidx++) {
    const unsigned int inoffset = staticoffset + (rowidx * dim[0] * dim[1]);
    const uint8_t * srcptr = &(inputbytebuffer[inoffset * voxelsize]);

    uint8_t * dstptr = &(outputbytebuffer[nrhorizvoxels * rowidx * voxelsize]);

    (void)memcpy(dstptr, srcptr, nrhorizvoxels * voxelsize);
  }

  return output;
}

// Copies rows of x-axis data down the y-axis.
CvrVoxelChunk *
CvrVoxelChunk::buildSubPageZ(const int pageidx, // FIXME: get rid of this by using an SbBox3s for cutslice. 20021203 mortene.
                             const SbBox2s & cutslice)
{
  assert(pageidx >= 0);
  assert(pageidx < this->getDimensions()[2]);

  SbVec2s ssmin, ssmax;
  cutslice.getBounds(ssmin, ssmax);

  const SbVec3s dim = this->getDimensions();

  const unsigned int nrhorizvoxels = ssmax[0] - ssmin[0];
  assert(nrhorizvoxels > 0);
  const unsigned int nrvertvoxels = ssmax[1] - ssmin[1];
  assert(nrvertvoxels > 0);

  const SbVec3s outputdims(nrhorizvoxels, nrvertvoxels, 1);
  CvrVoxelChunk * output = new CvrVoxelChunk(outputdims, this->getUnitSize());

  const unsigned int staticoffset =
    (pageidx * dim[0] * dim[1]) + (ssmin[1] * dim[0]) + ssmin[0];

  const unsigned int voxelsize = this->getUnitSize();
  uint8_t * inputbytebuffer = (uint8_t *)this->getBuffer();
  uint8_t * outputbytebuffer = (uint8_t *)output->getBuffer();

  for (unsigned int rowidx = 0; rowidx < nrvertvoxels; rowidx++) {
    const unsigned int inoffset = staticoffset + (rowidx * dim[0]);
    const uint8_t * srcptr = &(inputbytebuffer[inoffset * voxelsize]);

    uint8_t * dstptr = &(outputbytebuffer[nrhorizvoxels * rowidx * voxelsize]);

    (void)memcpy(dstptr, srcptr, nrhorizvoxels * voxelsize);
  }

  return output;
}
