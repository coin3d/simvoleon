/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http:// www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/


#include <VolumeViz/nodes/SoVolumeData.h>
#include <memory.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <VolumeViz/elements/SoVolumeDataElement.h>
#include <Inventor/elements/SoGLCacheContextElement.h>
#include <VolumeViz/misc/SoVolumeDataPage.h>

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif // HAVE_WINDOWS_H*/

#include <GL/gl.h>


/*

- LOAD_ALL: Builds as many pages as possible, limited by the texellimit. 
  This is done whenever StorageHint changes and building is required. 
  Both SetData and changes in StorageHint must trigger the build. 
  The textures are NOT uploaded to OpenGL here. They're uploaded in VolumeData's 
  GLRender. 
- Create SoVolumeDataSlice-class:
    * SbVec2s size (columns, rows)
    * SoVolumeDataPage * getPage(col, row, SoTransferFunction *, SoVolumeReader *)
- Create SoVolumeDataPage-struct
    * storage (opengl, memory) 
    * format (GL_INDEX8, GL_RGBA)
    * size
    * textureName
    * lastUse
    * void * data;
    * unsigned long lastuse;
    * setActive(long tick)
    * float * palette 
    * setData()
* Create SoVRMemReader-class
    * 
*/


//Extensions for compressed textures...
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXIMAGE3DARBPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXIMAGE2DARBPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXIMAGE1DARBPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * PFNGLGETCOMPRESSEDTEXIMAGEARBPROC) (GLenum target, GLint level, void *img);

PFNGLCOMPRESSEDTEXIMAGE3DARBPROC glCompressedTexImage3DARB;
PFNGLCOMPRESSEDTEXIMAGE2DARBPROC glCompressedTexImage2DARB;
PFNGLCOMPRESSEDTEXIMAGE1DARBPROC glCompressedTexImage1DARB;
PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC glCompressedTexSubImage3DARB;
PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC	glCompressedTexSubImage2DARB;
PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC	glCompressedTexSubImage1DARB;
PFNGLGETCOMPRESSEDTEXIMAGEARBPROC	glGetCompressedTexImageARB;

//Extensions for paletted textures...
#ifndef GL_EXT_paletted_texture
#define GL_EXT_paletted_texture
#define GL_COLOR_INDEX1_EXT (0x80E2)
#define GL_COLOR_INDEX2_EXT (0x80E3)
#define GL_COLOR_INDEX4_EXT (0x80E4)
#define GL_COLOR_INDEX8_EXT (0x80E5)
#define GL_COLOR_INDEX12_EXT (0x80E6)
#define GL_COLOR_INDEX16_EXT (0x80E7)
#define GL_TEXTURE_INDEX_SIZE_EXT (0x80ED)

#define GL_COLOR_TABLE_FORMAT_EXT (0x80D8)
#define GL_COLOR_TABLE_WIDTH_EXT (0x80D9)
#define GL_COLOR_TABLE_RED_SIZE_EXT (0x80DA)
#define GL_COLOR_TABLE_GREEN_SIZE_EXT (0x80DB)
#define GL_COLOR_TABLE_BLUE_SIZE_EXT (0x80DC)
#define GL_COLOR_TABLE_ALPHA_SIZE_EXT 80x80DD)
#define GL_COLOR_TABLE_LUMINANCE_SIZE_EXT (0x80DE)
#define GL_COLOR_TABLE_INTENSITY_SIZE_EXT (0x80DF)
#define GL_TEXTURE_INDEX_SIZE_EXT (0x80ED)
#endif 

typedef void (APIENTRY * PFNGLCOLORTABLEEXTPROC) (GLenum target, GLenum internalFormat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);
typedef void (APIENTRY * PFNGLCOLORSUBTABLEEXTPROC) (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data);
typedef void (APIENTRY * PFNGLGETCOLORTABLEEXTPROC) (GLenum target, GLenum format, GLenum type, GLvoid *data);
typedef void (APIENTRY * PFNGLGETCOLORTABLEPARAMETERIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLGETCOLORTABLEPARAMETERFVEXTPROC) (GLenum target, GLenum pname, GLfloat *params);

