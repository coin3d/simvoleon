/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/


#include <VolumeViz/nodes/SoVolumeData.h>
#include <Inventor/misc/SoGLImage.h>
#include <memory.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <VolumeViz/elements/SoVolumeDataElement.h>

/*#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H*/


// *************************************************************************

SO_NODE_SOURCE(SoVolumeData);

// *************************************************************************

class SoVolumeDataP{
public:
  SoVolumeDataP(SoVolumeData * master) 
  {
    this->master = master;
    sliceX = NULL;
    sliceY = NULL;
    sliceZ = NULL;

    volumeSize = SbBox3f(-1, -1, -1, 1, 1, 1);
    dimensions = SbVec3s(1, 1, 1);
    data = NULL;
  }// constructor

  ~SoVolumeDataP()
  {
    releaseTextures();
  }// destructor


  void * data;
  SbVec3s dimensions;
  SbBox3f volumeSize;

  SoGLImage **sliceX;
  SoGLImage **sliceY;
  SoGLImage **sliceZ;

  const void * getRGBASlice(int sliceIdx, SoVolumeData::Axis axis);
  const void * getRGBASliceX(int sliceIdx);
  const void * getRGBASliceY(int sliceIdx);
  const void * getRGBASliceZ(int sliceIdx);
  void releaseTextures();
private:
  SoVolumeData * master;
};


#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

// *************************************************************************


/*!
  Constructor.
*/
SoVolumeData::SoVolumeData(void)
{
  SO_NODE_CONSTRUCTOR(SoVolumeData);

  PRIVATE(this) = new SoVolumeDataP(this);
  
  SO_NODE_DEFINE_ENUM_VALUE(StorageHint, AUTO);
  SO_NODE_DEFINE_ENUM_VALUE(StorageHint, TEX2D_MULTI);
  SO_NODE_DEFINE_ENUM_VALUE(StorageHint, TEX2D);
  SO_NODE_DEFINE_ENUM_VALUE(StorageHint, TEX3D);
  SO_NODE_DEFINE_ENUM_VALUE(StorageHint, MEMORY);
  SO_NODE_DEFINE_ENUM_VALUE(StorageHint, VOLUMEPRO);
  SO_NODE_DEFINE_ENUM_VALUE(StorageHint, TEX2D_SINGLE);
  SO_NODE_SET_SF_ENUM_TYPE(storageHint, StorageHint);

  SO_NODE_DEFINE_ENUM_VALUE(DataType, VOLUMEPRO);
  SO_NODE_DEFINE_ENUM_VALUE(DataType, TEX2D_SINGLE);

  SO_NODE_DEFINE_ENUM_VALUE(DataType, NEAREST);
  SO_NODE_DEFINE_ENUM_VALUE(DataType, MAX);
  SO_NODE_DEFINE_ENUM_VALUE(DataType, AVERAGE);

  SO_NODE_DEFINE_ENUM_VALUE(DataType, NONE);
  SO_NODE_DEFINE_ENUM_VALUE(DataType, CONSTANT);
  SO_NODE_DEFINE_ENUM_VALUE(DataType, LINEAR);
  SO_NODE_DEFINE_ENUM_VALUE(DataType, CUBIC);

  SO_NODE_DEFINE_ENUM_VALUE(Axis, X);
  SO_NODE_DEFINE_ENUM_VALUE(Axis, Y);
  SO_NODE_DEFINE_ENUM_VALUE(Axis, Z);

  SO_NODE_ADD_FIELD(fileName, (""));
  SO_NODE_ADD_FIELD(storageHint, (SoVolumeData::AUTO));
  SO_NODE_ADD_FIELD(usePalettedTexture, (TRUE));
  SO_NODE_ADD_FIELD(useCompressedTexture, (TRUE));
}//Constructor



/*!
  Destructor.
*/
SoVolumeData::~SoVolumeData()
{
  delete PRIVATE(this);
}




// Doc from parent class.
void
SoVolumeData::initClass(void)
{
  static int first = 0;
  if (first == 1) return;
  first = 1;

  SO_NODE_INIT_CLASS(SoVolumeData, SoVolumeRendering, "VolumeRendering");

  SoVolumeDataElement::initClass();

  SO_ENABLE(SoGLRenderAction, SoVolumeDataElement);
}// initClass




void 
SoVolumeData::setVolumeSize(const SbBox3f &size)
{
  PRIVATE(this)->volumeSize = size;
}// setVolumeSize


