/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http:// www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/


#include <VolumeViz/nodes/SoVolumeData.h>
#include <Inventor/misc/SoGLImage.h>
#include <memory.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <VolumeViz/elements/SoVolumeDataElement.h>
#include <Inventor/elements/SoGLCacheContextElement.h>

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif // HAVE_WINDOWS_H*/

#include <GL/gl.h>

// *************************************************************************

SO_NODE_SOURCE(SoVolumeData);

// *************************************************************************

class SoVolumeDataP{
public:
  SoVolumeDataP(SoVolumeData * master) 
  {
    this->master = master;
    pageX = NULL;
    pageY = NULL;
    pageZ = NULL;

    volumeSize = SbBox3f(-1, -1, -1, 1, 1, 1);
    dimensions = SbVec3s(1, 1, 1);
    data = NULL;

    pageSize = SbVec3s(64, 64, 64);
  }// constructor

  ~SoVolumeDataP()
  {
    releaseTextures();
  }// destructor


  void * data;
  SbVec3s dimensions;
  SbBox3f volumeSize;

  SbVec3s pageSize;


  // Pointers to arrays with pointers. Each array contains every page along 
  // that axis. If the pagesize generates 16 pages each slice along Z-axis, 
  // page (col, row) in slice n would be positioned at 
  // slice[n*pageSize[0]*pageSize[1] + row*pageSize[0] + col]
  // Use get...Page-functions to retreive the pages. 
  SoGLImage **pageX;
  SoGLImage **pageY;
  SoGLImage **pageZ;

  const void * getRGBAPage(int sliceIdx, SoVolumeData::Axis axis, int col, int row);
  const void * getRGBAPageX(int sliceIdx, int col, int row);
  const void * getRGBAPageY(int sliceIdx, int col, int row);
  const void * getRGBAPageZ(int sliceIdx, int col, int row);

  // FIXME: Implement support for other pixelformats. torbjorv 08312002
  SoGLImage * getGLImagePageX(int sliceIdx, int col, int row);
  SoGLImage * getGLImagePageY(int sliceIdx, int col, int row);
  SoGLImage * getGLImagePageZ(int sliceIdx, int col, int row);

  // FIXME: Create functions that return GLImage for each page. 
  // Just for convenience. torbjorv 08312002 

  void releaseTextures();
  void releaseTexturesX();
  void releaseTexturesY();
  void releaseTexturesZ();

  bool check2n(int n);
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
}// Constructor



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



// FIXME: If size != 2^n these functions should extend to the nearest size. 
// torbjorv 07292002
void 
SoVolumeData::setVolumeData(const SbVec3s &dimensions, 
                            void *data, 
                            SoVolumeData::DataType type) 
{
  
  PRIVATE(this)->data = data;
  PRIVATE(this)->dimensions = dimensions;

  if (PRIVATE(this)->pageSize[0] > PRIVATE(this)->dimensions[0])
    PRIVATE(this)->pageSize[0] = PRIVATE(this)->dimensions[0];
  if (PRIVATE(this)->pageSize[1] > PRIVATE(this)->dimensions[1])
    PRIVATE(this)->pageSize[1] = PRIVATE(this)->dimensions[1];
  if (PRIVATE(this)->pageSize[2] > PRIVATE(this)->dimensions[2])
    PRIVATE(this)->pageSize[2] = PRIVATE(this)->dimensions[2];
}// setVolumeData




void 
SoVolumeData::setPageSize(int size) 
{
  setPageSize(SbVec3s(size, size, size));
}// setPageSize





void 
SoVolumeData::setPageSize(SbVec3s &size) 
{

  // Checking if the sizes are 2^n.
  // FIXME: Should there have been an assertion here? This baby doesn't
  // return anything... 08312002 torbjorv
  if (!PRIVATE(this)->check2n(size[0]) ||
      !PRIVATE(this)->check2n(size[1]) ||
      !PRIVATE(this)->check2n(size[2]))
  return;


  bool rebuildX = false;
  bool rebuildY = false;
  bool rebuildZ = false;

  // The X-size has changed. Rebuild Y- and Z-axis maps. 
  if (size[0] != PRIVATE(this)->pageSize[0]) {
    rebuildY = true;
    rebuildZ = true;
  }// if

  // The Y-size has changed. Rebuild X- and Z-axis maps. 
  if (size[1] != PRIVATE(this)->pageSize[1]) {
    rebuildX = true;
    rebuildZ = true;
  }// if

  // The Z-size has changed. Rebuild X- and Y-axis maps. 
  if (size[2] != PRIVATE(this)->pageSize[2]) {
    rebuildX = true;
    rebuildY = true;
  }// if

  PRIVATE(this)->pageSize = size;

  if (rebuildX) PRIVATE(this)->releaseTexturesX();
  if (rebuildY) PRIVATE(this)->releaseTexturesY();
  if (rebuildZ) PRIVATE(this)->releaseTexturesZ();
}// setPageSize




