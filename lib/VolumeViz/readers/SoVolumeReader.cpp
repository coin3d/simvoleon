/*!
  \class SoVolumeReader VolumeViz/readers/SoVolumeReader.h
  \brief Abstract superclass for all volume data reader classes.
  \ingroup volviz
*/

#include <VolumeViz/readers/SoVolumeReader.h>

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H


class SoVolumeReaderP{
public:
  SoVolumeReaderP(SoVolumeReader * master) {
    this->master = master;
  }

private:
  SoVolumeReader * master;
};


#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

// *************************************************************************

SoVolumeReader::SoVolumeReader(void)
{
  PRIVATE(this) = new SoVolumeReaderP(this);

}

SoVolumeReader::~SoVolumeReader()
{
  delete PRIVATE(this);
}

// FIXME: unimplemented methods. 20021108 mortene.

void SoVolumeReader::setUserData(void * data) { }
int SoVolumeReader::setFilename(const char * filename) { return 0; }
void * SoVolumeReader::getBuffer(int64_t offset, unsigned int size) { return NULL; }
int SoVolumeReader::bytesToInt(unsigned char * ptr, int sizeBytes) { return 0x0000; }
void SoVolumeReader::swapBytes(int * intPtr, int sizeBytes) { return; }
int64_t SoVolumeReader::fileSize(void) { return 0; }
