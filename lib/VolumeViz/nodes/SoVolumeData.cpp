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

DICTIONARY

  "Slice"      : One complete cut through the volume along an axis. 
  "Page"       : A segment of a slice. 
  "In-memory"  : In this context, "in-memory" means volumedata (pages) that's 
                 pulled out of the reader and run through a transferfunction,
                 thus ready to be rendered.
  LRU          : Least Recently Used




DATASTRUCTURES

  As several different renderingnodes may share the same volumedatanode, 
  a sharing mechanism for in-memory data is implemented. The volume is 
  partioned into slices along each of the three axis, and each slice is 
  segmented into pages. Each page is identified by it's slicenumber and 
  it's (x,y) position in the slice. Even though different renderingnodes 
  may share the same volumedata, they may have have individual 
  transferfunctions. A page shared by two renderingnodes with different 
  transferfunctions cannot share the same in-memory page. A page is 
  therefore also identified by the nodeId of it's transferfunctions. 
  All pages with same coordinates (sliceIdx, x, y) but different 
  transferfunctions are saved as a linked list. 


  
LRU-system

  To support large sets of volumedata, a simple memorymanagementsystem is
  implemented. The scenegraphs VolumeData-node contains a logical clock 
  that's incremented for each run through it's GLRender. All in-memory 
  pages are tagged with a timestamp at the time they're loaded. The 
  VolumeData-node has a maxlimit for the amount of HW/SW memory it should
  occupy, and whenever it exceeds this limit it throws out the page with
  the smallest timestamp. A simple LRU-cache. Of course, all pages' 
  timestamps are updated whenever they're rendered. 

  The datastructures does not work perfectly together with the LRU-cache. 
  Whenever a page is to be deallocated, a search through all slices and
  pages is required. The monitors for bytes allocated by in-memory pages
  are updated in a very non-elegant way (study i.e. 
  SoVolumeData::renderOrthoSliceX)

  Some sort of pagemanagersystem could be implemented to solve this 
  problem by storing all pages in a "flat" structure. This would look 
  nicer, but would be slower as long as (sliceIdx, x, y) can't be used
  as direct indices into the tables. 



USER INTERACTION

  As for now, the implementation loads only the pages that are needed for
  the current position of a SoROI/SoVolumeRender-node. Due to significant
  overhead when loading data and squeezing them through a transferfunction,
  the user experiences major delays in the visual response on her 
  interactions. TGS does not have functionality for dynamic loading of data,
  so the StorageHint-enum is extended with two elements (LOAD_MAX,
  LOAD_DYNAMIC). Currently, LOAD_DYNAMIC is the only one implemented of 
  the StorageHints, and is set as default. By using LOAD_MAX, the specified
  available memory should be filled to it's maximum. Pages should be 
  selected in an intelligent way, depending on the location of possible 
  SoROI-nodes (load the surrounding area). This should in most cases speed up 
  the visual feedback. 



PALETTED TEXTURES
  
  Paletted textures rocks. Depending on the size of the pages, it could 
  save significant amounts of memory. The current implementation uses
  individual palettes for each page. This can be both a good idea and a 
  terrible one. 

  Good: If the videocard supports palettes with different sizes. If, 
  for eaxmple, a page contains only one color, a 1-bit palette could be used
  and each pixel will occupy 1 bit of hardware memory. 

  Bad: If the videocard does NOT support palettes with different sizes. This
  means that each palette i.e. has to have 256 entries, and with RGBA colors
  the palette will occupy 256x4=1024 bytes. With pagesizes smaller than 64x64,
  this would make the palette occupy just as many bytes as the actual 
  pixeldata. If the videocard actually DOES support variable-size palettes, it
  could still be a bad idea. If all the pages require a 256-entry palette
  (or more) due to heavy colorvariations, the palettes would require a lot 
  of hardware memory.

  These problems may be solved by the use of several techniques. First of all, 
  there is an extension called GL_SHARED_PALETTE_EXT, that allows several 
  textures to share the same palette. A global palette for the entire volume 
  could be generated, resulting in some heavy precalculation and possibly loss
  of coloraccuracy, but saving a lot of memory. The best solution would 
  probably be a combination of local and global palettes. Local, if the 
  page consist entirely of one color. Global and shared whenever heavy 
  colorvariations occur. 



glColorTableEXT

  Study SoVolumeDataPage::setData. The code supports palettes of variable 
  sizes, exploiting the obvious advantages explained in the previous section.
  In between the uploading of palette and texture, there is a check of what 
  palettesize actually achieved. It seems like there's no guarantee that
  a videocard supports the different palettesizes/formats. If the following
  glTexImage2D tries to set a internal format that doesn't fit the 
  palettesize, the entire uploading could fail. At least it does on this 
  card (3DLabs Oxygen GVX1). The check for palettesize fixes this problem. 