/*!
  Coin method.
*/
void
SoVolumeData::GLRender(SoGLRenderAction * action)
{
  SoVolumeDataElement::setVolumeData(action->getState(), this, this);
}// GLRender



SoGLImage *
SoVolumeData::getGLImagePage(int sliceIdx, Axis axis, int col, int row)
{
  // FIXME: Need to implement support for different pixelformats. 
  // torbjorv 07122002

  // FIXME: Can we delete the pixeldata after sending to SoGLImage?
  // torbjorv 07122002
  switch (axis)
  {
    case X: return PRIVATE(this)->getGLImagePageX(sliceIdx, col, row);
            break;

    case Y: return PRIVATE(this)->getGLImagePageY(sliceIdx, col, row);
            break;

    case Z: return PRIVATE(this)->getGLImagePageZ(sliceIdx, col, row);
            break;
  }// switch
  return NULL;
}// getSlice




void SoVolumeData::renderOrthoSliceX(SoState * state, int sliceIdx, SbBox2f &slice, SbBox2f &mappingcoordinates, float x)
{
  float minU, minV, maxU, maxV;
  float minZ, minY, maxZ, maxY;
  mappingcoordinates.getBounds(minU, minV, maxU, maxV);
  slice.getBounds(minZ, minY, maxZ, maxY);
 
  SbVec2f pageSizef = SbVec2f(float(PRIVATE(this)->pageSize[2])/PRIVATE(this)->dimensions[2], 
                              float(PRIVATE(this)->pageSize[1])/PRIVATE(this)->dimensions[1]);


  float localLeftU, localRightU;
  float localUpperV, localLowerV;
  float globalCurrentLeftU, globalCurrentRightU;
  float globalCurrentUpperV, globalCurrentLowerV;
  float currentLeftZ, currentRightZ, currentLowerY, currentUpperY;


  globalCurrentLowerV = minV;
  currentLowerY = minY;
  int row = minV*PRIVATE(this)->dimensions[1]/PRIVATE(this)->pageSize[1];
  localLowerV = (globalCurrentLowerV - row*pageSizef[1])/pageSizef[1];
  while (globalCurrentLowerV != maxV) {
    if ((row + 1)*pageSizef[1] < maxV) {
      globalCurrentUpperV = (row + 1)*pageSizef[1];
      currentUpperY = (maxY - minY)*(globalCurrentUpperV - minV)/(maxV - minV) + minY;
      localUpperV = 1.0;
    }// if
    else {
      globalCurrentUpperV = maxV;
      currentUpperY = maxY;
      localUpperV = (globalCurrentUpperV - row*pageSizef[1])/pageSizef[1];
    }// else
  

    int col = minU*PRIVATE(this)->dimensions[2]/PRIVATE(this)->pageSize[2];
    globalCurrentLeftU = minU;
    currentLeftZ = minZ;
    localLeftU = (globalCurrentLeftU - col*pageSizef[0])/pageSizef[0];
    while (globalCurrentLeftU != maxU) {
      if ((col + 1)*pageSizef[0] < maxU) {
        globalCurrentRightU = (col + 1)*pageSizef[0];
        currentRightZ = (maxZ - minZ)*(globalCurrentRightU - minU)/(maxU - minU) + minZ;
        localRightU = 1.0;
      }// if
      else {
        globalCurrentRightU = maxU;
        currentRightZ = maxZ;
        localRightU = (globalCurrentRightU - col*pageSizef[0])/pageSizef[0];
      }// else


      // rendering
      SoGLImage * image = 
        PRIVATE(this)->getGLImagePageX(sliceIdx, col, row);
      SoGLDisplayList * dl = image->getGLDisplayList(state);
      dl->call(state);

      glBegin(GL_QUADS);
      glColor4f(1, 1, 1, 1);
      glTexCoord2f(localLeftU, localLowerV);
      glVertex3f(x, currentLowerY, currentLeftZ);    
      glTexCoord2f(localRightU, localLowerV);
      glVertex3f(x, currentLowerY, currentRightZ);    
      glTexCoord2f(localRightU, localUpperV);
      glVertex3f(x, currentUpperY, currentRightZ);    
      glTexCoord2f(localLeftU, localUpperV);
      glVertex3f(x, currentUpperY, currentLeftZ); 

      glEnd();

      globalCurrentLeftU = globalCurrentRightU;
      currentLeftZ = currentRightZ;
      localLeftU = 0.0;
      col++;
    }// while

    globalCurrentLowerV = globalCurrentUpperV;
    currentLowerY = currentUpperY;
    localLowerV = 0.0;
    row++;
  }// while
}// renderOrthoSliceX







