// This datatype is a color lookup table.

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