MEMORYMANAGMENT

  The TGS-API contains a function setTexMemSize. It makes the client app
  able to specify the amount of memory the volumedatanode should occupy. 
  In TEXELS?!? As far as my neurons can figure out, this doesn't make 
  sense at all. This implementation supports this function, but also 
  provides a setHWMemorySize which specifies the maximum number of BYTES
  the volumedata should occupy of hardware memory. 



READERS

  Currently, only a reader of memoryprovided data is implemented 
  (SoVRMemReader). SoVolumeData uses the interface specified with 
  SoVolumeReader, and extensions with other readers should be straight
  forward. When running setReader or setVolumeData, only a pointer to the
  reader is stored. In other words, things could go bananas if the client
  app start mocking around with the reader's settings after a call to 
  setReader. If the reader is changed, setReader must be run over again. 
  This requirement differs from TGS as their implementation loads all data
  once specified a reader (I guess). 



RENDERING

  A SoVolumeRender is nothing but a SoROI which renders the entire
  volume. And this is how it should be implemented. But this is not how
  it is implemented now. :) The GLRender-function for both SoROI and 
  SoVolumeRender is more or less identical, and they should share some
  common renderfunction capable of rendering an entire volume. This should 
  in turn use a slicerendering-function similar to SoVolumeDataSlice::render. 



TODO
  
  No pickingfunctionality whatsoever is implemented. Other missing functions
  are tagged with FIXMEs. 

  Missing classes: SoObliqueSlice, SoOrthoSlice, all readers, all details. 
  


REFACTORING

  - hva jeg synes er mest gjenbrukbart ved en refaktorering


*/

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
    maxBytesHW = 1024*1024*16;
    numTexels = 0;
    numPages = 0;
    numBytesSW = 0;
    numBytesHW = 0;
    tick = 0;

    VRMemReader = NULL;
    reader = NULL;
    extensionsInitialized = false;
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
  int numTexels;
  int numPages;
  int numBytesSW;
  int maxBytesHW;
  int numBytesHW;

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
  void freeHWBytes(int desired);
  void managePages();
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



// FIXME: If size != 2^n these functions should extend to the nearest 
// accepted size. 
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

  // FIXME: Move this initialization to a proper home in Coin. 
  // torbjorv 08282002
  if (!PRIVATE(this)->extensionsInitialized) {

    // Compressed texture extensions
    glCompressedTexImage3DARB = 
      (PFNGLCOMPRESSEDTEXIMAGE3DARBPROC)
      wglGetProcAddress("glCompressedTexImage3DARB");
    glCompressedTexImage2DARB = 
      (PFNGLCOMPRESSEDTEXIMAGE2DARBPROC)
      wglGetProcAddress("glCompressedTexImage2DARB");


    // Paletted texture extensions
    glColorTableEXT =
      (PFNGLCOLORTABLEEXTPROC)
      wglGetProcAddress("glColorTableEXT");

    glColorSubTableEXT =
      (PFNGLCOLORSUBTABLEEXTPROC)
      wglGetProcAddress("glColorSubTableEXT");

    glGetColorTableEXT =     
      (PFNGLGETCOLORTABLEEXTPROC)
      wglGetProcAddress("glGetColorTableEXT");

    glGetColorTableParameterivEXT = 
      (PFNGLGETCOLORTABLEPARAMETERIVEXTPROC)
      wglGetProcAddress("glGetColorTableParameterivEXT");
  
    glGetColorTableParameterfvEXT = 
      (PFNGLGETCOLORTABLEPARAMETERFVEXTPROC)
      wglGetProcAddress("glGetColorTableParameterfvEXT"); 

    PRIVATE(this)->extensionsInitialized = true;
  }// if
}// GLRender



void 
SoVolumeData::renderOrthoSliceX(SoState * state, 
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

  PRIVATE(this)->numTexels -= slice->numTexels;
  PRIVATE(this)->numPages -= slice->numPages;
  PRIVATE(this)->numBytesSW -= slice->numBytesSW;
  PRIVATE(this)->numBytesHW -= slice->numBytesHW;

  slice->render(state,
                SbVec3f(x, min[1], min[0]),
                SbVec3f(x, min[1], max[0]),
                SbVec3f(x, max[1], max[0]),
                SbVec3f(x, max[1], min[0]),
                textureCoords,
                transferFunction,
                PRIVATE(this)->tick);

  PRIVATE(this)->numTexels += slice->numTexels;
  PRIVATE(this)->numPages += slice->numPages;
  PRIVATE(this)->numBytesSW += slice->numBytesSW;
  PRIVATE(this)->numBytesHW += slice->numBytesHW;

  PRIVATE(this)->managePages();
}// renderOrthoSliceX