void SoVolumeData::renderOrthoSliceY(SoState * state, int sliceIdx, SbBox2f &slice, SbBox2f &mappingcoordinates, float y)
{
  float minU, minV, maxU, maxV;
  float minX, minZ, maxX, maxZ;
  mappingcoordinates.getBounds(minU, minV, maxU, maxV);
  slice.getBounds(minX, minZ, maxX, maxZ);
 
  SbVec2f pageSizef = SbVec2f(float(PRIVATE(this)->pageSize[2])/PRIVATE(this)->dimensions[2], 
                              float(PRIVATE(this)->pageSize[1])/PRIVATE(this)->dimensions[1]);


  float localLeftU, localRightU;
  float localUpperV, localLowerV;
  float globalCurrentLeftU, globalCurrentRightU;
  float globalCurrentUpperV, globalCurrentLowerV;
  float currentLeftX, currentRightX, currentLowerZ, currentUpperZ;


  globalCurrentLowerV = minV;
  currentLowerZ = minZ;
  int row = minV*PRIVATE(this)->dimensions[1]/PRIVATE(this)->pageSize[1];
  localLowerV = (globalCurrentLowerV - row*pageSizef[1])/pageSizef[1];
  while (globalCurrentLowerV != maxV) {
    if ((row + 1)*pageSizef[1] < maxV) {
      globalCurrentUpperV = (row + 1)*pageSizef[1];
      currentUpperZ = (maxZ - minZ)*(globalCurrentUpperV - minV)/(maxV - minV) + minZ;
      localUpperV = 1.0;
    }// if
    else {
      globalCurrentUpperV = maxV;
      currentUpperZ = maxZ;
      localUpperV = (globalCurrentUpperV - row*pageSizef[1])/pageSizef[1];
    }// else
  

    int col = minU*PRIVATE(this)->dimensions[0]/PRIVATE(this)->pageSize[0];
    globalCurrentLeftU = minU;
    currentLeftX = minX;
    localLeftU = (globalCurrentLeftU - col*pageSizef[0])/pageSizef[0];
    while (globalCurrentLeftU != maxU) {
      if ((col + 1)*pageSizef[0] < maxU) {
        globalCurrentRightU = (col + 1)*pageSizef[0];
        currentRightX = (maxX - minX)*(globalCurrentRightU - minU)/(maxU - minU) + minX;
        localRightU = 1.0;
      }// if
      else {
        globalCurrentRightU = maxU;
        currentRightX = maxX;
        localRightU = (globalCurrentRightU - col*pageSizef[0])/pageSizef[0];
      }// else


      // rendering
      SoGLImage * image = 
        PRIVATE(this)->getGLImagePageY(sliceIdx, col, row);
      SoGLDisplayList * dl = image->getGLDisplayList(state);
      dl->call(state);

      glBegin(GL_QUADS);
      glColor4f(1, 1, 1, 1);
      glTexCoord2f(localLeftU, localLowerV);
      glVertex3f(currentLeftX, y, currentLowerZ);    
      glTexCoord2f(localRightU, localLowerV);
      glVertex3f(currentRightX, y, currentLowerZ);    
      glTexCoord2f(localRightU, localUpperV);
      glVertex3f(currentRightX, y, currentUpperZ);    
      glTexCoord2f(localLeftU, localUpperV);
      glVertex3f(currentLeftX, y, currentUpperZ);    

      glEnd();

      globalCurrentLeftU = globalCurrentRightU;
      currentLeftX = currentRightX;
      localLeftU = 0.0;
      col++;
    }// while

    globalCurrentLowerV = globalCurrentUpperV;
    currentLowerZ = currentUpperZ;
    localLowerV = 0.0;
    row++;
  }// while
}// renderOrthoSliceY












