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

/*!
  Constructor.
*/
SoVolumeReader::SoVolumeReader(void)
{
  PRIVATE(this) = new SoVolumeReaderP(this);

}





/*!
  Destructor.
*/
SoVolumeReader::~SoVolumeReader()
{
  delete PRIVATE(this);
}



void 
SoVolumeReader::setUserData(void * data)
{
}
