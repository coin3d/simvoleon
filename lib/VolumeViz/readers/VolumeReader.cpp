/*!
  \class SoVolumeReader VolumeViz/readers/SoVolumeReader.h
  \brief Abstract superclass for all volume data reader classes.
  \ingroup volviz
*/

#include <VolumeViz/readers/SoVolumeReader.h>
#include <Inventor/errors/SoDebugError.h>

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

class SoVolumeReaderP{
public:
  SoVolumeReaderP(SoVolumeReader * master) {
    this->master = master;
  }

  SbString filename;

private:
  SoVolumeReader * master;
};


#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

// *************************************************************************

SoVolumeReader::SoVolumeReader(void)
{
  PRIVATE(this) = new SoVolumeReaderP(this);
  this->m_data = NULL;
}

SoVolumeReader::~SoVolumeReader()
{
  // FIXME: dealloc this->m_data when we allocated it
  // ourselves. 20021109 mortene.

  delete PRIVATE(this);
}

void
SoVolumeReader::setUserData(void * data)
{
  // FIXME: unimplemented. 20021108 mortene.
}

// *************************************************************************

int
SoVolumeReader::setFilename(const char * filename)
{
  PRIVATE(this)->filename = filename;

  // TGS doc doesn't say what this is supposed to return.
  return 0;
}

int64_t
SoVolumeReader::fileSize(void)
{
  struct stat buf;
  const char * filename = PRIVATE(this)->filename.getString();

  if (stat(filename, &buf) == 0) {
    return buf.st_size;
  }
  else {
    SoDebugError::post("SoVolumeReader::fileSize", "couldn't stat() '%s': %s",
                       filename, strerror(errno));
    return -1;
  }
}

void *
SoVolumeReader::getBuffer(int64_t offset, unsigned int size)
{
  return NULL;
}

// FIXME: unimplemented methods. 20021108 mortene.

int SoVolumeReader::bytesToInt(unsigned char * ptr, int sizeBytes) { return 0x0000; }
void SoVolumeReader::swapBytes(int * intPtr, int sizeBytes) { return; }

// *************************************************************************