void SoVolumeData::renderOrthoSliceZ(SoState * state, int sliceIdx, SbBox2f &slice, SbBox2f &mappingcoordinates, float z)
{
  float minU, minV, maxU, maxV;
  float minX, minY, maxX, maxY;
  mappingcoordinates.getBounds(minU, minV, maxU, maxV);
  slice.getBounds(minX, minY, maxX, maxY);
 
  SbVec2f pageSizef = SbVec2f(float(PRIVATE(this)->pageSize[0])/PRIVATE(this)->dimensions[0], 
                              float(PRIVATE(this)->pageSize[1])/PRIVATE(this)->dimensions[1]);


  float localLeftU, localRightU;
  float localUpperV, localLowerV;
  float globalCurrentLeftU, globalCurrentRightU;
  float globalCurrentUpperV, globalCurrentLowerV;
  float currentLeftX, currentRightX, currentLowerY, currentUpperY;


  globalCurrentLowerV = minV;
  currentLowerY = minY;
  int row = minV*PRIVATE(this)->dimensions[1]/PRIVATE(this)->pageSize[1];
  localLowerV = (globalCurrentLowerV - row*pageSizef[1])/pageSizef[1];
  while (globalCurrentLowerV != maxV) {
    if ((row + 1)*pageSizef[1] < maxV) {
      globalCurrentUpperV = (row + 1)*pageSizef[1];
      currentUpperY = (maxY - minY)*(globalCurrentUpperV - minV)/(maxV - minV) + minY;
      localUpperV = 1.0;
    }// if
    else {
      globalCurrentUpperV = maxV;
      currentUpperY = maxY;
      localUpperV = (globalCurrentUpperV - row*pageSizef[1])/pageSizef[1];
    }// else
  

    int col = minU*PRIVATE(this)->dimensions[0]/PRIVATE(this)->pageSize[0];
    globalCurrentLeftU = minU;
    currentLeftX = minX;
    localLeftU = (globalCurrentLeftU - col*pageSizef[0])/pageSizef[0];
    while (globalCurrentLeftU != maxU) {
      if ((col + 1)*pageSizef[0] < maxU) {
        globalCurrentRightU = (col + 1)*pageSizef[0];
        currentRightX = (maxX - minX)*(globalCurrentRightU - minU)/(maxU - minU) + minX;
        localRightU = 1.0;
      }// if
      else {
        globalCurrentRightU = maxU;
        currentRightX = maxX;
        localRightU = (globalCurrentRightU - col*pageSizef[0])/pageSizef[0];
      }// else


      // rendering
      SoGLImage * image = 
        PRIVATE(this)->getGLImagePageZ(sliceIdx, col, row);
      SoGLDisplayList * dl = image->getGLDisplayList(state);
      dl->call(state);

      glBegin(GL_QUADS);
      glColor4f(1, 1, 1, 1);
      glTexCoord2f(localLeftU, localLowerV);
      glVertex3f(currentLeftX, currentLowerY, z);    
      glTexCoord2f(localRightU, localLowerV);
      glVertex3f(currentRightX, currentLowerY, z);    
      glTexCoord2f(localRightU, localUpperV);
      glVertex3f(currentRightX, currentUpperY, z);    
      glTexCoord2f(localLeftU, localUpperV);
      glVertex3f(currentLeftX, currentUpperY, z);    

      glEnd();

      globalCurrentLeftU = globalCurrentRightU;
      currentLeftX = currentRightX;
      localLeftU = 0.0;
      col++;
    }// while

    globalCurrentLowerV = globalCurrentUpperV;
    currentLowerY = currentUpperY;
    localLowerV = 0;
    row++;
  }// while
}// renderOrthoSliceZ






SbVec3s & SoVolumeData::getPageSize()
{
  return PRIVATE(this)->pageSize;
}// getPageSize






/*************************** PIMPL-FUNCTIONS ********************************/

// FIXME: Perhaps there already is a function somewhere in C or Coin
// that can test this easily?  31082002 torbjorv
bool SoVolumeDataP::check2n(int n)
{
  for (int i = 0; i < sizeof(int)*8; i++) {

    if (n & 1) {
      if (n != 1) 
        return false;
      else
        return true;
    }// if

    n >>= 1;
  }// for
  return true;
}// Check2n



