/*!
  \class SoVolumeData VolumeViz/nodes/SoVolumeData.h
  \brief The interface for working with volume data sets.
  \ingroup volviz
*/

#include <VolumeViz/nodes/SoVolumeData.h>

#include <Inventor/C/tidbits.h>
#include <Inventor/SbVec3s.h>
#include <Inventor/actions/SoCallbackAction.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/actions/SoPickAction.h>
#include <Inventor/elements/SoGLCacheContextElement.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/system/gl.h>
#include <VolumeViz/elements/SoVolumeDataElement.h>
#include <VolumeViz/readers/SoVRMemReader.h>
#include <VolumeViz/render/2D/Cvr2DTexPage.h>
#include <VolumeViz/render/2D/Cvr2DTexSubPage.h>
#include <limits.h>

/*
DICTIONARY

  "Page"      : One complete cut through the volume, normal to an axis.
  "SubPage"   : A segment of a page.


USER INTERACTION

  As for now, the implementation loads only the subpages that are needed
  for the current position of a SoROI/SoVolumeRender-node. Due to
  significant overhead when loading data and squeezing them through a
  transfer function, the user experiences major delays in the visual
  response on her interactions. TGS does not have functionality for
  dynamic loading of data, so the StorageHint-enum is extended with
  two elements (LOAD_MAX, LOAD_DYNAMIC). Currently, LOAD_DYNAMIC is
  the only one implemented of the StorageHints, and is set as
  default. By using LOAD_MAX, the specified available memory should be
  filled to it's maximum. Subpages should be selected in an intelligent
  way, depending on the location of possible SoROI-nodes (load the
  surrounding area). This should in most cases speed up the visual
  feedback.



PALETTED TEXTURES

  See doc in Cvr2DTexSubPage.


MEMORY MANAGEMENT

  The TGS-API contains a function setTexMemorySize(). It makes the
  client application able to specify the amount of memory the volume
  data node should occupy.  In TEXELS?!? As far as my neurons can
  figure out, this doesn't make sense at all. This implementation
  supports this function, but also provides a setHWMemorySize which
  specifies the maximum number of BYTES the volume data should occupy
  of hardware memory.



RENDERING

  An SoVolumeRender is nothing but an SoROI which renders the entire
  volume. And this is how it should be implemented. But this is not
  how it is implemented now. :) The GLRender-function for both SoROI
  and SoVolumeRender is more or less identical, and they should share
  some common render function capable of rendering an entire
  volume. This should in turn use a page rendering-function similar
  to Cvr2DTexPage::render.



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

  It returns a subpage within a specified page along the z-axis. This
  means that the responsibility for building pages along X and Y-axis
  lies within the reader-client.When generating textures along either
  x- or y-axis, this requires a significant number of iterations, one
  for each page along the z-axis. This will in turn trigger plenty
  filereads at different disklocations, and your disk's heads will have
  a disco showdown the Travolta way. I've extended the interface as
  following:

  void getSubSlice(SbBox2s &subSlice,
                   int sliceNumber,
                   void * data,
                   SoOrthoSlice::Axis axis = SoOrthoSlice::Z)

  This moves the responsibility for building pages to the reader.
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
  Cvr2DTexSubPage and Cvr2DTexPage. This library should be as
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
  * Renderingfunctionality. Volumerendering and pagerendering,
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
  Cvr2DTexPage::Render and i.e. SoVolumeRender::GLRender.
  I actually spent quite some time implementing the pagerendering,
  getting all the interpolation correct when switching from one subpage
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
  enum Axis { X = 0, Y = 1, Z = 2 };

  SoVolumeDataP(SoVolumeData * master)
  {
    this->master = master;

    // FIXME: I think I can kill these since the pagehandler was
    // introduced. 20021122 mortene.
    this->dimensions = SbVec3s(0, 0, 0);
    this->subpagesize = SbVec3s(64, 64, 64);

    // Default size enforced by TGS API.
    this->maxnrtexels = 64*1024*1024;
    // FIXME: should be based on info about the actual run-time
    // system. 20021118 mortene.
    this->maxtexmem = 16*1024*1024;

    this->VRMemReader = NULL;
    this->reader = NULL;
  }

  ~SoVolumeDataP()
  {
    delete this->VRMemReader;
    // FIXME: should really delete "this->reader", but that leads to
    // SEGFAULT now (reader and VRMemReader can be the same pointer.)
    // 20021120 mortene.
  }

  SbVec3s dimensions;
  SbBox3f volumesize;
  SbVec3s subpagesize;
  SoVolumeData::DataType datatype;

  SoVRMemReader * VRMemReader;
  SoVolumeReader * reader;

  // FIXME: this is fubar -- we need a global manager, of course, as
  // there can be more than one voxelcube in the scene at once. These
  // should probably be static variables in that manager. 20021118 mortene.
  unsigned int maxnrtexels;
  unsigned int maxtexmem;

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
  SO_NODE_INIT_CLASS(SoVolumeData, SoVolumeRendering, "SoVolumeRendering");

  SO_ENABLE(SoGLRenderAction, SoVolumeDataElement);
  SO_ENABLE(SoCallbackAction, SoVolumeDataElement);
  SO_ENABLE(SoGetBoundingBoxAction, SoVolumeDataElement);
  SO_ENABLE(SoPickAction, SoVolumeDataElement);
}

/*!
  Sets the geometric size of the volume.

  This will override the value found in a volumedata file by a reader
  (if any).
 */
