/*!
  \class SoVolumeData VolumeViz/nodes/SoVolumeData.h
  \brief The interface for working with volume data sets.
  \ingroup volviz
*/

#include <VolumeViz/nodes/SoVolumeData.h>
#include <memory.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <VolumeViz/elements/SoVolumeDataElement.h>
#include <Inventor/elements/SoGLCacheContextElement.h>
#include <VolumeViz/readers/SoVRMemReader.h>
#include <Inventor/SbVec3s.h>
#include <VolumeViz/misc/SoVolumeDataPage.h>
#include <VolumeViz/misc/SoVolumeDataSlice.h>

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
  "In-memory"  : In this context, "in-memory" means volume data (pages) that's 
                 pulled out of the reader and run through a transfer function,
                 thus ready to be rendered.
  LRU          : Least Recently Used




DATA STRUCTURES

  As several different rendering nodes may share the same volume data
  node, a sharing mechanism for in-memory data is implemented. The
  volume is partitioned into slices along each of the three axis, and
  each slice is segmented into pages. Each page is identified by it's
  slice number and it's (x,y) position in the slice. Even though
  different rendering nodes may share the same volume data, they may
  have individual transfer functions. A page shared by two rendering
  nodes with different transfer functions cannot share the same
  in-memory page. A page is therefore also identified by the nodeId of
  it's transfer functions.  All pages with same coordinates (sliceIdx,
  x, y) but different transfer functions are saved as a linked list.


  
LRU-system

  To support large sets of volume data, a simple memory management
  system is implemented. The scenegraphs VolumeData-node contains a
  logical clock that's incremented for each run through it's
  GLRender. All in-memory pages are tagged with a timestamp at the
  time they're loaded. The VolumeData-node has a max limit for the
  amount of HW/SW memory it should occupy, and whenever it exceeds
  this limit it throws out the page with the smallest timestamp. A
  simple LRU-cache. Of course, all pages' timestamps are updated
  whenever they're rendered.

  The data structures does not work perfectly together with the
  LRU-cache.  Whenever a page is to be deallocated, a search through
  all slices and pages is required. The monitors for bytes allocated
  by in-memory pages are updated in a very non-elegant way (study i.e.
  SoVolumeData::renderOrthoSliceX).

  Some sort of page manager system could be implemented to solve this
  problem by storing all pages in a "flat" structure. This would look
  nicer, but would be slower as long as (sliceIdx, x, y) can't be used
  as direct indices into the tables.



USER INTERACTION

  As for now, the implementation loads only the pages that are needed
  for the current position of a SoROI/SoVolumeRender-node. Due to
  significant overhead when loading data and squeezing them through a
  transfer function, the user experiences major delays in the visual
  response on her interactions. TGS does not have functionality for
  dynamic loading of data, so the StorageHint-enum is extended with
  two elements (LOAD_MAX, LOAD_DYNAMIC). Currently, LOAD_DYNAMIC is
  the only one implemented of the StorageHints, and is set as
  default. By using LOAD_MAX, the specified available memory should be
  filled to it's maximum. Pages should be selected in an intelligent
  way, depending on the location of possible SoROI-nodes (load the
  surrounding area). This should in most cases speed up the visual
  feedback.



PALETTED TEXTURES
  
  Paletted textures rocks. Depending on the size of the pages, it
  could save significant amounts of memory. The current implementation
  uses individual palettes for each page. This can be both a good idea
  and a terrible one.

  Good: If the video card supports palettes with different sizes. If,
  for example, a page contains only one color, a 1-bit palette could
  be used and each pixel will occupy 1 bit of hardware memory.

  Bad: If the video card does NOT support palettes with different
  sizes. This means that each palette i.e. has to have 256 entries,
  and with RGBA colors the palette will occupy 256x4=1024 bytes. With
  page sizes smaller than 64x64, this would make the palette occupy
  just as many bytes as the actual pixel data. If the video card
  actually DOES support variable-size palettes, it could still be a
  bad idea. If all the pages require a 256-entry palette (or more) due
  to heavy color variations, the palettes would require a lot of
  hardware memory.

  These problems may be solved by the use of several techniques. First
  of all, there is an extension called GL_SHARED_PALETTE_EXT, that
  allows several textures to share the same palette. A global palette
  for the entire volume could be generated, resulting in some heavy
  pre-calculation and possibly loss of color accuracy, but saving a
  lot of memory. The best solution would probably be a combination of
  local and global palettes. Local, if the page consist entirely of
  one color. Global and shared whenever heavy color variations occur.