SbBox3f &
SoVolumeData::getVolumeSize() 
{ return PRIVATE(this)->volumeSize; }


SbVec3s &
SoVolumeData::getDimensions() 
{ return PRIVATE(this)->dimensions; }


void 
SoVolumeData::setVolumeData(const SbVec3s &dimensions, 
                            void *data, 
                            SoVolumeData::DataType type) 
{
  
  PRIVATE(this)->data = data;
  PRIVATE(this)->dimensions = dimensions;

  PRIVATE(this)->sliceX = new SoGLImage*[dimensions[0]];
  PRIVATE(this)->sliceY = new SoGLImage*[dimensions[1]];
  PRIVATE(this)->sliceZ = new SoGLImage*[dimensions[2]];

  memset(PRIVATE(this)->sliceX, 0, sizeof(SoGLImage*)*dimensions[0]);
  memset(PRIVATE(this)->sliceY, 0, sizeof(SoGLImage*)*dimensions[1]);
  memset(PRIVATE(this)->sliceZ, 0, sizeof(SoGLImage*)*dimensions[2]);
}



SbBool 
SoVolumeData::getVolumeData(SbVec3s &dimensions, 
                            void *&data, 
                            SoVolumeData::DataType &type) 
{ return FALSE; }

void 
SoVolumeData::setReader(SoVolumeReader &reader) 
{}

SoVolumeReader * 
SoVolumeData::getReader() 
{ return NULL; }

SbBool 
SoVolumeData::getMinMax(int &min, int &max) 
{ return FALSE; }

SbBool 
SoVolumeData::getHistogram(int &length, int *&histogram) 
{ return FALSE; }

SoVolumeData * 
SoVolumeData::subSetting(const SbBox3s &region) 
{ return NULL; }

void 
SoVolumeData::updateRegions(const SbBox3s *region, int num) 
{}

SoVolumeData * 
SoVolumeData::reSampling(const SbVec3s &dimensions, 
                         SoVolumeData::SubMethod subMethod, 
                         SoVolumeData::OverMethod) 
{ return NULL; }

void 
SoVolumeData::setTexMemorySize(int size) 
{}

void 
SoVolumeData::setPageSize(int size) 
{}

void 
SoVolumeData::setPageSize(SbVec3s &size) 
{}

void 
SoVolumeData::enableSubSampling(SbBool enable) 
{}

void 
SoVolumeData::enableAutoSubSampling(SbBool enable) 
{}

void 
SoVolumeData::enableAutoUnSampling(SbBool enable) 
{}

void 
SoVolumeData::unSample() 
{}

void 
SoVolumeData::setSubSamplingMethod(SubMethod method) 
{}

void 
SoVolumeData::setSubSamplingLevel(const SbVec3s &ROISampling, 
                    const SbVec3s &secondarySampling) 
{}

/*!
  Coin method.
*/
void
SoVolumeData::GLRender(SoGLRenderAction * action)
{
  SoVolumeDataElement::setVolumeData(action->getState(), this, this);
}



SoGLImage *
SoVolumeData::getGLImageSlice(int sliceIdx, Axis axis)
{
  // FIXME: Need to implement support for different pixelformats. 
  // torbjorv 07122002

  // FIXME: Can we delete the pixeldata after sending to SoGLImage?
  // torbjorv 07122002
  switch (axis)
  {
    case X: if ( !PRIVATE(this)->sliceX[sliceIdx] ) {
              const unsigned char * RGBATexture = 
                (unsigned char *)PRIVATE(this)->getRGBASliceX(sliceIdx);
              SoGLImage * GLImage = new SoGLImage();
              GLImage->setData( RGBATexture, 
                                SbVec2s(PRIVATE(this)->dimensions[2], 
                                        PRIVATE(this)->dimensions[1]),
                                4);
              PRIVATE(this)->sliceX[sliceIdx] = GLImage;
            }// else      
            return PRIVATE(this)->sliceX[sliceIdx];
            break;

    case Y: if ( !PRIVATE(this)->sliceY[sliceIdx] ) {
              const unsigned char * RGBATexture = 
                (unsigned char *)PRIVATE(this)->getRGBASliceY(sliceIdx);
              SoGLImage * GLImage = new SoGLImage();
              GLImage->setData( RGBATexture, 
                                SbVec2s(PRIVATE(this)->dimensions[0], 
                                        PRIVATE(this)->dimensions[2]),
                                4);
              PRIVATE(this)->sliceY[sliceIdx] = GLImage;
            }// else      
            return PRIVATE(this)->sliceY[sliceIdx];
            break;

    case Z: if ( !PRIVATE(this)->sliceZ[sliceIdx] ) {
              const unsigned char * RGBATexture = 
                (unsigned char *)PRIVATE(this)->getRGBASliceZ(sliceIdx);
              SoGLImage * GLImage = new SoGLImage();
              GLImage->setData( RGBATexture, 
                                SbVec2s(PRIVATE(this)->dimensions[0], 
                                        PRIVATE(this)->dimensions[1]),
                                4);
              PRIVATE(this)->sliceZ[sliceIdx] = GLImage;
            }// else
            return PRIVATE(this)->sliceZ[sliceIdx];
            break;
  }// switch
  return NULL;
}// getSlice