void
SoVolumeData::setVolumeSize(const SbBox3f & size)
{
  PRIVATE(this)->volumesize = size;
}

/*!
  Returns geometric size of volume.
 */
SbBox3f &
SoVolumeData::getVolumeSize(void) const
{
  // If not empty, it was explicitly set by the application
  // programmer.
  if (!PRIVATE(this)->volumesize.isEmpty())
    return PRIVATE(this)->volumesize;

  // If no reader, return the empty box.
  if (!PRIVATE(this)->reader) 
    return PRIVATE(this)->volumesize;

  SoVolumeData::DataType type; SbVec3s dim; // dummy parameters
  PRIVATE(this)->reader->getDataChar(PRIVATE(this)->volumesize, type, dim);

  return PRIVATE(this)->volumesize;
}

// FIXME: If size != 2^n these functions should extend to the nearest
// accepted size.  torbjorv 07292002
void
SoVolumeData::setVolumeData(const SbVec3s & dimensions,
                            void * data,
                            SoVolumeData::DataType type)
{
#if CVR_DEBUG && 1 // debug
  SoDebugError::postInfo("SoVolumeData::setVolumeData",
                         "setting volume data \"by hand\"");
#endif // debug


  PRIVATE(this)->VRMemReader = new SoVRMemReader;
  PRIVATE(this)->VRMemReader->setData(dimensions, data, type);

  this->setReader(PRIVATE(this)->VRMemReader);

  PRIVATE(this)->datatype = type;
}

SbBool
SoVolumeData::getVolumeData(SbVec3s & dimensions, void *& data,
                            SoVolumeData::DataType & type) const
{
  dimensions = SbVec3s(PRIVATE(this)->dimensions);
  data = PRIVATE(this)->reader->m_data;
  type = PRIVATE(this)->datatype;
  return TRUE;
}

/*!
  Sets the internal size of texture pages and texture cubes.  This
  sets all dimensions to the same value at once.  Default value is
  64^3.

  The \a size value must be a power of two.

  This is essentially of interest only for the internal
  implementation, and should usually not be necessary to change from
  application code.
*/
void
SoVolumeData::setPageSize(int size)
{
  assert(size > 0);
  assert(coin_is_power_of_two(size));
  this->setPageSize(SbVec3s(size, size, size));
}

