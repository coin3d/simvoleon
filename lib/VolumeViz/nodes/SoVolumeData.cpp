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
#include <VolumeViz/readers/SoVRMemReader.h>

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

    slicesX = NULL;
    slicesY = NULL;
    slicesZ = NULL;

    volumeSize = SbBox3f(-1, -1, -1, 1, 1, 1);
    dimensions = SbVec3s(0, 0, 0);
    pageSize = SbVec3s(64, 64, 64);

    maxTexels = 64*1024*1024;
    currentTexels = 0;
    currentPages = 0;
    tick = 0;
    extensionsInitialized = false;

    VRMemReader = NULL;
    reader = NULL;
  }// constructor

  ~SoVolumeDataP()
  {
    delete VRMemReader;
    releaseSlices();
  }// destructor

  SbVec3s dimensions;
  SbBox3f volumeSize;
  SbVec3s pageSize;
  SoVolumeData::DataType dataType;

  SoVRMemReader * VRMemReader;
  SoVolumeReader * reader;

  long tick;
  bool extensionsInitialized;
  int maxTexels;
  int currentTexels;
  int currentPages;

  SoVolumeDataSlice **slicesX;
  SoVolumeDataSlice **slicesY;
  SoVolumeDataSlice **slicesZ;

  SoVolumeDataSlice * getSliceX(int sliceIdx);
  SoVolumeDataSlice * getSliceY(int sliceIdx);
  SoVolumeDataSlice * getSliceZ(int sliceIdx);

  void releaseSlices();
  void releaseSlicesX();
  void releaseSlicesY();
  void releaseSlicesZ();

  void freeTexels(int desired);
  void releaseLRUPage();

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
  if (PRIVATE(this)->VRMemReader)
    PRIVATE(this)->VRMemReader->setVolumeSize(size);
}// setVolumeSize


SbBox3f &
SoVolumeData::getVolumeSize() 
{ 

  return PRIVATE(this)->volumeSize; 
}


SbVec3s &
SoVolumeData::getDimensions() 
{ 
  return PRIVATE(this)->dimensions; 
}



