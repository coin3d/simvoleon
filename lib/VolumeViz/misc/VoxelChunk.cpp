#include <VolumeViz/misc/CvrVoxelChunk.h>
#include <VolumeViz/render/2D/CvrTextureObject.h>
#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <VolumeViz/nodes/SoTransferFunction.h>
#include <VolumeViz/misc/CvrGIMPGradient.h>

#include <../nodes/gradients/GREY.h>
#include <../nodes/gradients/TEMPERATURE.h>
#include <../nodes/gradients/PHYSICS.h>
#include <../nodes/gradients/STANDARD.h>
#include <../nodes/gradients/GLOW.h>
#include <../nodes/gradients/BLUE_RED.h>
#include <../nodes/gradients/SEISMIC.h>

#include <Inventor/C/tidbits.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/misc/SoState.h>

#include <assert.h>

uint32_t CvrVoxelChunk::transfertable[256];
SbBool CvrVoxelChunk::transferdone[256];
uint32_t CvrVoxelChunk::transfertablenodeid;

uint8_t CvrVoxelChunk::PREDEFGRADIENTS[SoTransferFunction::SEISMIC + 1][256][4];


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

  // Make sure this is set to an unused value, so it gets initialized
  // at first invocation.
  CvrVoxelChunk::transfertablenodeid = SoNode::getNextNodeId();

  static SbBool init_gradients = TRUE;
  if (init_gradients) {
    CvrVoxelChunk::initPredefGradients();
    init_gradients = FALSE;
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

static inline uint32_t
compileRGBALittleEndian(const uint8_t red, const uint8_t green,
                        const uint8_t blue, const uint8_t opaqueness)
{
  return
    (uint32_t(red) << 0) | (uint32_t(green) << 8) |
    (uint32_t(blue) << 16) | (uint32_t(opaqueness) << 24);
}

static inline uint32_t
compileRGBABigEndian(const uint8_t red, const uint8_t green,
                     const uint8_t blue, const uint8_t opaqueness)
{
  return
    (uint32_t(red) << 24) | (uint32_t(green) << 16) |
    (uint32_t(blue) << 8) | (uint32_t(opaqueness) << 0);
}

// Transfers voxel data from input buffer to output buffer, according
// to specified parameters.
//
// Output buffer is allocated within the function.
//
// The "invisible" flag will be set according to whether or not
// there's at least one texel that's not fully transparent.
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
  SoTransferFunction * transferfunc = tfelement->getTransferFunction();
  assert(transferfunc != NULL);

  // This table needs to be invalidated when any parameter of the
  // SoTransferFunction node changes.
  if (CvrVoxelChunk::transfertablenodeid != transferfunc->getNodeId()) {
    CvrVoxelChunk::blankoutTransferTable();
    CvrVoxelChunk::transfertablenodeid = transferfunc->getNodeId();
  }

  static int endianness = COIN_HOST_IS_UNKNOWNENDIAN;
  if (endianness == COIN_HOST_IS_UNKNOWNENDIAN) {
    endianness = coin_host_get_endianness();
    assert(endianness != COIN_HOST_IS_UNKNOWNENDIAN && "weird hardware!");
  }

  // FIXME: only handles 2D textures yet. 20021203 mortene.
  assert(this->getDimensions()[2] == 1);

  const SbVec2s size(this->getDimensions()[0], this->getDimensions()[1]);

  const SbVec2s texsize(coin_next_power_of_two(size[0] - 1),
                        coin_next_power_of_two(size[1] - 1));
  CvrTextureObject * texobj = new CvrTextureObject(texsize);

  uint32_t * output = texobj->getRGBABuffer();

  invisible = TRUE;

  // Handling RGBA inputdata. Just forwarding to output
  if (this->getUnitSize() == CvrVoxelChunk::UINT_32) {
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
    int colormaptype = transferfunc->colorMapType.getValue();
    int nrcomponents = 1;
    switch (colormaptype) {
    case SoTransferFunction::ALPHA: nrcomponents = 1; break;
    case SoTransferFunction::LUM_ALPHA: nrcomponents = 2; break;
    case SoTransferFunction::RGBA: nrcomponents = 4; break;
    default: assert(FALSE && "invalid SoTransferFunction::colorMapType value"); break;
    }

    const float * colormap = transferfunc->colorMap.getValues(0);
    int nrcols = transferfunc->colorMap.getNum() / nrcomponents;
    // FIXME: a bit strict this, should warn instead. 20021119 mortene.
    assert((transferfunc->colorMap.getNum() % nrcomponents) == 0);

    uint8_t * predefmap = NULL;
    const int predefmapidx = transferfunc->predefColorMap.getValue();
    assert(predefmapidx >= SoTransferFunction::NONE);
    assert(predefmapidx <= SoTransferFunction::SEISMIC);

    if (predefmapidx != SoTransferFunction::NONE) {
      predefmap = &(CvrVoxelChunk::PREDEFGRADIENTS[predefmapidx][0][0]);
    }

    int32_t shiftval = transferfunc->shift.getValue();
    int32_t offsetval = transferfunc->offset.getValue();

    uint32_t transparencythresholds[2];
    tfelement->getTransparencyThresholds(transparencythresholds[0],
                                         transparencythresholds[1]);

    const uint8_t * inputbytebuffer = this->getBuffer8();

    for (unsigned int y=0; y < (unsigned int)size[1]; y++) {
      for (unsigned int x=0; x < (unsigned int)size[0]; x++) {

        const uint8_t voldataidx = inputbytebuffer[y * size[0] + x];
        const int texelidx = y * texsize[0] + x;

        if (CvrVoxelChunk::transferdone[voldataidx]) {
          output[texelidx] = CvrVoxelChunk::transfertable[voldataidx];

          uint8_t alpha = (endianness == COIN_HOST_IS_LITTLEENDIAN) ?
            ((output[texelidx] & 0xff000000) > 24) : (output[texelidx] & 0x000000ff);
          invisible = invisible && (alpha == 0x00);
          continue;
        }

        uint8_t inval = voldataidx << shiftval;
        inval += offsetval;

        if ((inval == 0x00) || // FIXME: not sure about this.. 20021119 mortene.
            (inval < transparencythresholds[0]) ||
            (inval > transparencythresholds[1])) {
          output[texelidx] = 0x00000000;
        }
        else {
          uint8_t rgba[4];

          if (predefmapidx == SoTransferFunction::NONE) {
            assert(inval < nrcols);
            const float * colvals = &(colormap[inval * nrcomponents]);
#if CVR_DEBUG
            // Done for robustness. Costly, though, so should probably remove.
            for (int cvchk = 0; cvchk < nrcomponents; cvchk++) {
              assert(colvals[cvchk] >= 0.0f && colvals[cvchk] <= 1.0f);
            }
#endif // CVR_DEBUG
            switch (colormaptype) {
            case SoTransferFunction::ALPHA:
              rgba[0] = rgba[1] = rgba[2] = rgba[3] = uint8_t(colvals[0] * 255.0f);
              break;

            case SoTransferFunction::LUM_ALPHA:
              rgba[0] = rgba[1] = rgba[2] = uint8_t(colvals[0] * 255.0f);
              rgba[3] = uint8_t(colvals[1] * 255.0f);
              break;

            case SoTransferFunction::RGBA:
              rgba[0] = uint8_t(colvals[0] * 255.0f);
              rgba[1] = uint8_t(colvals[1] * 255.0f);
              rgba[2] = uint8_t(colvals[2] * 255.0f);
              rgba[3] = uint8_t(colvals[3] * 255.0f);
              break;

            default:
              assert(FALSE && "impossible");
              break;
            }
          }
          else { // using one of the predefined colormaps
            rgba[0] = predefmap[inval * 4 + 0];
            rgba[1] = predefmap[inval * 4 + 1];
            rgba[2] = predefmap[inval * 4 + 2];
            rgba[3] = predefmap[inval * 4 + 3];
          }

          output[texelidx] = (endianness == COIN_HOST_IS_LITTLEENDIAN) ?
            compileRGBALittleEndian(rgba[0], rgba[1], rgba[2], rgba[3]) :
            compileRGBABigEndian(rgba[0], rgba[1], rgba[2], rgba[3]);

          invisible = invisible && (rgba[3] == 0x00);
        }
      
        CvrVoxelChunk::transferdone[voldataidx] = TRUE;
        CvrVoxelChunk::transfertable[voldataidx] = output[texelidx];
      }
    }
  }

  else if (this->getUnitSize() == CvrVoxelChunk::UINT_16) {
    assert(FALSE && "not yet implemented");
  }

  else {
    assert(FALSE && "unknown unit size");
  }

  return texobj;
}

void
CvrVoxelChunk::blankoutTransferTable(void)
{
  for (unsigned int i=0; i < 256; i++) {
    CvrVoxelChunk::transferdone[i] = FALSE;
  }
}

// Initialize all 256-value predefined colormaps.
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

#if 0 // DEBUG: spits out a 256x1 image of the colormap.
    SbString s;
    s.sprintf("/tmp/gradient-%d.ppm", j + 1);
    FILE * f = fopen(s.getString(), "w");
    assert(f);
    (void)fprintf(f, "P3\n256 1 255\n"); // width height maxcolval

    for (int i=0; i < 256; i++) {
      fprintf(f, "%d %d %d\n",
              CvrVoxelChunk::PREDEFGRADIENTS[TEMPERATURE][i][0],
              CvrVoxelChunk::PREDEFGRADIENTS[TEMPERATURE][i][1],
              CvrVoxelChunk::PREDEFGRADIENTS[TEMPERATURE][i][2]);
    }
    fclose(f);
#endif // DEBUG
  }
}
