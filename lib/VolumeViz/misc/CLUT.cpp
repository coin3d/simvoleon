// This datatype is a color lookup table.
//
// FIXME: handle other types of entries apart from 32-bit RGBA.

#include <VolumeViz/misc/CvrCLUT.h>
#include <assert.h>
#include <stdlib.h>


CvrCLUT::CvrCLUT(const unsigned int nrcols, const uint8_t * rgba8bits)
{
  this->nrentries = nrcols;
  this->nrcomponents = 4;
  this->datatype = INTS;

  const int blocksize = this->nrentries * this->nrcomponents;
  this->int_entries = new uint8_t[blocksize];
  (void)memcpy(this->int_entries, rgba8bits, blocksize * sizeof(uint8_t));
  // (yeah, yeah, I know sizeof(uint8_t) is always == 1)
}

// nrcomponents == 1: LUMINANCE
// nrcomponents == 2: LUMINANCE + ALPHA
// nrcomponents == 4: RGBA
CvrCLUT::CvrCLUT(const unsigned int nrcols, const unsigned int nrcomponents,
                 const float * colormap)
{
  this->nrentries = nrcols;
  this->nrcomponents = nrcomponents;
  this->datatype = FLOATS;

  const int blocksize = this->nrentries * this->nrcomponents;
  this->flt_entries = new float[blocksize];
  (void)memcpy(this->int_entries, colormap, blocksize * sizeof(float));
}

CvrCLUT::~CvrCLUT()
{
  if (this->datatype == INTS)
    delete[] this->int_entries;
  if (this->datatype == FLOATS)
    delete[] this->flt_entries;
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

  // FIXME: should probably check if all is ok with by using
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
  int actualsize;
  glGetColorTableParameteriv(GL_TEXTURE_2D, GL_COLOR_TABLE_WIDTH, &actualsize);
  assert(actualsize == this->nrentries);
}