PFNGLCOLORTABLEEXTPROC glColorTableEXT;
PFNGLCOLORSUBTABLEEXTPROC glColorSubTableEXT;
PFNGLGETCOLORTABLEEXTPROC glGetColorTableEXT;
PFNGLGETCOLORTABLEPARAMETERIVEXTPROC glGetColorTableParameterivEXT;
PFNGLGETCOLORTABLEPARAMETERFVEXTPROC glGetColorTableParameterfvEXT;




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

    maxTexels = 64*1024*1024;
    currentTexels = 0;
    currentPages = 0;

    tick = 0;
    extensionsInitialized = false;
  }// constructor

  ~SoVolumeDataP()
  {
    releasePages();
  }// destructor

  const void * data;

  SbVec3s dimensions;
  SbBox3f volumeSize;
  SbVec3s pageSize;

  long tick;
  bool extensionsInitialized;

  int maxTexels;
  int currentTexels;
  int currentPages;
  SoVolumeData::DataType dataType;


  // Pointers to arrays with pointers. Each array contains every SoVolumeDataPage along 
  // that axis. If the pagesize generates 16 pages each slice along Z-axis, 
  // SoVolumeDataPage (col, row) in slice n would be positioned at 
  // slice[n*pageSize[0]*pageSize[1] + row*pageSize[0] + col]
  // Use get...SoVolumeDataPage-functions to retreive the pages. 
  SoVolumeDataPage **pageX;
  SoVolumeDataPage **pageY;
  SoVolumeDataPage **pageZ;

  SoVolumeDataPage * getPage(int sliceIdx, SoVolumeData::Axis axis, int col, int row);
  const void * getRGBAPage(int sliceIdx, SoVolumeData::Axis axis, int col, int row);
  const void * getRGBAPageX(int sliceIdx, int col, int row);
  const void * getRGBAPageY(int sliceIdx, int col, int row);
  const void * getRGBAPageZ(int sliceIdx, int col, int row);

  // FIXME: Implement support for other pixelformats. torbjorv 08312002
  SoVolumeDataPage * getPageX(int sliceIdx, int col, int row);
  SoVolumeDataPage * getPageY(int sliceIdx, int col, int row);
  SoVolumeDataPage * getPageZ(int sliceIdx, int col, int row);

  void releasePages();
  void releasePagesX();
  void releasePagesY();
  void releasePagesZ();

  void freeTexels(int desired);
  void releaseOldestPage();

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
                            const void *data, 
                            SoVolumeData::DataType type) 
{
  
  PRIVATE(this)->data = data;
  PRIVATE(this)->dimensions = dimensions;
  PRIVATE(this)->dataType = type;

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

  if (size[0] < 4) size[0] = 4;
  if (size[1] < 4) size[1] = 4;
  if (size[2] < 4) size[2] = 4;


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

  if (rebuildX) PRIVATE(this)->releasePagesX();
  if (rebuildY) PRIVATE(this)->releasePagesY();
  if (rebuildZ) PRIVATE(this)->releasePagesZ();
}// setPageSize




/*!
  Coin method.
*/
void
SoVolumeData::GLRender(SoGLRenderAction * action)
{
  SoVolumeDataElement::setVolumeData(action->getState(), this, this);
  PRIVATE(this)->tick++;
  printf( "currentPages: %06d  currentTexels = %010d\n", 
          PRIVATE(this)->currentPages, 
          PRIVATE(this)->currentTexels);


  if (!PRIVATE(this)->extensionsInitialized) {

    // Compressed texture extensions
    glCompressedTexImage3DARB = 
      (PFNGLCOMPRESSEDTEXIMAGE3DARBPROC)wglGetProcAddress("glCompressedTexImage3DARB");
    glCompressedTexImage2DARB = 
      (PFNGLCOMPRESSEDTEXIMAGE2DARBPROC)wglGetProcAddress("glCompressedTexImage2DARB");


    // Paletted texture extensions
    glColorTableEXT =
      (PFNGLCOLORTABLEEXTPROC)wglGetProcAddress("glColorTableEXT");

    glColorSubTableEXT =
      (PFNGLCOLORSUBTABLEEXTPROC)wglGetProcAddress("glColorSubTableEXT");

    glGetColorTableEXT =     
      (PFNGLGETCOLORTABLEEXTPROC)wglGetProcAddress("glGetColorTableEXT");

    glGetColorTableParameterivEXT = 
      (PFNGLGETCOLORTABLEPARAMETERIVEXTPROC)wglGetProcAddress("glGetColorTableParameterivEXT");
  
    glGetColorTableParameterfvEXT = 
      (PFNGLGETCOLORTABLEPARAMETERFVEXTPROC)wglGetProcAddress("glGetColorTableParameterfvEXT"); 

    PRIVATE(this)->extensionsInitialized = true;
  }// if
}// GLRender



