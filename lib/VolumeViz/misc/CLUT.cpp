// This datatype is a color lookup table.

#include <VolumeViz/misc/CvrCLUT.h>
#include <assert.h>
#include <stdlib.h>
#include <Inventor/SbBasic.h>
#include <Inventor/errors/SoDebugError.h>


// colormap values are between 0 and 255
CvrCLUT::CvrCLUT(const unsigned int nrcols, const uint8_t * colormap)
{
  this->nrentries = nrcols;
  this->nrcomponents = 4;
  this->datatype = INTS;

  const int blocksize = this->nrentries * this->nrcomponents;
  this->int_entries = new uint8_t[blocksize];
  (void)memcpy(this->int_entries, colormap, blocksize * sizeof(uint8_t));
  // (yeah, yeah, I know sizeof(uint8_t) is always == 1)

  this->commonConstructor();
}

// nrcomponents == 1: LUMINANCE
// nrcomponents == 2: LUMINANCE + ALPHA
// nrcomponents == 4: RGBA
//
// values in colormap are between 0.0 and 1.0
CvrCLUT::CvrCLUT(const unsigned int nrcols, const unsigned int nrcomponents,
                 const float * colormap)
{
  this->nrentries = nrcols;
  this->nrcomponents = nrcomponents;
  this->datatype = FLOATS;

  const int blocksize = this->nrentries * this->nrcomponents;
  this->flt_entries = new float[blocksize];
  (void)memcpy(this->int_entries, colormap, blocksize * sizeof(float));

  this->commonConstructor();
}

void
CvrCLUT::commonConstructor(void)
{
  this->refcount = 0;

  this->transparencythresholds[0] = 0;
  this->transparencythresholds[1] = this->nrentries - 1;

  // This is the block of fully transparent color values used by the
  // glColorSubTable() calls which implements the transparency
  // thresholds. It will be dynamically grown according to invocations
  // of setTransparencyThresholds().
  this->transparentblock = NULL;
  this->transparentblockentries = 0;
}

CvrCLUT::~CvrCLUT()
{
  if (this->datatype == INTS)
    delete[] this->int_entries;
  if (this->datatype == FLOATS)
    delete[] this->flt_entries;

  delete[] this->transparentblock;
}

void
CvrCLUT::ref(void) const
{
  CvrCLUT * that = (CvrCLUT *)this; // cast away constness
  that->refcount++;
}

void
CvrCLUT::unref(void) const
{
  CvrCLUT * that = (CvrCLUT *)this; // cast away constness
  that->refcount--;
  assert(this->refcount >= 0);
  if (this->refcount == 0) delete this;
}

int32_t
CvrCLUT::getRefCount(void) const
{
  return this->refcount;
}

// Everything below "low" (but not including) and above "high" (but
// not including), will be fully transparent.
void
CvrCLUT::setTransparencyThresholds(uint32_t low, uint32_t high)
{
  assert(low <= high);
  assert(low < this->nrentries);
  assert(high < this->nrentries);

  this->transparencythresholds[0] = low;
  this->transparencythresholds[1] = high;

  const int max_empty_entries = SbMax(low, this->nrentries - high - 1);
  if (this->transparentblockentries < max_empty_entries) {
    delete[] this->transparentblock;
    uint8_t * ptr = this->transparentblock =
      new uint8_t[max_empty_entries * 4];
    this->transparentblockentries = max_empty_entries;
    for (int i=0; i < this->transparentblockentries * 4; i++) { *ptr++ = 0x00; }
  } 
}

void
CvrCLUT::activate(const cc_glglue * glw) const
{
  // FIXME: should only need to do this once somewhere else
  glEnable(GL_COLOR_TABLE);

  // FIXME: handle this->nrentries != 256
  assert(this->nrentries == 256);

  void * dataptr = NULL;
  GLenum format, type;

  if (this->datatype == INTS) {
    dataptr = this->int_entries;
    type = GL_UNSIGNED_BYTE;
    format = GL_RGBA;
  }
  else if (this->datatype == FLOATS) {
    dataptr = this->flt_entries;
    type = GL_FLOAT;
    switch (this->nrcomponents) {
    case 1: format = GL_LUMINANCE; break;
    case 2: format = GL_LUMINANCE_ALPHA; break;
    case 4: format = GL_RGBA; break;
    default: assert(FALSE); break;
    }
  }
  else assert(FALSE);

  // FIXME: should probably set glColorTableParameter() on
  // GL_COLOR_TABLE_SCALE and GL_COLOR_TABLE_BIAS.

  // FIXME: should probably check if all is ok by using
  // PROXY_TEXTURE_2D first.

  cc_glglue_glColorTable(glw,
                         GL_TEXTURE_2D, /* target */
                         GL_RGBA, /* GL internalformat */
                         this->nrentries, /* nr of paletteentries */
                         format, /* palette entry format */
                         type, /* palette entry unit type */
                         dataptr); /* data ptr */

  // Sanity check.
  assert(glGetError() == GL_NO_ERROR);

  // Sanity check.
  GLint actualsize;
  cc_glglue_glGetColorTableParameteriv(glw, GL_TEXTURE_2D,
                                       GL_COLOR_TABLE_WIDTH, &actualsize);
  assert(actualsize == (GLint)this->nrentries);


  // Set up transparent areas of the colortable.

  uint32_t low = this->transparencythresholds[0];
  uint32_t high = this->transparencythresholds[1];
  uint32_t numtrailing = this->nrentries - high - 1;

  if (low > 0) {
    cc_glglue_glColorSubTable(glw,
                              GL_TEXTURE_2D, /* target */
                              0, /* start */
                              low, /* count */
                              GL_RGBA, /* palette entry format */
                              GL_UNSIGNED_BYTE, /* palette entry unit type */
                              this->transparentblock); /* data ptr */
  }

  if (high < (this->nrentries - 1)) {
    cc_glglue_glColorSubTable(glw,
                              GL_TEXTURE_2D, /* target */
                              high + 1, /* start */
                              numtrailing, /* count */
                              GL_RGBA, /* palette entry format */
                              GL_UNSIGNED_BYTE, /* palette entry unit type */
                              this->transparentblock); /* data ptr */
  }

  assert(glGetError() == GL_NO_ERROR);
}

