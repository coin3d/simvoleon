/*!
  \class SoVRVolFileReader VolumeViz/readers/SoVRVolFileReader.h
  \brief Loader for files in the VOL data format.
  \ingroup volviz


*/


#include <VolumeViz/readers/SoVRVolFileReader.h>

SoVRVolFileReader::SoVRVolFileReader(void)
{
}

SoVRVolFileReader::~SoVRVolFileReader()
{
}

void
SoVRVolFileReader::setUserData(void * data)
{
}

void
SoVRVolFileReader::getDataChar(SbBox3f & size, SoVolumeData::DataType & type,
                               SbVec3s & dim)
{
}

void
SoVRVolFileReader::getSubSlice(SbBox2s & subslice, int slicenumber, void * data)
{
}

void
SoVRVolFileReader::setUserData(void * data)
{
  inherited::setFilename((const char *)data);
}
