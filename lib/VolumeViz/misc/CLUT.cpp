// This datatype is a color lookup table.

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <Inventor/SbBasic.h>
#include <Inventor/errors/SoDebugError.h>

#include <VolumeViz/misc/CvrCLUT.h>
#include <VolumeViz/misc/CvrUtil.h>


// colormap values are between 0 and 255
CvrCLUT::CvrCLUT(const unsigned int nrcols, const uint8_t * colormap)
{
  this->nrentries = nrcols;
  this->nrcomponents = 4;
  this->datatype = INTS;

  const int blocksize = this->nrentries * this->nrcomponents;
  this->int_entries = new uint8_t[blocksize];
  (void)memcpy(this->int_entries, colormap, blocksize);

  this->crc32cmap = CvrUtil::crc32(this->int_entries, blocksize);

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
  const int copysize = blocksize * sizeof(float);
  this->flt_entries = new float[blocksize];
  (void)memcpy(this->flt_entries, colormap, copysize);

  this->crc32cmap = CvrUtil::crc32((uint8_t *)this->flt_entries, copysize);

  this->commonConstructor();
}

void
CvrCLUT::commonConstructor(void)
{
  this->refcount = 0;

  this->transparencythresholds[0] = 0;
  this->transparencythresholds[1] = this->nrentries - 1;

  this->alphapolicy = CvrCLUT::ALPHA_AS_IS;

  this->glcolors = new uint8_t[this->nrentries * 4];
  this->regenerateGLColorData();
}

// Copy constructor.
CvrCLUT::CvrCLUT(const CvrCLUT & clut)
{
  this->nrentries = clut.nrentries;
  this->nrcomponents = clut.nrcomponents;
  this->datatype = clut.datatype;

  const int blocksize = this->nrentries * this->nrcomponents;

  switch (this->datatype) {
  case INTS:
    this->int_entries = new uint8_t[blocksize];
    (void)memcpy(this->int_entries, clut.int_entries, blocksize * sizeof(uint8_t));
    break;

  case FLOATS:
    this->flt_entries = new float[blocksize];
    (void)memcpy(this->flt_entries, clut.flt_entries, blocksize * sizeof(float));
    break;

  default: assert(FALSE); break;
  }
  this->crc32cmap = clut.crc32cmap;

  this->refcount = 0;

  this->transparencythresholds[0] = clut.transparencythresholds[0];
  this->transparencythresholds[1] = clut.transparencythresholds[1];

  this->alphapolicy = clut.alphapolicy;

  this->glcolors = new uint8_t[this->nrentries * 4];
  this->regenerateGLColorData();
}