// Find RGBA color at the given idx.
void
CvrCLUT::lookupRGBA(const unsigned int idx, uint8_t rgba[4]) const
{
  if ((idx < this->transparencythresholds[0]) ||
      (idx > this->transparencythresholds[1])) {
    rgba[0] = 0x00; rgba[1] = 0x00; rgba[2] = 0x00; rgba[3] = 0x00;
    return;
  }

  if (this->datatype == FLOATS) {
    assert(idx < this->nrentries);
    const float * colvals = &(this->flt_entries[idx * this->nrcomponents]);
    switch (this->nrcomponents) {
    case 1: // ALPHA
      rgba[0] = rgba[1] = rgba[2] = rgba[3] = uint8_t(colvals[0] * 255.0f);
      break;

    case 2: // LUMINANCE_ALPHA
      rgba[0] = rgba[1] = rgba[2] = uint8_t(colvals[0] * 255.0f);
      rgba[3] = uint8_t(colvals[1] * 255.0f);
      break;

    case 4: // RGBA
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
  else if (this->datatype == INTS) {
    const int colidx = idx * 4;
    rgba[0] = this->int_entries[colidx + 0];
    rgba[1] = this->int_entries[colidx + 1];
    rgba[2] = this->int_entries[colidx + 2];
    rgba[3] = this->int_entries[colidx + 3];
  }
  else assert(FALSE);
}

// FIXME: reactivate the stuff from the transfer table caching (ripped
// out of VoxelChunk.cpp):
//
//   // The simple idea for speeding up transfer of volume data is to
//   // dynamically fill in an index array, so each transfer value
//   // calculation is done only once.
//   static void blankoutTransferTable(void);
//   static uint32_t transfertable[256];
//   static SbBool transferdone[256];
//   static uint32_t transfertablenodeid;
//
// [...]
//
// uint32_t CvrVoxelChunk::transfertable[COLOR_TABLE_PREDEF_SIZE];
// SbBool CvrVoxelChunk::transferdone[COLOR_TABLE_PREDEF_SIZE];
// uint32_t CvrVoxelChunk::transfertablenodeid = 0;
//
// constructor [...]
//   static SbBool init_static = TRUE;
//   if (init_static) {
//     init_static = FALSE;
//     // Make sure this is set to an unused value, so it gets
//     // initialized at first invocation.
//     CvrVoxelChunk::transfertablenodeid = SoNode::getNextNodeId();
//   }
//
// transfer() [...]
//   // This table needs to be invalidated when any parameter of the
//   // SoTransferFunction node changes.
//   if (CvrVoxelChunk::transfertablenodeid != transferfunc->getNodeId()) {
//     CvrVoxelChunk::blankoutTransferTable();
//     CvrVoxelChunk::transfertablenodeid = transferfunc->getNodeId();
//   }
// [...]
//           if (CvrVoxelChunk::transferdone[voldataidx]) {
//             output[texelidx] = CvrVoxelChunk::transfertable[voldataidx];
//             if (invisible) {
//               uint8_t alpha =
//                 (endianness == COIN_HOST_IS_LITTLEENDIAN) ?
//                 ((output[texelidx] & 0xff000000) > 24) :
//                 (output[texelidx] & 0x000000ff);
//               invisible = (alpha == 0x00);
//             }
//             continue;
//           }
// [...]
//           CvrVoxelChunk::transferdone[voldataidx] = TRUE;
//           CvrVoxelChunk::transfertable[voldataidx] = output[texelidx];
//
// [...]
// void
// CvrVoxelChunk::blankoutTransferTable(void)
// {
//   for (unsigned int i=0; i < COLOR_TABLE_PREDEF_SIZE; i++) {
//     CvrVoxelChunk::transferdone[i] = FALSE;
//   }
// }
