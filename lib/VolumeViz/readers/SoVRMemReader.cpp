/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/


#include <VolumeViz/readers/SoVRMemReader.h>

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H


class SoVRMemReaderP{
public:
  SoVRMemReaderP(SoVRMemReader * master) {
    this->master = master;
  }

  SbVec3s size;
  void * data;
  SoVolumeData::DataType type;


private:
  SoVRMemReader * master;
};


#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

// *************************************************************************

/*!
  Constructor.
*/
SoVRMemReader::SoVRMemReader(void)
{
  PRIVATE(this) = new SoVRMemReaderP(this);

}//Constructor

/*!
  Destructor.
*/
SoVRMemReader::~SoVRMemReader()
{
  delete PRIVATE(this);
}



void SoVRMemReader::setUserData(void * data)
{
}

void SoVRMemReader::getDataChar(SbBox3f &size, SoVolumeData::DataType &type, SbVec3s &dim)
{
}

void SoVRMemReader::getSubSlice(SbBox2s &subSlice, int sliceNumber, void * data, SoVolumeData::Axis axis)
{
}


void SoVRMemReader::setData(const SbVec3s &dimension, 
                            const void *data, 
                            SoVolumeData::DataType type)
{
}//setData