SoGLImage * SoVolumeDataP::getGLImagePageX(int sliceIdx, int col, int row)
{
  assert(data);

  // Valid page?
  if ((col >= pageSize[2]) || (row >= pageSize[1]))
    return NULL;

  // First page ever?
  if (!pageX) {
    pageX = new SoGLImage*[dimensions[0]*pageSize[1]*pageSize[2]];
    memset( pageX, 
            0, 
            sizeof(SoGLImage*)*dimensions[0]*pageSize[1]*pageSize[2]);
  }// if

  // Texture already generated?
  if (!pageX[sliceIdx*pageSize[2]*pageSize[1] + row*pageSize[2] + col]) {
    const unsigned char * RGBATexture = 
        (unsigned char *)getRGBAPageX(sliceIdx, col, row);
    
    SoGLImage * GLImage = new SoGLImage();
    GLImage->setData( RGBATexture, 
                      SbVec2s(pageSize[2], pageSize[1]), 
                      4,
                      SoGLImage::CLAMP_TO_EDGE,
                      SoGLImage::CLAMP_TO_EDGE);


    pageX[sliceIdx*pageSize[2]*pageSize[1] + row*pageSize[2] + col] = GLImage;
  }// if

  return pageX[sliceIdx*pageSize[2]*pageSize[1] + row*pageSize[2] + col];
}// getGLImagePageX



SoGLImage * SoVolumeDataP::getGLImagePageY(int sliceIdx, int col, int row)
{
  assert(data);

  // Valid page?
  if ((col >= pageSize[0]) || (row >= pageSize[2]))
    return NULL;

  // First page ever?
  if (!pageY) {
    pageY = new SoGLImage*[dimensions[1]*pageSize[0]*pageSize[2]];
    memset( pageY, 
            0, 
            sizeof(SoGLImage*)*dimensions[1]*pageSize[0]*pageSize[2]);
  }// if

  // Texture already generated?
  if (!pageY[sliceIdx*pageSize[0]*pageSize[2] + row*pageSize[0] + col])
  {
    const unsigned char * RGBATexture = 
        (unsigned char *)getRGBAPageY(sliceIdx, col, row);
    
    SoGLImage * GLImage = new SoGLImage();
    GLImage->setData( RGBATexture, 
                      SbVec2s(pageSize[0], pageSize[2]), 
                      4,
                      SoGLImage::CLAMP_TO_EDGE,
                      SoGLImage::CLAMP_TO_EDGE);

    pageY[sliceIdx*pageSize[0]*pageSize[2] + row*pageSize[0] + col] = GLImage;
  }// if

  return pageY[sliceIdx*pageSize[0]*pageSize[2] + row*pageSize[0] + col];
}// getGLImagePageY






SoGLImage * SoVolumeDataP::getGLImagePageZ(int sliceIdx, int col, int row)
{
  assert(data);

  if ((col >= pageSize[0]) || (row >= pageSize[1]))
    return NULL;

  if (!pageZ) {
    pageZ = new SoGLImage*[dimensions[2]*pageSize[0]*pageSize[1]];
    memset( pageZ, 
            0, 
            sizeof(SoGLImage*)*dimensions[2]*pageSize[0]*pageSize[1]);
  }// if


  // Texture already generated?
  void * p = pageZ[sliceIdx*pageSize[0]*pageSize[1] + row*pageSize[0] + col];
  if (!pageZ[sliceIdx*pageSize[0]*pageSize[1] + row*pageSize[0] + col])
  {
    unsigned char * RGBATexture = 
        (unsigned char *)getRGBAPageZ(sliceIdx, col, row);
    
    SoGLImage * GLImage = new SoGLImage();
    GLImage->setData( RGBATexture, 
                      SbVec2s(pageSize[0], pageSize[1]), 
                      4,
                      SoGLImage::CLAMP_TO_EDGE,
                      SoGLImage::CLAMP_TO_EDGE);

    pageZ[sliceIdx*pageSize[0]*pageSize[1] + row*pageSize[0] + col] = GLImage;
  }// if

  return pageZ[sliceIdx*pageSize[0]*pageSize[1] + row*pageSize[0] + col];
}// getGLImagePageZ