/*!
  Sets the internal size of texture pages and texture cubes.  Default
  value is [64, 64, 64].

  All elements of \a texsize must be a power of two.

  This is essentially of interest only for the internal
  implementation, and should usually not be necessary to change from
  application code.
*/
void
SoVolumeData::setPageSize(const SbVec3s & texsize)
{
  SbVec3s size = texsize;

  // FIXME: texsize dimensions must also be <=
  // GL_MAX_TEXTURE_SIZE. (Note: according to the OpenGL spec, all
  // implementations must support a width and height of at least
  // 64x64.) 20021122 mortene.
  assert(size[0] > 0 && size[1] > 0 && size[2] > 0);
  assert(coin_is_power_of_two(size[0]));
  assert(coin_is_power_of_two(size[1]));
  assert(coin_is_power_of_two(size[2]));

  PRIVATE(this)->subpagesize = size;

  // This causes a notification and a redraw. The new pagesize should
  // then be picked up automatically by the rendering code.
  this->touch();
}

/*!
  Returns internal dimensions of each 2D texture rectangle or 3D
  texture cube.
 */
const SbVec3s &
SoVolumeData::getPageSize(void) const
{
  return PRIVATE(this)->subpagesize;
}


void
SoVolumeData::doAction(SoAction * action)
{
  SoVolumeDataElement::setVolumeData(action->getState(), this, this);
}

void
SoVolumeData::GLRender(SoGLRenderAction * action)
{
  this->doAction(action);
}

void
SoVolumeData::callback(SoCallbackAction * action)
{
  this->doAction(action);
}

void
SoVolumeData::getBoundingBox(SoGetBoundingBoxAction * action)
{
  this->doAction(action);
}

void
SoVolumeData::pick(SoPickAction * action)
{
  this->doAction(action);
}

/*!
  Set the maximum number of texels we can bind up for 2D and 3D
  textures for volume rendering. The default value is 64 megatexels
  (i.e. 64 * 1024 * 1024).

  Note that you can in general not know in advance how much actual
  texture memory a texel is going to use, as textures can be paletted
  with a variable number of bits-pr-texel, and even compressed before
  transfered to the graphics card's on-chip memory.

  For better control of the actual texture memory used, invoke the
  SoVolumeData::setTextureMemorySize() method instead.

  This is an extension not part of TGS's API for VolumeViz.
 */
void
SoVolumeData::setTexMemorySize(int megatexels)
{
  assert(megatexels > 0);
  // FIXME: should use a sanity check here for an upper limit?
  // 20021118 mortene.

  PRIVATE(this)->maxnrtexels = megatexels * 1024 * 1024;

  // FIXME: should kick out texmem pages if we're currently over
  // limit. 20021121 mortene.
}

void
SoVolumeData::setReader(SoVolumeReader * reader)
{
  PRIVATE(this)->reader = reader;

  reader->getDataChar(PRIVATE(this)->volumesize,
                      PRIVATE(this)->datatype,
                      PRIVATE(this)->dimensions);
}

/*!
  Set the maximum actual texture memory we can bind up for
  2D and 3D textures for volume rendering.

  FIXME: mention what the default value is. (Haven't decided yet.)
  20021120 mortene.

  This is an extension not part of TGS's API for VolumeViz.
 */
void
SoVolumeData::setTextureMemorySize(int texturememory)
{
  assert(texturememory > 0);
  // FIXME: should use a sanity check here for an upper limit?
  // 20021118 mortene.

  PRIVATE(this)->maxtexmem = texturememory;

  // FIXME: should kick out texmem pages if we're currently over
  // limit. 20021121 mortene.
}

/*************************** PIMPL-FUNCTIONS ********************************/

SoVolumeReader *
SoVolumeData::getReader(void) const
{
  return PRIVATE(this)->reader;
}

/****************** UNIMPLEMENTED FUNCTIONS ******************************/
// FIXME: Implement these functions. torbjorv 08282002


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