SoVolumeDataPage *
SoVolumeDataP::getPage(int sliceIdx, SoVolumeData::Axis axis, int col, int row)
{
  // FIXME: Need to implement support for different pixelformats. 
  // torbjorv 07122002

  switch (axis)
  {
    case SoVolumeData::X: return this->getPageX(sliceIdx, col, row);
                          break;  

    case SoVolumeData::Y: return this->getPageY(sliceIdx, col, row);
                          break;

    case SoVolumeData::Z: return this->getPageZ(sliceIdx, col, row);
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
      SoVolumeDataPage * page = 
        PRIVATE(this)->getPageX(sliceIdx, col, row);
      page->setActivePage(PRIVATE(this)->tick);

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
      SoVolumeDataPage * page = 
        PRIVATE(this)->getPageY(sliceIdx, col, row);
      page->setActivePage(PRIVATE(this)->tick);

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
      SoVolumeDataPage * page = 
        PRIVATE(this)->getPageZ(sliceIdx, col, row);
      page->setActivePage(PRIVATE(this)->tick);

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






void 
SoVolumeData::setTexMemorySize(int size) 
{
  PRIVATE(this)->maxTexels = size;
}


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



SoVolumeDataPage * SoVolumeDataP::getPageX(int sliceIdx, int col, int row)
{
  assert(data);

  // Valid SoVolumeDataPage?
  if ((col >= (dimensions[2]/pageSize[2])) || 
      (row >= (dimensions[1]/pageSize[1])))
    return NULL;

  // First SoVolumeDataPage ever?
  if (!pageX) {
    pageX = new SoVolumeDataPage*[dimensions[0]*pageSize[1]*pageSize[2]];
    memset( pageX, 
            0, 
            sizeof(SoVolumeDataPage*)*dimensions[0]*pageSize[1]*pageSize[2]);
  }// if

  // Texture already generated?
  if (!pageX[sliceIdx*pageSize[2]*pageSize[1] + row*pageSize[2] + col]) {

    freeTexels(pageSize[2]*pageSize[1]);

    unsigned char * RGBATexture = 
        (unsigned char *)getRGBAPageX(sliceIdx, col, row);
    
    SoVolumeDataPage * page = new SoVolumeDataPage();
    page->setData(SoVolumeDataPage::OPENGL,
                  RGBATexture, 
                  SbVec2s(pageSize[2], pageSize[1]), 
                  4,
                  GL_RGBA);

    delete [] RGBATexture;

    pageX[sliceIdx*pageSize[2]*pageSize[1] + row*pageSize[2] + col] = page;

    currentTexels += pageSize[2]*pageSize[1];
    currentPages++;
  }// if

  return pageX[sliceIdx*pageSize[2]*pageSize[1] + row*pageSize[2] + col];
}// getPageX



SoVolumeDataPage * SoVolumeDataP::getPageY(int sliceIdx, int col, int row)
{
  assert(data);

  // Valid SoVolumeDataPage?
  if ((col >= (dimensions[0]/pageSize[0])) || 
      (row >= (dimensions[2]/pageSize[2])))
    return NULL;

  // First SoVolumeDataPage ever?
  if (!pageY) {
    pageY = new SoVolumeDataPage*[dimensions[1]*pageSize[0]*pageSize[2]];
    memset( pageY, 
            0, 
            sizeof(SoVolumeDataPage*)*dimensions[1]*pageSize[0]*pageSize[2]);
  }// if

  // Texture already generated?
  if (!pageY[sliceIdx*pageSize[0]*pageSize[2] + row*pageSize[0] + col])
  {
    freeTexels(pageSize[0]*pageSize[2]);

    unsigned char * RGBATexture = 
        (unsigned char *)getRGBAPageY(sliceIdx, col, row);
    
    SoVolumeDataPage * page = new SoVolumeDataPage();
    page->setData(SoVolumeDataPage::OPENGL,
                  RGBATexture, 
                  SbVec2s(pageSize[0], pageSize[2]), 
                  4,
                  GL_RGBA);

    delete [] RGBATexture;

    pageY[sliceIdx*pageSize[0]*pageSize[2] + row*pageSize[0] + col] = page;

    currentTexels += pageSize[0]*pageSize[2];
    currentPages++;
  }// if

  return pageY[sliceIdx*pageSize[0]*pageSize[2] + row*pageSize[0] + col];
}// getPageY






SoVolumeDataPage * SoVolumeDataP::getPageZ(int sliceIdx, int col, int row)
{
  assert(data);

  if ((col >= (this->dimensions[0]/this->pageSize[0])) || 
      (row >= (this->dimensions[1]/this->pageSize[1])))
    return NULL;

  if (!pageZ) {
    pageZ = new SoVolumeDataPage*[dimensions[2]*pageSize[0]*pageSize[1]];
    memset( pageZ, 
            0, 
            sizeof(SoVolumeDataPage*)*dimensions[2]*pageSize[0]*pageSize[1]);
  }// if


  // Texture already generated?
  void * p = pageZ[sliceIdx*pageSize[0]*pageSize[1] + row*pageSize[0] + col];
  if (!pageZ[sliceIdx*pageSize[0]*pageSize[1] + row*pageSize[0] + col])
  {
    freeTexels(pageSize[0]*pageSize[1]);

    unsigned char * RGBATexture = 
        (unsigned char *)getRGBAPageZ(sliceIdx, col, row);
    
    SoVolumeDataPage * page = new SoVolumeDataPage();
    page->setData(SoVolumeDataPage::OPENGL,
                  RGBATexture, 
                  SbVec2s(pageSize[0], pageSize[1]), 
                  4,
                  GL_RGBA);

    delete [] RGBATexture;

    pageZ[sliceIdx*pageSize[0]*pageSize[1] + row*pageSize[0] + col] = page;

    currentTexels += pageSize[0]*pageSize[1];
    currentPages++;
  }// if

  return pageZ[sliceIdx*pageSize[0]*pageSize[1] + row*pageSize[0] + col];
}// getPageZ




// Returns a texture with Z as horisontal and Y as vertical axis
// Assumes that the provided data is in RGBA-form
// Caller deletes of course. 

// This function and the similar functions for Y- and Z-axis should
// be fairly optimized. They're unrolled four times as the pages 
// are restricted downwards to a size of 4. 
const void * 
SoVolumeDataP::getRGBAPageX(int sliceIdx, int col, int row)
{
  int * texture = 
    new int[pageSize[2]*pageSize[1]];
  int * intData = (int*)data;

  int out = 0;
  int xOffset = sliceIdx;
  int yOffset = pageSize[1]*row*dimensions[0];
  int yLimit = pageSize[1]*(row + 1)*dimensions[0];
  int zAdd = dimensions[0]*dimensions[1];
  int zAdd4 = zAdd*4;
  int zStart = pageSize[2]*col*dimensions[0]*dimensions[1];

  while (yOffset < yLimit) {
    int zOffset = zStart + xOffset + yOffset;
    int zLimit  = pageSize[2]*(col + 1)*dimensions[0]*dimensions[1] 
                + xOffset + yOffset;
    while (zOffset < zLimit) {
      texture[out + 0] = intData[zOffset + 0*zAdd];
      texture[out + 1] = intData[zOffset + 1*zAdd];
      texture[out + 2] = intData[zOffset + 2*zAdd];
      texture[out + 3] = intData[zOffset + 3*zAdd];
      out += 4;
      zOffset += zAdd4;
    }// while
    yOffset += dimensions[0];
  }// while

  return texture;
}// getRGBAPageX



// Returns a texture with X as horisontal and Z as vertical axis
// Assumes that the provided data is in RGBA-form
// Caller deletes of course.
const void * 
SoVolumeDataP::getRGBAPageY(int sliceIdx, int col, int row)
{
  int * texture = 
    new int[pageSize[0]*pageSize[2]];
  int * intData = (int*)data;

  int out = 0;
  int yOffset = sliceIdx*dimensions[0];
  int zOffset = pageSize[2]*dimensions[0]*dimensions[1]*row + yOffset;
  int zLimit = dimensions[0]*dimensions[1]*(row + 1)*pageSize[2] + yOffset;

  while (zOffset < zLimit) {
    int xOffset = pageSize[0]*col + zOffset;
    int xLimit = pageSize[0]*(col + 1) + zOffset;
    while (xOffset < xLimit) {
      texture[out] = intData[xOffset];
      texture[out + 1] = intData[xOffset + 1];
      texture[out + 2] = intData[xOffset + 2];
      texture[out + 3] = intData[xOffset + 3];
      out += 4;
      xOffset += 4;
    }// while
    zOffset += dimensions[0]*dimensions[1];
  }// while

  return texture;
}// getRGBAPageY



// Returns a texture with X as horisontal and Y as vertical axis
// Assumes that the provided data is in RGBA-form
// Caller deletes of course.
const void * 
SoVolumeDataP::getRGBAPageZ(int sliceIdx, int col, int row)
{
  int * texture = 
    new int[pageSize[0]*pageSize[1]];
  int * intData = (int*)data;

  int out = 0;
  int zOffset = sliceIdx*dimensions[0]*dimensions[1];
  int yOffset = pageSize[1]*row*dimensions[0] + zOffset;
  int yLimit = pageSize[1]*(row + 1)*dimensions[0] + zOffset;
  int xStart = pageSize[0]*col;
  while (yOffset < yLimit) {
    int xOffset = xStart + yOffset; 
    int xLimit = pageSize[0] + xStart + yOffset; 
    while (xOffset < xLimit) {
      texture[out] = intData[xOffset];
      texture[out + 1] = intData[xOffset + 1];
      texture[out + 2] = intData[xOffset + 2];
      texture[out + 3] = intData[xOffset + 3];
      out += 4;
      xOffset += 4;
    }// while

    // Next line of pixels
    yOffset += dimensions[0];
  }// while

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


void SoVolumeDataP::releasePages()
{
  releasePagesX();
  releasePagesY();
  releasePagesZ();
}// releasePages


void SoVolumeDataP::releasePagesX()
{
  int i;
  if ( pageX ) {
    for (i = 0; i < dimensions[0]; i++) {
      if (pageX[i]) {

        delete pageX[i];
        pageX[i] = NULL;
      }
    }// for
  }

  delete [] pageX;
  pageX = NULL;
}// releasePages

void SoVolumeDataP::releasePagesY()
{
  int i;
  if ( pageY ) {
    for (i = 0; i < dimensions[1]; i++) {
      if (pageY[i]) {
        delete pageY[i];
        pageY[i] = NULL;
      }
    }// for
  }

  delete [] pageY;
  pageY = NULL;
}// releasePages

void SoVolumeDataP::releasePagesZ()
{
  int i;
  if ( pageZ ) {
    for (i = 0; i < dimensions[2]; i++) {
      if (pageZ[i]) {
        delete pageZ[i];
        pageZ[i] = NULL;
      }
    }// for
  }

  delete [] pageZ;
  pageZ = NULL;
}// releasePages



void SoVolumeDataP::freeTexels(int desired)
{
  if (desired > maxTexels) return;

  while ((maxTexels - currentTexels) < desired)
    releaseOldestPage();
}// freeTexels


void SoVolumeDataP::releaseOldestPage()
{
  int i;

  unsigned int oldest = -1;
  SoVolumeDataPage **p = NULL;

  if (pageX)
    for (i = this->dimensions[0]*this->pageSize[1]*this->pageSize[2] - 1; i >= 0; i--) {
      if (pageX[i] )
        if (pageX[i]->lastuse < oldest) {
          p = &pageX[i];
          oldest = pageX[i]->lastuse;
        }// if
    }//for

  if (pageY)
    for (i = this->dimensions[1]*this->pageSize[0]*this->pageSize[2] - 1; i >= 0; i--) {
      if (pageY[i] )
        if (pageY[i]->lastuse < oldest) {
          p = &pageY[i];
          oldest = pageY[i]->lastuse;
        }// if
    }//for

  if (pageZ)
    for (i = this->dimensions[2]*this->pageSize[0]*this->pageSize[1] - 1; i >= 0; i--) {
      if (pageZ[i] )
        if (pageZ[i]->lastuse < oldest) {
          p = &pageZ[i];
          oldest = pageZ[i]->lastuse;
        }// if
    }//for

  this->currentPages--;
  this->currentTexels -= (*p)->size[0]*(*p)->size[1];
  delete *p;
  *p = NULL;
}// relaseOldestPage





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