// Returns a texture with Z as horisontal and Y as vertical axis
// Assumes that the provided data is in RGBA-form
// Caller deletes of course. This function may be optimized 
// when coming to multiplications and stuff. Could just as well 
// use additions, but it'll be more messy. The cache-misses will 
// dominate anyway. 
// FIXME: This code should REALLY be made more readable. 
// torbjorv 08312002
const void * 
SoVolumeDataP::getRGBAPageX(int sliceIdx, int col, int row)
{
  int * texture = 
    new int[dimensions[2]*dimensions[1]];
  int * intData = (int*)data;

  int i = 0;
  for (int y = pageSize[1]*row; y < pageSize[1]*(row + 1); y++)
    for (int z = pageSize[2]*col; z < pageSize[2]*(col + 1); z++) {
      texture[i] = 
        intData[z*dimensions[0]*dimensions[1] + dimensions[0]*y + sliceIdx];
      i++;
    }// for

  return texture;
}// getRGBAPageX



// Returns a texture with X as horisontal and Z as vertical axis
// Assumes that the provided data is in RGBA-form
// Caller deletes of course.
// FIXME: This code should REALLY be made more readable. 
// torbjorv 08312002
const void * 
SoVolumeDataP::getRGBAPageY(int sliceIdx, int col, int row)
{
  int * texture = 
    new int[dimensions[0]*dimensions[2]];
  int * intData = (int*)data;

  int i = 0;
  for (int z = pageSize[2]*row; z < pageSize[2]*(row + 1); z++)
    for (int x = pageSize[0]*col; x < pageSize[0]*(col + 1); x++) {
      texture[i] = 
        intData[z*dimensions[0]*dimensions[1] + dimensions[0]*sliceIdx + x];
      i++;
    }// for

  return texture;
}// getRGBAPageY



// Returns a texture with X as horisontal and Y as vertical axis
// Assumes that the provided data is in RGBA-form
// Caller deletes of course.
const void * 
SoVolumeDataP::getRGBAPageZ(int sliceIdx, int col, int row)
{
  int * texture = 
    new int[dimensions[0]*dimensions[1]];
  int * intData = (int*)data;

  int i = 0;
  for (int y = pageSize[1]*row; y < pageSize[1]*(row + 1); y++)
    for (int x = pageSize[0]*col; x < pageSize[1]*(col + 1); x++) {
      texture[i] = 
        intData[sliceIdx*dimensions[0]*dimensions[1] + dimensions[0]*y + x];
      i++;
    }// for

  return texture;
}// getRGBAPageZ



const void * 
SoVolumeDataP::getRGBAPage(int sliceIdx, SoVolumeData::Axis axis, int col, int row)
{
  switch (axis)
  {
    case SoVolumeData::X:
      return getRGBAPageX(sliceIdx, col, row);
      break;

    case SoVolumeData::Y:
      return getRGBAPageY(sliceIdx, col, row);
      break;

    case SoVolumeData::Z:
      return getRGBAPageZ(sliceIdx, col, row);
      break;
  }// switch

  return NULL;
}// getRGBAPage


void SoVolumeDataP::releaseTextures()
{
  releaseTexturesX();
  releaseTexturesY();
  releaseTexturesZ();
}// releaseTextures


void SoVolumeDataP::releaseTexturesX()
{
  int i;
  if ( pageX ) {
    for (i = 0; i < dimensions[0]; i++) {
      if (pageX[i]) {
        pageX[i]->unref();
        pageX[i] = NULL;
      }
    }// for
  }

  delete [] pageX;
  pageX = NULL;
}// releaseTextures

void SoVolumeDataP::releaseTexturesY()
{
  int i;
  if ( pageY ) {
    for (i = 0; i < dimensions[1]; i++) {
      if (pageY[i]) {
        pageY[i]->unref();
        pageY[i] = NULL;
      }
    }// for
  }

  delete [] pageY;
  pageY = NULL;
}// releaseTextures

void SoVolumeDataP::releaseTexturesZ()
{
  int i;
  if ( pageZ ) {
    for (i = 0; i < dimensions[2]; i++) {
      if (pageZ[i]) {
        pageZ[i]->unref();
        pageZ[i] = NULL;
      }
    }// for
  }

  delete [] pageZ;
  pageZ = NULL;
}// releaseTextures



/****************** UNIMPLEMENTED FUNCTIONS ******************************/



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