CvrCLUT::~CvrCLUT()
{
  if (this->datatype == INTS)
    delete[] this->int_entries;
  if (this->datatype == FLOATS)
    delete[] this->flt_entries;

  delete[] this->glcolors;
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

// Equality comparison between the color lookup tables. The comparison
// will be quick, as it's based on having a checksum for the color
// table.
int
operator==(const CvrCLUT & c1, const CvrCLUT & c2)
{
  if (&c1 == &c2) { return TRUE; }

  if (c1.crc32cmap != c2.crc32cmap) { return FALSE; }

  assert(c1.nrentries == c2.nrentries);
  assert(c1.nrcomponents == c2.nrcomponents);
  assert(c1.datatype == c2.datatype);

#if CVR_DEBUG
  // Checking that CRC32 checksums don't give false positives.  This
  // has implications for performance, so get rid of it after a while.
  const int blocksize = c1.nrentries * c1.nrcomponents;
  const int blksize = blocksize * (c1.datatype == CvrCLUT::INTS ? 1 : sizeof(float));
  assert(memcmp(c1.int_entries, c2.int_entries, blksize) == 0);
#endif // CVR_DEBUG

  if (c1.transparencythresholds[0] != c2.transparencythresholds[0]) { return FALSE; }
  if (c1.transparencythresholds[1] != c2.transparencythresholds[1]) { return FALSE; }
  if (c1.alphapolicy != c2.alphapolicy) { return FALSE; }

  return TRUE;
}

int
operator!=(const CvrCLUT & c1, const CvrCLUT & c2)
{
  return ! operator==(c1, c2);
}


// Everything below "low" (but not including) and above "high" (but
// not including), will be fully transparent.
void
CvrCLUT::setTransparencyThresholds(uint32_t low, uint32_t high)
{
  if ((this->transparencythresholds[0] == low) &&
      (this->transparencythresholds[1] == high)) {
    return;
  }

  assert(low <= high);
  assert(low < this->nrentries);
  assert(high < this->nrentries);

  this->transparencythresholds[0] = low;
  this->transparencythresholds[1] = high;

  this->regenerateGLColorData();
}

void
CvrCLUT::activate(const cc_glglue * glw) const
{
  // FIXME: should only need to do this once somewhere else
  glEnable(GL_COLOR_TABLE);

  // FIXME: handle this->nrentries != 256
  assert(this->nrentries == 256);

  // FIXME: should probably set glColorTableParameter() on
  // GL_COLOR_TABLE_SCALE and GL_COLOR_TABLE_BIAS.

  // FIXME: should probably check if all is ok by using
  // PROXY_TEXTURE_2D first.

  cc_glglue_glColorTable(glw,
                         GL_TEXTURE_2D, /* target */
                         GL_RGBA, /* GL internalformat */
                         this->nrentries, /* nr of paletteentries */
                         GL_RGBA, /* palette entry format */
                         GL_UNSIGNED_BYTE, /* palette entry unit type */
                         this->glcolors); /* data ptr */

  // Error checking.

  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    static SbBool warn = TRUE;
    if (warn) {
      warn = FALSE;
      SoDebugError::postWarning("CvrCLUT::activate",
                                "glColorTable(GL_TEXTURE_2D, ...) caused "
                                "glGetError()==0x%x (dec==%d)",
                                err, err);

      // This matches the driver on ASK.trh.sim.no.
      const char * VERSION = "1.1.28 PT";
      const char * VENDOR = "3Dlabs";
      const char * RENDERER = "GLINT R3 PT + GLINT Gamma";
      if (strcmp((const char *)glGetString(GL_VERSION), VERSION) == 0 &&
          strcmp((const char *)glGetString(GL_VENDOR), VENDOR) == 0 &&
          strcmp((const char *)glGetString(GL_RENDERER), RENDERER) == 0) {
        SoDebugError::postWarning("CvrCLUT::activate",
                                  "This is a known problem with this driver "
                                  "(vendor='%s', renderer='%s', version='%s') "
                                  "and seems to be harmless.",
                                  VENDOR, RENDERER, VERSION);
      }
    }
  }

  // Sanity check.
  GLint actualsize;
  cc_glglue_glGetColorTableParameteriv(glw, GL_TEXTURE_2D,
                                       GL_COLOR_TABLE_WIDTH, &actualsize);
  assert(actualsize == (GLint)this->nrentries);
}

// Find RGBA color at the given idx.
void
CvrCLUT::lookupRGBA(const unsigned int idx, uint8_t rgba[4]) const
{
  assert(idx < this->nrentries);
  for (int i=0; i < 4; i++) { rgba[i] = this->glcolors[idx * 4 + i]; }
}

void
CvrCLUT::setAlphaUse(AlphaUse policy)
{
  if (this->alphapolicy == policy) { return; }

  this->alphapolicy = policy;
  this->regenerateGLColorData();
}

void
CvrCLUT::regenerateGLColorData(void)
{
  for (unsigned int idx = 0; idx < this->nrentries; idx++) {
    uint8_t * rgba = &this->glcolors[idx * 4];
    if ((idx < this->transparencythresholds[0]) ||
        (idx > this->transparencythresholds[1])) {
      rgba[0] = rgba[1] = rgba[2] = rgba[3] = 0x00;
    }
    else {
      if (this->datatype == FLOATS) {
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

    switch (this->alphapolicy) {
    case ALPHA_AS_IS: /* "as is", leave alone */ break;
    case ALPHA_OPAQUE: rgba[3] = 0xff; break;
    case ALPHA_BINARY: rgba[3] = (rgba[3] == 0) ? 0 : 0xff; break;
    default: assert(FALSE); break;
    }
  }
}
