/*!
  \class SoVRMemReader VolumeViz/readers/SoVRMemReader.h
  \brief FIXME: doc
  \ingroup volviz
*/

#include <VolumeViz/readers/SoVRMemReader.h>
#include <VolumeViz/misc/CvrUtil.h>
#include <Inventor/errors/SoDebugError.h>


class SoVRMemReaderP {
public:
  SoVRMemReaderP(SoVRMemReader * master) {
    this->master = master;

    this->dimensions = SbVec3s(0, 0, 0);
    this->dataType = SoVolumeData::UNSIGNED_BYTE;
  }

  SbVec3s dimensions;
  SoVolumeData::DataType dataType;

private:
  SoVRMemReader * master;
};

#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

// *************************************************************************

SoVRMemReader::SoVRMemReader(void)
{
  PRIVATE(this) = new SoVRMemReaderP(this);

}

SoVRMemReader::~SoVRMemReader()
{
  delete PRIVATE(this);
}


void SoVRMemReader::setUserData(void * data)
{
}

void SoVRMemReader::getDataChar(SbBox3f & size,
                                SoVolumeData::DataType & type,
                                SbVec3s & dim)
{
  type = PRIVATE(this)->dataType;
  dim = PRIVATE(this)->dimensions;

  size.setBounds(-dim[0]/2.0f, -dim[1]/2.0f, -dim[2]/2.0f,
                 dim[0]/2.0f, dim[1]/2.0f, dim[2]/2.0f);
}

void SoVRMemReader::getSubSlice(SbBox2s & subslice,
                                int sliceNumber,
                                void * data,
                                Axis axis)
{
  short width, height;
  subslice.getSize(width, height);

  unsigned int axisidx = (axis == X) ? 0 : ((axis == Y) ? 1 : 2);
  CvrUtil::buildSubPage(axisidx,
                        (const uint8_t *)this->m_data, (uint8_t *)data,
                        sliceNumber, subslice, width,
                        PRIVATE(this)->dataType, PRIVATE(this)->dimensions);
}


void
SoVRMemReader::setData(const SbVec3s &dimensions,
                       void * data,
                       SoVolumeData::DataType type)
{
  PRIVATE(this)->dimensions = dimensions;
  this->m_data = data;
  PRIVATE(this)->dataType = type;
}