glColorTableEXT

  Study SoVolumeDataPage::setData. The code supports palettes of
  variable sizes, exploiting the obvious advantages explained in the
  previous section.  In between the uploading of palette and texture,
  there is a check of what palette size actually achieved. It seems
  like there's no guarantee that a video card supports the different
  palette sizes/formats. If the following glTexImage2D tries to set a
  internal format that doesn't fit the palette size, the entire
  uploading could fail. At least it does on this card (3DLabs Oxygen
  GVX1). The check for palette size fixes this problem.



MEMORY MANAGEMENT

  The TGS-API contains a function setTexMemSize. It makes the client
  application able to specify the amount of memory the volume data
  node should occupy.  In TEXELS?!? As far as my neurons can figure
  out, this doesn't make sense at all. This implementation supports
  this function, but also provides a setHWMemorySize which specifies
  the maximum number of BYTES the volume data should occupy of
  hardware memory.



RENDERING

  A SoVolumeRender is nothing but a SoROI which renders the entire
  volume. And this is how it should be implemented. But this is not
  how it is implemented now. :) The GLRender-function for both SoROI
  and SoVolumeRender is more or less identical, and they should share
  some common render function capable of rendering an entire
  volume. This should in turn use a slice rendering-function similar
  to SoVolumeDataSlice::render.



VOLUMEREADERS

  Currently, only a reader of memory provided data is implemented
  (SoVRMemReader). SoVolumeData uses the interface specified with
  SoVolumeReader, and extensions with other readers should be straight
  forward. When running setReader or setVolumeData, only a pointer to
  the reader is stored. In other words, things could go bananas if the
  client application start mocking around with the reader's settings
  after a call to setReader. If the reader is changed, setReader must
  be run over again.  This requirement differs from TGS as their
  implementation loads all data once specified a reader (I guess).

  The TGS interface for SoVolumeReader contains a function getSubSlice
  with the following definition:

  void getSubSlice(SbBox2s &subSlice, int sliceNumber, void * data)
                  
  It returns a subslice within a specified slice along the z-axis. This 
  means that the responsibility for building slices along X and Y-axis 
  lies within the reader-client.When generating textures along either 
  x- or y-axis, this requires a significant number of iterations, one 
  for each slice along the z-axis. This will in turn trigger plenty 
  filereads at different disklocations, and your disk's heads will have 
  a disco showdown the Travolta way. I've extended the interface as 
  following:

  void getSubSlice(SbBox2s &subSlice, 
                   int sliceNumber, 
                   void * data,
                   SoVolumeRendering::Axis axis = SoVolumeRendering::Z)

  This moves the responsibility for building slices to the reader. 
  It makes it possible to exploit fileformats with possible clever
  data layout, and if the fileformat/input doesn't provide intelligent
  organization, it still wouldn't be any slower. The only drawback is 
  that some functionality would be duplicated among several readers 
  and making them more complicated.

  The consequences is that readers developed for TGS's implementation 
  would not work with ours, but the opposite should work just fine.



TODO
  
  No picking functionality whatsoever is implemented. Other missing
  functions are tagged with FIXMEs.

  Missing classes: SoObliqueSlice, SoOrthoSlice, all readers, all
  details.
  


REFACTORING

  Rumours has it that parts of this library will be refactored and 
  extracted into a more or less external c-library. This is a 
  very good idea, and it is already partially implemented through
  SoVolumeDataPage and SoVolumeDataSlice. This library should be as 
  decoupled from Coin as possible, but it would be a lot of work to 
  build a completely standalone one. An intermediate layer between 
  the lib and Coin would be required, responsible for translating all
  necessary datastructures (i.e. readers and transferfunctions), 
  functioncalls and opengl/coin-states. 
  
  The interface of the library should be quite simple and would 
  probably require the following:
  * A way to support the library with data and data characteristics. 
    Should be done by providing the lib with pointers to 
    SoVolumeReader-objects. 
  * Renderingfunctionality. Volumerendering and slicerendering, 
    specifying location in space, texturecoordinates in the volume and
    transferfunction.
  * Functionality to specify maximum resource usage by the lib. 
  * Preferred storage- and rendering-technique. 

  etc etc...

  Conclusion: This interface came out quite obvious. :) And it will 
  end up a lot like the existing one, except that most of the code in
  SoVolumeData will be pushed into this lib. As mentioned, the lib
  will rely heavily on different Coin-classes, especially SoState, 
  SoVolumeReader and SoTransferFunction and must be designed to fit
  with these.

  The renderingcode is totally independent of the dataformats of 
  textures. (RGBA, paletted etc), and may be reused with great ease 
  whereever needed in the new lib. This code is located in 
  SoVolumeDataSlice::Render and i.e. SoVolumeRender::GLRender. 
  I actually spent quite some time implementing the slicerendering, 
  getting all the interpolation correct when switching from one page 
  to another within the same arbitrary shaped quad. 
  SoVolumeRender::GLRender is more straightforward, but it should be 
  possible to reuse the same loop for all three axis rendering the code 
  more elegant. And it's all about the looks, isn't it? 

  All class declarations are copied from TGS reference manual, and
  should be consistent with the TGS VolumeViz-interface (see
  "VOLUMEREADERS").



  
  torbjorv 08292002