void 
SoVolumeData::renderOrthoSliceY(SoState * state, 
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

  PRIVATE(this)->numTexels -= slice->numTexels;
  PRIVATE(this)->numPages -= slice->numPages;
  PRIVATE(this)->numBytesSW -= slice->numBytesSW;
  PRIVATE(this)->numBytesHW -= slice->numBytesHW;

  slice->render(state,
                SbVec3f(min[0], y, min[1]),
                SbVec3f(max[0], y, min[1]),
                SbVec3f(max[0], y, max[1]),
                SbVec3f(min[0], y, max[1]),
                textureCoords,
                transferFunction,
                PRIVATE(this)->tick);

  PRIVATE(this)->numTexels += slice->numTexels;
  PRIVATE(this)->numPages += slice->numPages;
  PRIVATE(this)->numBytesSW += slice->numBytesSW;
  PRIVATE(this)->numBytesHW += slice->numBytesHW;

  PRIVATE(this)->managePages();
}// renderOrthoSliceY



void 
SoVolumeData::renderOrthoSliceZ(SoState * state, 
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

  PRIVATE(this)->numTexels -= slice->numTexels;
  PRIVATE(this)->numPages -= slice->numPages;
  PRIVATE(this)->numBytesSW -= slice->numBytesSW;
  PRIVATE(this)->numBytesHW -= slice->numBytesHW;

  slice->render(state,
                SbVec3f(min[0], min[1], z),
                SbVec3f(max[0], min[1], z),
                SbVec3f(max[0], max[1], z),
                SbVec3f(min[0], max[1], z),
                textureCoords,
                transferFunction,
                PRIVATE(this)->tick);

  PRIVATE(this)->numTexels += slice->numTexels;
  PRIVATE(this)->numPages += slice->numPages;
  PRIVATE(this)->numBytesSW += slice->numBytesSW;
  PRIVATE(this)->numBytesHW += slice->numBytesHW;

  PRIVATE(this)->managePages();
}// renderOrthoSliceZ






SbVec3s & 
SoVolumeData::getPageSize()
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

void 
SoVolumeData::setHWMemorySize(int size) 
{
  PRIVATE(this)->maxBytesHW = size;
}// setTexMemorySize



/*************************** PIMPL-FUNCTIONS ********************************/


SoVolumeDataSlice * 
SoVolumeDataP::getSliceX(int sliceIdx)
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


SoVolumeDataSlice * 
SoVolumeDataP::getSliceY(int sliceIdx)
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



SoVolumeDataSlice * 
SoVolumeDataP::getSliceZ(int sliceIdx)
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
bool 
SoVolumeDataP::check2n(int n)
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




void 
SoVolumeDataP::releaseSlices()
{
  releaseSlicesX();
  releaseSlicesY();
  releaseSlicesZ();
}// releasePages

void 
SoVolumeDataP::freeTexels(int desired)
{
  if (desired > maxTexels) return;

  while ((maxTexels - numTexels) < desired)
    releaseLRUPage();
}// freeTexels


void 
SoVolumeDataP::releaseLRUPage()
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

  this->numTexels -= LRUSlice->numTexels;
  this->numPages -= LRUSlice->numPages;
  this->numBytesSW -= LRUSlice->numBytesSW;
  this->numBytesHW -= LRUSlice->numBytesHW;

  LRUSlice->releasePage(LRUPage);

  this->numTexels += LRUSlice->numTexels;
  this->numPages += LRUSlice->numPages;
  this->numBytesSW += LRUSlice->numBytesSW;
  this->numBytesHW += LRUSlice->numBytesHW;
}// relaseLRUPage



void 
SoVolumeDataP::releaseSlicesX()
{
  if (slicesX) {
    for (int i = 0; i < dimensions[0]; i++) {
      delete slicesX[i];
      slicesX[i] = NULL;
    }// for

    delete [] slicesX;
  }// if
}// releaseSlicesX





void 
SoVolumeDataP::releaseSlicesY()
{
  if (slicesY) {
    for (int i = 0; i < dimensions[1]; i++) {
      delete slicesY[i];
      slicesY[i] = NULL;
    }// for

    delete [] slicesY;
  }// if
}// releaseSlicesY





void 
SoVolumeDataP::releaseSlicesZ()
{
  if (slicesZ) {
    for (int i = 0; i < dimensions[2]; i++) {
      delete slicesZ[i];
      slicesX[i] = NULL;
    }// for

    delete [] slicesZ;
  }// if
}// releaseSlicesZ




void 
SoVolumeDataP::freeHWBytes(int desired)
{
  if (desired > maxBytesHW) return;

  while ((maxBytesHW - numBytesHW) < desired)
    releaseLRUPage();
}// freeHWBytes




void 
SoVolumeDataP::managePages()
{
  // Keep both measures within maxlimits
  freeHWBytes(0);
  freeTexels(0);
}// managePages


/****************** UNIMPLEMENTED FUNCTIONS ******************************/
// FIXME: Implement these functions. torbjorv 08282002


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
