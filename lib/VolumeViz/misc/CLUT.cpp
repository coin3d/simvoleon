// This datatype is a color lookup table.
//
// FIXME: handle other types of entries apart from 32-bit RGBA.

#include <VolumeViz/misc/CvrCLUT.h>


CvrCLUT::CvrCLUT(const unsigned int nrentries, uint8_t * rgba8bits)
{
  this->nrentries = nrentries;

  const int blocksize = nrentries * 4;
  this->entries = new uint8_t[blocksize];
  (void)memcpy(this->entries, rgba8bits, blocksize);
}

CvrCLUT::~CvrCLUT()
{
  delete[] this->entries;
}

void
CvrCLUT::activate(const cc_glglue * glw) const
{
  // FIXME: should only need to do this once somewhere else
  glEnable(GL_COLOR_TABLE);

  // FIXME: handle this->nrentries != 256
  assert(this->nrentries == 256);

  cc_glglue_glColorTable(glw,
                         GL_TEXTURE_2D, /* target */
                         GL_RGBA, /* GL internalformat */
                         this->nrentries, /* nr of paletteentries */
                         GL_RGBA, /* palette entry format */
                         GL_UNSIGNED_BYTE, /* palette entry unit type */
                         this->entries); /* data ptr */
}