*/

// *************************************************************************

SO_NODE_SOURCE(SoVolumeData);

// *************************************************************************

class SoVolumeDataP {
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
  }

  ~SoVolumeDataP()
  {
    delete VRMemReader;
    releaseSlices();
  }

  SbVec3s dimensions;
  SbBox3f volumeSize;
  SbVec3s pageSize;
  SoVolumeData::DataType dataType;

  SoVRMemReader * VRMemReader;
  SoVolumeReader * reader;

  long tick;
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
}


SoVolumeData::~SoVolumeData()
{
  delete PRIVATE(this);
}


// Doc from parent class.
void
SoVolumeData::initClass(void)
{
  SO_NODE_INIT_CLASS(SoVolumeData, SoVolumeRendering, "VolumeRendering");

  SoVolumeDataElement::initClass();

  SO_ENABLE(SoGLRenderAction, SoVolumeDataElement);
}

void 
SoVolumeData::setVolumeSize(const SbBox3f & size)
{
  PRIVATE(this)->volumeSize = size;
  if (PRIVATE(this)->VRMemReader)
    PRIVATE(this)->VRMemReader->setVolumeSize(size);
}

SbBox3f &
SoVolumeData::getVolumeSize(void) 
{ 
  return PRIVATE(this)->volumeSize; 
}


SbVec3s &
SoVolumeData::getDimensions(void)
{ 
  return PRIVATE(this)->dimensions; 
}


// FIXME: If size != 2^n these functions should extend to the nearest
// accepted size.  torbjorv 07292002
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
}

void 
SoVolumeData::setPageSize(int size) 
{
  setPageSize(SbVec3s(size, size, size));
}

void 
SoVolumeData::setPageSize(const SbVec3s & insize) 
{
  SbVec3s size = insize;

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
  }

  // The Y-size has changed. Rebuild X- and Z-axis maps. 
  if (size[1] != PRIVATE(this)->pageSize[1]) {
    rebuildX = true;
    rebuildZ = true;
  }

  // The Z-size has changed. Rebuild X- and Y-axis maps. 
  if (size[2] != PRIVATE(this)->pageSize[2]) {
    rebuildX = true;
    rebuildY = true;
  }

  PRIVATE(this)->pageSize = size;

  if (rebuildX) PRIVATE(this)->releaseSlicesX();
  if (rebuildY) PRIVATE(this)->releaseSlicesY();
  if (rebuildZ) PRIVATE(this)->releaseSlicesZ();
}

void
SoVolumeData::GLRender(SoGLRenderAction * action)
{
  SoVolumeDataElement::setVolumeData(action->getState(), this, this);
  PRIVATE(this)->tick++;
}

void 
SoVolumeData::renderOrthoSliceX(SoState * state, 
                                const SbBox2f & quad, 
                                float x,
                                int sliceIdx, 
                                const SbBox2f & textureCoords,
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
}

void 
SoVolumeData::renderOrthoSliceY(SoState * state, 
                                const SbBox2f & quad, 
                                float y,
                                int sliceIdx, 
                                const SbBox2f & textureCoords,
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
}

void 
SoVolumeData::renderOrthoSliceZ(SoState * state, 
                                const SbBox2f & quad, 
                                float z,
                                int sliceIdx, 
                                const SbBox2f & textureCoords,
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
}

SbVec3s & 
SoVolumeData::getPageSize()
{
  return PRIVATE(this)->pageSize;
}

void 
SoVolumeData::setTexMemorySize(int size) 
{
  PRIVATE(this)->maxTexels = size;
}

void 
SoVolumeData::setReader(SoVolumeReader * reader) 
{
  PRIVATE(this)->reader = reader;

  reader->getDataChar(PRIVATE(this)->volumeSize,
                      PRIVATE(this)->dataType, 
                      PRIVATE(this)->dimensions);
}

