/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/


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

}//Constructor






/*!
  Destructor.
*/
SoVolumeReader::~SoVolumeReader()
{
  delete PRIVATE(this);
}



void 
SoVolumeReader::setUserData(void * data)
{}

void 
SoVolumeReader::getDataChar(SbBox3f &size, SoVolumeRendering::DataType &type, SbVec3s &dim)
{}

void 
SoVolumeReader::getSubSlice(SbBox2s &subSlice, 
                            int sliceNumber, 
                            void * data,
                            SoVolumeRendering::Axis axis)
{}