// Returns a texture with Z as horisontal and Y as vertical axis
// Assumes that the provided data is in RGBA-form
// Caller deletes of course. This function may be optimized 
// when coming to multiplications and stuff. Could just as well 
// use additions, but it will be more messy. The cache-misses will 
// dominate anyway. 
const void * 
SoVolumeDataP::getRGBASliceX(int sliceIdx)
{
  int * texture = 
    new int[dimensions[2]*dimensions[1]];
  int * intData = (int*)data;

  int i = 0;
  for (int y = 0; y < dimensions[1]; y++)
    for (int z = 0; z < dimensions[2]; z++) {
      texture[i] = 
        intData[z*dimensions[0]*dimensions[1] + dimensions[0]*y + sliceIdx];
      i++;
    }// for

  return texture;
}// getRGBASliceX



// Returns a texture with X as horisontal and Z as vertical axis
// Assumes that the provided data is in RGBA-form
// Caller deletes of course. const void * 
const void * 
SoVolumeDataP::getRGBASliceY(int sliceIdx)
{
  int * texture = 
    new int[dimensions[0]*dimensions[2]];
  int * intData = (int*)data;

  int i = 0;
  for (int z = dimensions[2] - 1; z >= 0; z--)
    for (int x = 0; x < dimensions[0]; x++) {
      texture[i] = 
        intData[z*dimensions[0]*dimensions[1] + dimensions[0]*sliceIdx + x];
      i++;
    }// for

  return texture;
}// getRGBASliceY



// Returns a texture with X as horisontal and Y as vertical axis
// Assumes that the provided data is in RGBA-form
// Caller deletes of course. const void * 
const void * 
SoVolumeDataP::getRGBASliceZ(int sliceIdx)
{
  int * texture = 
    new int[dimensions[0]*dimensions[1]];
  int * intData = (int*)data;

  int i = 0;
  for (int y = 0; y < dimensions[1]; y++)
    for (int x = 0; x < dimensions[0]; x++) {
      texture[i] = 
        intData[sliceIdx*dimensions[0]*dimensions[1] + dimensions[0]*y + x];
      i++;
    }// for

  return texture;
}// getRGBASliceZ



const void * 
SoVolumeDataP::getRGBASlice(int sliceIdx, SoVolumeData::Axis axis)
{
  switch (axis)
  {
    case SoVolumeData::X:
      return getRGBASliceX(sliceIdx);
      break;

    case SoVolumeData::Y:
      return getRGBASliceY(sliceIdx);
      break;

    case SoVolumeData::Z:
      return getRGBASliceZ(sliceIdx);
      break;
  }// switch

  return NULL;
}// getRGBASlice


void SoVolumeDataP::releaseTextures()
{
  int i;
  if ( sliceX ) {
    for (i = 0; i < dimensions[0]; i++) {
      if (sliceX[i]) {
        sliceX[i]->unref();
        sliceX[i] = NULL;
      }
    }// for
  }
  if (sliceY) {
    for (i = 0; i < dimensions[1]; i++) {
      if (sliceY[i]) {
        sliceY[i]->unref();
        sliceY[i] = NULL;
      }
    }// for
  }
  if ( sliceZ ) {
    for (i = 0; i < dimensions[2]; i++) {
      if (sliceZ[i]) {
        sliceZ[i]->unref();
        sliceZ[i] = NULL;
      }
    }// for
  }

  delete [] sliceX;
  delete [] sliceY;
  delete [] sliceZ;
  sliceX = NULL;
  sliceY = NULL;
  sliceZ = NULL;
}// releaseTextures