// FIXME: If size != 2^n these functions should extend to the nearest size. 
// torbjorv 07292002
void 
SoVolumeData::setVolumeData(const SbVec3s &dimensions, 
                            const void *data, 
                            SoVolumeData::DataType type) 
{

  PRIVATE(this)->VRMemReader = new SoVRMemReader;
  PRIVATE(this)->VRMemReader->setData(dimensions, 
                                      data, 
                                      PRIVATE(this)->volumeSize,
                                      type);
  this->setReader(PRIVATE(this)->VRMemReader);
 
  if (PRIVATE(this)->pageSize[0] > dimensions[0])
    PRIVATE(this)->pageSize[0] = dimensions[0];
  if (PRIVATE(this)->pageSize[1] > dimensions[1])
    PRIVATE(this)->pageSize[1] = dimensions[1];
  if (PRIVATE(this)->pageSize[2] > dimensions[2])
    PRIVATE(this)->pageSize[2] = dimensions[2];
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

  if (rebuildX) PRIVATE(this)->releaseSlicesX();
  if (rebuildY) PRIVATE(this)->releaseSlicesY();
  if (rebuildZ) PRIVATE(this)->releaseSlicesZ();
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



void SoVolumeData::renderOrthoSliceX(SoState * state, 
                                     SbBox2f &quad, 
                                     float x,
                                     int sliceIdx, 
                                     SbBox2f &textureCoords,
                                     SoTransferFunction * transferFunction)
{
  SbVec2f max, min;
  quad.getBounds(min, max);

  SoVolumeDataSlice * slice = 
    PRIVATE(this)->getSliceX(sliceIdx);

  PRIVATE(this)->currentTexels -= slice->numTexels;
  PRIVATE(this)->currentPages -= slice->numPages;

  slice->render(state,
                SbVec3f(x, min[1], min[0]),
                SbVec3f(x, min[1], max[0]),
                SbVec3f(x, max[1], max[0]),
                SbVec3f(x, max[1], min[0]),
                textureCoords,
                transferFunction,
                PRIVATE(this)->tick);

  PRIVATE(this)->currentTexels += slice->numTexels;
  PRIVATE(this)->currentPages += slice->numPages;

  PRIVATE(this)->freeTexels(0);
}// renderOrthoSliceX

void SoVolumeData::renderOrthoSliceY(SoState * state, 
                                     SbBox2f &quad, 
                                     float y,
                                     int sliceIdx, 
                                     SbBox2f &textureCoords,
                                     SoTransferFunction * transferFunction)
{

  SbVec2f max, min;
  quad.getBounds(min, max);

  SoVolumeDataSlice * slice = 
    PRIVATE(this)->getSliceY(sliceIdx);

  PRIVATE(this)->currentTexels -= slice->numTexels;
  PRIVATE(this)->currentPages -= slice->numPages;

  slice->render(state,
                SbVec3f(min[0], y, min[1]),
                SbVec3f(max[0], y, min[1]),
                SbVec3f(max[0], y, max[1]),
                SbVec3f(min[0], y, max[1]),
                textureCoords,
                transferFunction,
                PRIVATE(this)->tick);

  PRIVATE(this)->currentTexels += slice->numTexels;
  PRIVATE(this)->currentPages += slice->numPages;

  PRIVATE(this)->freeTexels(0);
}// renderOrthoSliceY



void SoVolumeData::renderOrthoSliceZ(SoState * state, 
                                     SbBox2f &quad, 
                                     float z,
                                     int sliceIdx, 
                                     SbBox2f &textureCoords,
                                     SoTransferFunction * transferFunction)
{

  SbVec2f max, min;
  quad.getBounds(min, max);

  SoVolumeDataSlice * slice = 
    PRIVATE(this)->getSliceZ(sliceIdx);

  PRIVATE(this)->currentTexels -= slice->numTexels;
  PRIVATE(this)->currentPages -= slice->numPages;

  slice->render(state,
                SbVec3f(min[0], min[1], z),
                SbVec3f(max[0], min[1], z),
                SbVec3f(max[0], max[1], z),
                SbVec3f(min[0], max[1], z),
                textureCoords,
                transferFunction,
                PRIVATE(this)->tick);

  PRIVATE(this)->currentTexels += slice->numTexels;
  PRIVATE(this)->currentPages += slice->numPages;

  PRIVATE(this)->freeTexels(0);
}// renderOrthoSliceZ






SbVec3s & SoVolumeData::getPageSize()
{
  return PRIVATE(this)->pageSize;
}// getPageSize






void 
SoVolumeData::setTexMemorySize(int size) 
{
  PRIVATE(this)->maxTexels = size;
}// setTexMemorySize



void 
SoVolumeData::setReader(SoVolumeReader * reader) 
{
  PRIVATE(this)->reader = reader;

  reader->getDataChar(PRIVATE(this)->volumeSize,
                      PRIVATE(this)->dataType, 
                      PRIVATE(this)->dimensions);
}// setReader




/*************************** PIMPL-FUNCTIONS ********************************/


SoVolumeDataSlice * SoVolumeDataP::getSliceX(int sliceIdx)
{
  // Valid slice?
  if (sliceIdx >= this->dimensions[0]) return NULL;

  // First SoVolumeDataPage ever?
  if (!slicesX) {
    slicesX = new SoVolumeDataSlice*[dimensions[0]];
    memset( slicesX, 
            0, 
            sizeof(SoVolumeDataSlice*)*dimensions[0]);
  }// if

  if (!slicesX[sliceIdx]) {
    SoVolumeDataSlice * newSlice = new SoVolumeDataSlice;
    newSlice->init(this->reader, 
                   sliceIdx, 
                   SoVolumeRendering::X, 
                   SbVec2s(this->pageSize[2], this->pageSize[1]));

    slicesX[sliceIdx] = newSlice;
  }// if

  return slicesX[sliceIdx];
}// getSliceX


SoVolumeDataSlice * SoVolumeDataP::getSliceY(int sliceIdx)
{
  // Valid slice?
  if (sliceIdx >= this->dimensions[1]) return NULL;

  // First SoVolumeDataPage ever?
  if (!slicesY) {
    slicesY = new SoVolumeDataSlice*[dimensions[1]];
    memset( slicesY, 
            0, 
            sizeof(SoVolumeDataSlice*)*dimensions[1]);
  }// if

  if (!slicesY[sliceIdx]) {
    SoVolumeDataSlice * newSlice = new SoVolumeDataSlice;
    newSlice->init(this->reader, 
                   sliceIdx, 
                   SoVolumeRendering::Y, 
                   SbVec2s(this->pageSize[0], this->pageSize[2]));

    slicesY[sliceIdx] = newSlice;
  }// if

  return slicesY[sliceIdx];
}// getSliceZ



SoVolumeDataSlice * SoVolumeDataP::getSliceZ(int sliceIdx)
{
  // Valid slice?
  if (sliceIdx >= this->dimensions[2]) return NULL;

  // First SoVolumeDataPage ever?
  if (!slicesZ) {
    slicesZ = new SoVolumeDataSlice*[dimensions[2]];
    memset( slicesZ, 
            0, 
            sizeof(SoVolumeDataSlice*)*dimensions[2]);
  }// if

  if (!slicesZ[sliceIdx]) {
    SoVolumeDataSlice * newSlice = new SoVolumeDataSlice;
    newSlice->init(this->reader, 
                   sliceIdx, 
                   SoVolumeRendering::Z, 
                   SbVec2s(this->pageSize[0], this->pageSize[1]));

    slicesZ[sliceIdx] = newSlice;
  }// if

  return slicesZ[sliceIdx];
}// getSliceZ


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




void SoVolumeDataP::releaseSlices()
{
  releaseSlicesX();
  releaseSlicesY();
  releaseSlicesZ();
}// releasePages

void SoVolumeDataP::freeTexels(int desired)
{
  if (desired > maxTexels) return;

  while ((maxTexels - currentTexels) < desired)
    releaseLRUPage();
}// freeTexels


void SoVolumeDataP::releaseLRUPage()
{
  SoVolumeDataPage * LRUPage = NULL;
  SoVolumeDataPage * tmpPage = NULL;
  SoVolumeDataSlice * LRUSlice = NULL;

  // Searching for LRU page among X-slices
  if (this->slicesX) {
    for (int i = 0; i < this->dimensions[0]; i++) {
      tmpPage = NULL;
      if (slicesX[i]) {
        tmpPage = slicesX[i]->getLRUPage();
        if (LRUPage == NULL) {
          LRUSlice = slicesX[i];
          LRUPage = tmpPage;
        }// if
        else
          if (tmpPage != NULL)
            if (tmpPage->lastuse < LRUPage->lastuse) {
              LRUSlice = slicesX[i];
              LRUPage = tmpPage;
            }// if
      }// if
    }//for
  }//if

  // Searching for LRU page among X-slices
  if (this->slicesY) {
    for (int i = 0; i < this->dimensions[1]; i++) {
      tmpPage = NULL;
      if (slicesY[i]) {
        tmpPage = slicesY[i]->getLRUPage();
        if (LRUPage == NULL) {
          LRUSlice = slicesY[i];
          LRUPage = tmpPage;
        }// if
        else
          if (tmpPage != NULL)
            if (tmpPage->lastuse < LRUPage->lastuse) {
              LRUSlice = slicesY[i];
              LRUPage = tmpPage;
            }// if
      }// if
    }//for
  }//if

  // Searching for LRU page among X-slices
  if (this->slicesZ) {
    for (int i = 0; i < this->dimensions[2]; i++) {
      tmpPage = NULL;
      if (slicesZ[i]) {
        tmpPage = slicesZ[i]->getLRUPage();
        if (LRUPage == NULL) {
          LRUSlice = slicesZ[i];
          LRUPage = tmpPage;
        }// if
        else
          if (tmpPage != NULL)
            if (tmpPage->lastuse < LRUPage->lastuse) {
              LRUSlice = slicesZ[i];
              LRUPage = tmpPage;
            }// if
      }// if
    }//for
  }//if

  this->currentTexels -= LRUSlice->numTexels;
  this->currentPages -= LRUSlice->numPages;

  LRUSlice->releasePage(LRUPage);

  this->currentTexels += LRUSlice->numTexels;
  this->currentPages += LRUSlice->numPages;
}// relaseLRUPage



void SoVolumeDataP::releaseSlicesX()
{
}// releaseSlicesX


void SoVolumeDataP::releaseSlicesY()
{
}// releaseSlicesY

void SoVolumeDataP::releaseSlicesZ()
{
}// releaseSlicesZ

/****************** UNIMPLEMENTED FUNCTIONS ******************************/



SbBool 
SoVolumeData::getVolumeData(SbVec3s &dimensions, 
                            void *&data, 
                            SoVolumeData::DataType &type) 
{ return FALSE; }

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