void 
SoVolumeData::setHWMemorySize(int size) 
{
  PRIVATE(this)->maxBytesHW = size;
}

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
  }

  if (!slicesX[sliceIdx]) {
    SoVolumeDataSlice * newSlice = new SoVolumeDataSlice;
    newSlice->init(this->reader, 
                   sliceIdx, 
                   SoVolumeRendering::X, 
                   SbVec2s(this->pageSize[2], this->pageSize[1]));

    slicesX[sliceIdx] = newSlice;
  }

  return slicesX[sliceIdx];
}


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
  }

  if (!slicesY[sliceIdx]) {
    SoVolumeDataSlice * newSlice = new SoVolumeDataSlice;
    newSlice->init(this->reader, 
                   sliceIdx, 
                   SoVolumeRendering::Y, 
                   SbVec2s(this->pageSize[0], this->pageSize[2]));

    slicesY[sliceIdx] = newSlice;
  }

  return slicesY[sliceIdx];
}



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
  }

  if (!slicesZ[sliceIdx]) {
    SoVolumeDataSlice * newSlice = new SoVolumeDataSlice;
    newSlice->init(this->reader, 
                   sliceIdx, 
                   SoVolumeRendering::Z, 
                   SbVec2s(this->pageSize[0], this->pageSize[1]));

    slicesZ[sliceIdx] = newSlice;
  }

  return slicesZ[sliceIdx];
}


// FIXME: Perhaps there already is a function somewhere in C or Coin
// that can test this easily?  31082002 torbjorv
bool 
SoVolumeDataP::check2n(int n)
{
  for (int i = 0; i < (int) (sizeof(int)*8); i++) {

    if (n & 1) {
      if (n != 1) 
        return false;
      else
        return true;
    }

    n >>= 1;
  }
  return true;
}

void 
SoVolumeDataP::releaseSlices(void)
{
  this->releaseSlicesX();
  this->releaseSlicesY();
  this->releaseSlicesZ();
}

void 
SoVolumeDataP::freeTexels(int desired)
{
  if (desired > maxTexels) return;

  while ((maxTexels - numTexels) < desired)
    this->releaseLRUPage();
}


void 
SoVolumeDataP::releaseLRUPage(void)
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
        }
        else
          if (tmpPage != NULL)
            if (tmpPage->lastuse < LRUPage->lastuse) {
              LRUSlice = slicesX[i];
              LRUPage = tmpPage;
            }
      }
    }
  }

  // Searching for LRU page among X-slices
  if (this->slicesY) {
    for (int i = 0; i < this->dimensions[1]; i++) {
      tmpPage = NULL;
      if (slicesY[i]) {
        tmpPage = slicesY[i]->getLRUPage();
        if (LRUPage == NULL) {
          LRUSlice = slicesY[i];
          LRUPage = tmpPage;
        }
        else
          if (tmpPage != NULL)
            if (tmpPage->lastuse < LRUPage->lastuse) {
              LRUSlice = slicesY[i];
              LRUPage = tmpPage;
            }
      }
    }
  }

  // Searching for LRU page among X-slices
  if (this->slicesZ) {
    for (int i = 0; i < this->dimensions[2]; i++) {
      tmpPage = NULL;
      if (slicesZ[i]) {
        tmpPage = slicesZ[i]->getLRUPage();
        if (LRUPage == NULL) {
          LRUSlice = slicesZ[i];
          LRUPage = tmpPage;
        }
        else
          if (tmpPage != NULL)
            if (tmpPage->lastuse < LRUPage->lastuse) {
              LRUSlice = slicesZ[i];
              LRUPage = tmpPage;
            }
      }
    }
  }

  this->numTexels -= LRUSlice->numTexels;
  this->numPages -= LRUSlice->numPages;
  this->numBytesSW -= LRUSlice->numBytesSW;
  this->numBytesHW -= LRUSlice->numBytesHW;

  LRUSlice->releasePage(LRUPage);

  this->numTexels += LRUSlice->numTexels;
  this->numPages += LRUSlice->numPages;
  this->numBytesSW += LRUSlice->numBytesSW;
  this->numBytesHW += LRUSlice->numBytesHW;
}

void 
SoVolumeDataP::releaseSlicesX(void)
{
  if (this->slicesX) {
    for (int i = 0; i < dimensions[0]; i++) {
      delete this->slicesX[i];
      this->slicesX[i] = NULL;
    }

    delete[] this->slicesX;
  }
}

void 
SoVolumeDataP::releaseSlicesY(void)
{
  if (this->slicesY) {
    for (int i = 0; i < dimensions[1]; i++) {
      delete this->slicesY[i];
      this->slicesY[i] = NULL;
    }

    delete[] this->slicesY;
  }
}

void 
SoVolumeDataP::releaseSlicesZ(void)
{
  if (this->slicesZ) {
    for (int i = 0; i < dimensions[2]; i++) {
      delete this->slicesZ[i];
      this->slicesZ[i] = NULL;
    }

    delete[] this->slicesZ;
  }
}

void 
SoVolumeDataP::freeHWBytes(int desired)
{
  if (desired > maxBytesHW) return;

  while ((maxBytesHW - numBytesHW) < desired)
    this->releaseLRUPage();
}

void 
SoVolumeDataP::managePages(void)
{
  // Keep both measures within maxlimits
  this->freeHWBytes(0);
  this->freeTexels(0);
}


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
