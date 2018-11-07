/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
\**************************************************************************/

/*!
  \class SoVolumeData VolumeViz/nodes/SoVolumeData.h
  \brief The main interface for setting up volume data sets.

  This node class provides the interface for setting up the voxel data
  to be rendered. For a complete, stand-alone usage example, see the
  SIM Voleon main page documentation.

  Volume data will by default be \e normalized to be within a 2x2x2
  unit dimensions cube. (But note that this is up to the reader
  classes, so it may not be the same for all readers. Check the
  individual class documentation for the file formats you are using.)

  As an example, if you set up a voxel data set of dimensions
  100x400x200, this will be rendered within a bounding box of <-0.25,
  -1, -0.5> to <0.25, 1, 0.5>. Notice that the largest dimension (the
  Y dimension in this example) will be made to fit within unit size 2,
  and the other dimensions will be scaled accordingly.

  You may use SoVolumeData::setVolumeSize() to force a different unit
  size box around the volume, or you can simply use the standard Coin
  transformation nodes, like e.g. SoScale, to accomplish this.


  The volume rendering of SIM Voleon works well on volume data sets of
  any dimensions. With other volume rendering systems, it is often
  necessary to accommodate the rendering system by pre-processing the
  dataset to be of power-of-two dimensions, either to avoid the
  rendering to take up an extraordinary amount of resources related to
  texture-mapping, or from down-right failing. This restriction is not
  present in SIM Voleon, which works well with different dimensions
  along the principal axes, and with any non-power-of-two dimension.

  If the volume data is set from a memory location with the
  SoVolumeData::setVolumeData() method, the voxel data can be changed
  during visualization at will. But after making a batch of changes,
  make sure you notify the node that data has been modified by doing
  the following:

  \code
  volumedatanode->touch();
  \endcode

  Internal regeneration of textures etc for visualization will then be
  done automatically by the SIM Voleon rendering system.
*/

// *************************************************************************

#include <VolumeViz/nodes/SoVolumeData.h>

#include <limits.h>
#include <float.h> // FLT_MAX

#include <Inventor/C/tidbits.h>
#include <Inventor/SbVec3s.h>
#include <Inventor/actions/SoCallbackAction.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/actions/SoPickAction.h>
#include <Inventor/elements/SoGLCacheContextElement.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/sensors/SoFieldSensor.h>
#include <Inventor/lists/SbStringList.h>
#include <Inventor/system/gl.h>

#include <VolumeViz/elements/CvrCompressedTexturesElement.h>
#include <VolumeViz/elements/CvrPalettedTexturesElement.h>
#include <VolumeViz/elements/CvrPageSizeElement.h>
#include <VolumeViz/elements/CvrStorageHintElement.h>
#include <VolumeViz/elements/CvrVoxelBlockElement.h>
#include <VolumeViz/readers/SoVRMemReader.h>
#include <VolumeViz/readers/SoVRVolFileReader.h>
#include <VolumeViz/misc/CvrUtil.h>

// *************************************************************************

/*!
  \var SoSFBool SoVolumeData::usePalettedTexture

  Indicate whether or not to use paletted textures.

  Paletted textures uses only 1/4th of the amount of graphics card
  memory resources versus ordinary RGBA textures, with no degradation
  of rendering quality.

  Not all graphics cards and drivers supports paletted textures, but
  the library will fall back on non-paletted textures automatically if
  that is the case.

  SIM Voleon supports two different OpenGL techniques for doing
  paletted textures: either through using the \c
  GL_EXT_paletted_texture extension available on many older OpenGL
  drivers, or by using fragment shader programs, the latter typically
  available on most modern OpenGL drivers.

  Default value of this field is \c TRUE. Apart from debugging
  purposes, there are not many good reasons to set this field to \c
  FALSE.

  It might however be of interest if one wants to take advantage of
  the typically larger resource savings which can be made from setting
  SoVolumeData::useCompressedTexture to \c TRUE, as that hint will be
  overridden by a \c TRUE value in this field (as long as paletted
  textures are actually supported by the OpenGL driver).

  This because paletted textures can not be "lossy" compressed, so
  only one of paletted textures and texture compression can be active
  at the same time -- not both of them. The policy of SIM Voleon is to
  prefer paletted textures, as that has certain other beneficial
  effects apart from resource savings, mainly that one can modify the
  SoTransferFunction at run-time with no rendering performance hit.
*/

/*!
  \var SoSFBool SoVolumeData::useSharedPalettedTexture

  Indicate whether or not to share texture palettes.

  Sharing of texture palettes is useful for slightly better
  utilization of graphics card memory.

  Default value is \c TRUE. Apart from debugging purposes, there is
  really no good reason to set this field to \c FALSE.

  NOTE: the actions of switching this flag has not been properly
  implemented in Coin yet, its value is simply ignored.

  \since SIM Voleon 2.0
  \since TGS VolumeViz ?.?
*/
// FIXME: implement support for useSharedPalettedTexture, then update
// above doc. Should also try to figure out when this field was added
// to TGS VolumeViz. 20041007 mortene.

/*!
  \var SoSFBool SoVolumeData::useCompressedTexture

  Indicate whether or not to use compressed textures, if supported by
  the graphics card and driver.

  Compressed textures can save a major amount of texture memory out of
  the graphics card memory resources, typically by a factor of about
  5x - 15x. Texture compression is however lossy, meaning that there
  will be a certain amount of degradation of visual quality -- but
  this should usually not be noticable.

  Not all graphics cards and drivers supports compressed textures, but
  the library will fall back on non-compressed textures automatically
  if that is the case.

  Default value is \c TRUE. To secure no loss of visual quality, set
  this field to \c FALSE.

  Note that texture compression will not be done if paletted textures
  are used. See the discussion at the end of the API documentation for
  the SoVolumeData::usePalettedTexture field.
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
    this->subpagesize = SbVec3s(128, 128, 128);

    // Our default size (0 == unlimited).
    this->maxnrtexels = 0;

    this->VRMemReader = new SoVRMemReader;
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
  SbVec3s subpagesize;
  SoVolumeData::DataType datatype;

  SoVRMemReader * VRMemReader;
  SoVolumeReader * reader;

  // FIXME: this is fubar -- we need a global manager, of course, as
  // there can be more than one voxelcube in the scene at once. These
  // should probably be static variables in that manager. 20021118 mortene.
  unsigned int maxnrtexels;

  SoFieldSensor * filenamesensor;
  static void filenameFieldModified(void * userdata, SoSensor * sensor);
  SbBool readNamedFile(void);
  static const char UNDEFINED_FILE[];

  int * histogram;
  unsigned int histogramlength;

  void downSample(SbVec3s dimensions, SoVolumeData::SubMethod subMethod, void * data);
  void overSample(SbVec3s dimensions, SoVolumeData::OverMethod overMethod, void * data);

private:
  SoVolumeData * master;
};

const char SoVolumeDataP::UNDEFINED_FILE[] = "";

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
  // FIXME: MEMORY storage and rendering not properly supported with
  // us. Should detect attempts at using this, and warn the user -- or
  // simply implement it. 20040712 mortene.
  SO_NODE_DEFINE_ENUM_VALUE(StorageHint, MEMORY);
  // FIXME: VOLUMEPRO not properly supported with us. Should detect
  // attempts at using this, and warn the user. 20040712 mortene.
  SO_NODE_DEFINE_ENUM_VALUE(StorageHint, VOLUMEPRO);
  // FIXME: TEX2D_SINGLE is not implemented. It just tells the
  // rendering driver to render the volume one 2D texture at a time,
  // not caching up the data on the card memory (nor in memory? check
  // TGS release notes doc (doesn't seem to be explained in online API
  // doc)). 20040712 mortene.
  SO_NODE_DEFINE_ENUM_VALUE(StorageHint, TEX2D_SINGLE);
  SO_NODE_SET_SF_ENUM_TYPE(storageHint, StorageHint);

  SO_NODE_ADD_FIELD(fileName, (SoVolumeDataP::UNDEFINED_FILE));
  SO_NODE_ADD_FIELD(storageHint, (SoVolumeData::AUTO));
  SO_NODE_ADD_FIELD(usePalettedTexture, (TRUE));
  // FIXME: implement support for this field, it is currently
  // ignored. 20041007 mortene.
  SO_NODE_ADD_FIELD(useSharedPalettedTexture, (TRUE));
  SO_NODE_ADD_FIELD(useCompressedTexture, (TRUE));

  SO_NODE_ADD_FIELD(volumeboxmin, (SbVec3f(FLT_MAX, FLT_MAX, FLT_MAX)));
  SO_NODE_ADD_FIELD(volumeboxmax, (SbVec3f(-FLT_MAX, -FLT_MAX, -FLT_MAX)));

  PRIVATE(this)->filenamesensor = new SoFieldSensor(SoVolumeDataP::filenameFieldModified, this);
  PRIVATE(this)->filenamesensor->setPriority(0); // immediate sensor
  PRIVATE(this)->filenamesensor->attach(&this->fileName);
  PRIVATE(this)->histogram = NULL;
  PRIVATE(this)->histogramlength = 0;
}


SoVolumeData::~SoVolumeData()
{
  delete PRIVATE(this)->filenamesensor;
  delete PRIVATE(this);
}


// Doc from parent class.
void
SoVolumeData::initClass(void)
{
  SO_NODE_INIT_CLASS(SoVolumeData, SoVolumeRendering, "SoVolumeRendering");

  SO_ENABLE(SoGLRenderAction, CvrVoxelBlockElement);
  SO_ENABLE(SoCallbackAction, CvrVoxelBlockElement);
  SO_ENABLE(SoGetBoundingBoxAction, CvrVoxelBlockElement);
  SO_ENABLE(SoPickAction, CvrVoxelBlockElement);

  SO_ENABLE(SoGLRenderAction, CvrCompressedTexturesElement);
  SO_ENABLE(SoGLRenderAction, CvrPalettedTexturesElement);
  SO_ENABLE(SoGLRenderAction, CvrPageSizeElement);
  SO_ENABLE(SoGLRenderAction, CvrStorageHintElement);
}

/*!
  Sets the geometric size of the volume.

  This will override the value found in a volumedata file by a reader
  (if any).
*/
void
SoVolumeData::setVolumeSize(const SbBox3f & size)
{
  SbVec3f volmin, volmax;
  size.getBounds(volmin, volmax);
  this->volumeboxmin = volmin;
  this->volumeboxmax = volmax;

  // Trigger a notification.
  this->touch();
}

/*!
  Returns geometric size of volume.
*/
SbBox3f
SoVolumeData::getVolumeSize(void) const
{
  SbBox3f volbox;

  // If not marked with FLT_MAX, it was explicitly set by the
  // application programmer.
  if (this->volumeboxmin.getValue()[0] != FLT_MAX) {
    volbox = SbBox3f(this->volumeboxmin.getValue(), this->volumeboxmax.getValue());
  }
  else {
    if (PRIVATE(this)->reader) {
      static const char * use_unit_box = coin_getenv("SIMVOLEON_UNIT_DIMENSIONS");
      if (use_unit_box) {
        // FIXME: kept around for compatibility with the EMGS View'EM
        // application. When the above envvar is no longer used there,
        // remove this. 20041006 mortene.
        volbox.setBounds(-0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f);
      }
      else {
        SoVolumeData::DataType type; SbVec3s dim; // dummy parameters
        PRIVATE(this)->reader->getDataChar(volbox, type, dim);
      }
    }
    // else, if no reader, return the empty box.
  }

  return volbox;
}

/*!
  This method lets the user set up a volume data set from data in
  memory, instead of loading it from file (through the
  SoVolumeData::fileName field).

  \a data should point to the block of voxels, of the data size given
  by \a type. \a dimensions specifies the size layout of the voxel
  array along the 3 axes.

  The data block passed in to this function will not be freed when the
  node is destructed, or new data set -- that is considered the
  responsibility of the caller.

  The input data will be mapped to the world coordinate system as
  follows: increasing memory addresses will first be mapped to \e
  increasing voxel positions along the \e X axis, row by row. The rows
  will be mapped to \e increasing positions along the \e Y axis,
  making up slices. Slices will be mapped to \e increasing Z axis
  positions.

  The data will be mapped to be within a cube of size 2x2x2, where the
  largest voxel dimension(s) will be used to normalize the other
  dimensions.

  As an example, if you set up a voxel data set of dimensions
  100x400x200, this will be rendered within a bounding box of <-0.25,
  -1, -0.5> to <0.25, 1, 0.5>. Notice that the largest dimension (the
  Y dimension in this example) will be made to fit within unit size 2,
  and the other dimensions will be scaled accordingly.
*/
void
SoVolumeData::setVolumeData(const SbVec3s & dimensions,
                            void * data,
                            SoVolumeData::DataType type,
                            int significantbits)
{
  // FIXME: implement support for this setting. 20041008 mortene.
  if (significantbits != 0) {
    SoDebugError::postWarning("SoVolumeData::setVolumeData",
                              "support for non-default number of significant "
                              "bits is not yet implemented -- ignoring");
  }

  delete[] PRIVATE(this)->histogram;
  PRIVATE(this)->histogram = NULL;

  if (CvrUtil::doDebugging()) {
    SbString typestr;
    switch (type) {
    case UNSIGNED_BYTE: typestr = "8-bit"; break;
    case UNSIGNED_SHORT: typestr = "16-bit"; break;
    default: assert(FALSE); break;
    }

    SoDebugError::postInfo("SoVolumeData::setVolumeData",
                           "setting %dx%dx%d %s volume data from memory",
                           dimensions[0], dimensions[1], dimensions[2],
                           typestr.getString());
  }

  PRIVATE(this)->VRMemReader->setData(dimensions, data, type);
  this->setReader(*(PRIVATE(this)->VRMemReader));

  PRIVATE(this)->datatype = type;

  // Trigger a notification and a node-ID update, so texture pages etc
  // are regenerated.
  this->touch();
}

/*!
  Returns information about the voxel \a dimensions, a data pointer to
  the memory block of voxels, and a \a type indicator for how many
  bytes are used for each voxel.

  The return value is \c FALSE if the data could not be loaded.
 */
SbBool
SoVolumeData::getVolumeData(SbVec3s & dimensions, void *& data,
                            SoVolumeData::DataType & type,
                            int * significantbits) const
{
  if (PRIVATE(this)->reader == NULL) { return FALSE; }

  dimensions = SbVec3s(PRIVATE(this)->dimensions);
  // FIXME: this is completely bogus use of SoVolumeReader::m_data --
  // this is *not* where the voxel data is supposed to be stored. That
  // is inside SoVolumeData. 20041008 mortene.
  data = PRIVATE(this)->reader->m_data;
  type = PRIVATE(this)->datatype;

  // FIXME: implement proper support for this setting. 20041008 mortene.
  if (significantbits) {
    SoDebugError::postWarning("SoVolumeData::getVolumeData",
                              "support for non-default number of significant "
                              "bits is not yet implemented -- returning "
                              "default value");
    *significantbits = 0;
  }

  return TRUE;
}

/*!
  Returns "raw" value of voxel at given position.
 */
uint32_t
SoVolumeData::getVoxelValue(const SbVec3s & voxelpos) const
{
  // FIXME: this function is also present in CvrVoxelBlockElement.
  // Refactor to common util function. 20040719 mortene.

  assert(voxelpos[0] < PRIVATE(this)->dimensions[0]);
  assert(voxelpos[1] < PRIVATE(this)->dimensions[1]);
  assert(voxelpos[2] < PRIVATE(this)->dimensions[2]);

  uint8_t * voxptr = (uint8_t *)PRIVATE(this)->reader->m_data;
  int advance = 0;
  const unsigned int dim[3] = { // so we don't overflow a short
    static_cast<unsigned int>(PRIVATE(this)->dimensions[0]),
    static_cast<unsigned int>(PRIVATE(this)->dimensions[1]),
    static_cast<unsigned int>(PRIVATE(this)->dimensions[2])
  };
  advance += voxelpos[2] * dim[0] * dim[1];
  advance += voxelpos[1] * dim[0];
  advance += voxelpos[0];

  switch (PRIVATE(this)->datatype) {
  case UNSIGNED_BYTE: break;
  case UNSIGNED_SHORT: advance *= 2; break;
  default: assert(FALSE); break;
  }

  voxptr += advance;

  uint32_t val = 0;
  switch (PRIVATE(this)->datatype) {
  case UNSIGNED_BYTE: val = *voxptr; break;
  case UNSIGNED_SHORT: val = *((uint16_t *)voxptr); break;
  default: assert(FALSE); break;
  }
  return val;
}

/*!

  Sets the largest internal size of texture pages and texture cubes.
  This sets all dimensions to the same value at once.  Default value
  is 128^3.

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

  Sets the largest internal size of texture pages and texture cubes.
  Default value is [128, 128, 128].

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

// *************************************************************************

void
SoVolumeData::doAction(SoAction * action)
{
  unsigned int bytesprvoxel;
  switch (PRIVATE(this)->datatype) {
  case UNSIGNED_BYTE: bytesprvoxel = 1; break;
  case UNSIGNED_SHORT: bytesprvoxel = 2; break;
  default: assert(FALSE); break;
  }

  const uint8_t * voxels = (const uint8_t *)
    (PRIVATE(this)->reader ? PRIVATE(this)->reader->m_data : NULL);

  CvrVoxelBlockElement::set(action->getState(), this, bytesprvoxel,
                            PRIVATE(this)->dimensions, voxels,
                            this->getVolumeSize());
}

void
SoVolumeData::GLRender(SoGLRenderAction * action)
{
  this->doAction(action);

  SoState * s = action->getState();
  CvrCompressedTexturesElement::set(s, this->useCompressedTexture.getValue());
  CvrPalettedTexturesElement::set(s, this->usePalettedTexture.getValue());
  CvrPageSizeElement::set(s, this->getPageSize());
  CvrStorageHintElement::set(s, this->storageHint.getValue());
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
  textures for volume rendering. The value is given in number of
  megatexels, e.g. an argument value "16" will be interpreted to set
  the limit at 16*1024*1024=16777216 texels.

  Note that you can in general not know in advance how much actual
  texture memory a texel is going to use, as textures can be paletted
  with a variable number of bits-pr-texel, and even compressed before
  transfered to the graphics card's on-chip memory.

  Due to the above mentioned reasons, the usefulness of this method is
  rather dubious, but it is still included for compatibility with
  TGS VolumeViz API extension to Open Inventor.

  The default value is to allow unlimited texture memory usage. This
  means that it's up to the underlying OpenGL driver to take care of
  the policy of how to handle scarcity of resources. This is the
  recommended strategy from OpenGL documentation.

  Note that SIM Voleon's default differs from TGS's VolumeViz default,
  which is set at 64 megatexels.
*/
void
SoVolumeData::setTexMemorySize(int megatexels)
{
  assert(megatexels > 0);
  // FIXME: should use a sanity check here for an upper limit?
  // 20021118 mortene.

  PRIVATE(this)->maxnrtexels = megatexels * 1024 * 1024;

#if CVR_DEBUG
  SoDebugError::postWarning("SoVolumeData::setTexMemorySize",
                            "Limitation of texture memory usage not "
                            "implemented yet.");
#endif // CVR_DEBUG


  // FIXME: should kick out texmem pages if we're currently over
  // limit. 20021121 mortene.

  // Trigger a notification and a node-ID update, so texture pages etc
  // are regenerated.
  this->touch();
}

/*!
  Returns limitation forced on texture memory usage.

  \sa SoVolumeData::setTexMemorySize()
  \since SIM Voleon 2.0
*/
int
SoVolumeData::getTexMemorySize(void) const
{
  return PRIVATE(this)->maxnrtexels / (1024 * 1024);
}

// FIXME: document. 20041008 mortene.
void
SoVolumeData::setReader(SoVolumeReader & reader)
{
  PRIVATE(this)->reader = &reader;

  SbBox3f dummyvolbox;
  reader.getDataChar(dummyvolbox,
                     PRIVATE(this)->datatype, PRIVATE(this)->dimensions);

  // Trigger a notification and a node-ID update, so texture pages etc
  // are regenerated.
  this->touch();
}

SoVolumeReader *
SoVolumeData::getReader(void) const
{
  return PRIVATE(this)->reader;
}

/*!
  Returns a reference to a histogram of all voxel values. \a length
  will be set to either 256 for 8-bit data or 65356 for 16-bit data.

  At each index of the histogram table, there will be a value
  indicating the number of voxels that has the data value
  corresponding to the index.

  Return value is always \c TRUE.
*/
SbBool
SoVolumeData::getHistogram(int & length, int *& histogram)
{
  assert(PRIVATE(this)->reader);
  assert(PRIVATE(this)->reader->m_data);

  if (PRIVATE(this)->histogram != NULL) {
    length = PRIVATE(this)->histogramlength;
    histogram = PRIVATE(this)->histogram;
    return TRUE;
  }

  const unsigned int dim[3] = { // so we don't overflow a short
    static_cast<unsigned int>(PRIVATE(this)->dimensions[0]),
    static_cast<unsigned int>(PRIVATE(this)->dimensions[1]),
    static_cast<unsigned int>(PRIVATE(this)->dimensions[2])
  };

  switch (PRIVATE(this)->datatype) {
  case UNSIGNED_BYTE: length = (1 << 8); break;
  case UNSIGNED_SHORT: length = (1 << 16); break;
  default: assert(FALSE); break;
  }

  PRIVATE(this)->histogram = new int[length];
  PRIVATE(this)->histogramlength = length;

  for (unsigned int blankidx = 0; blankidx < (unsigned int)length; blankidx++) {
    PRIVATE(this)->histogram[blankidx] = 0;
  }

  const unsigned long NRVOXELS = dim[0] * dim[1] * dim[2];

  if (PRIVATE(this)->datatype == UNSIGNED_BYTE) {
    uint8_t * voxptr = (uint8_t *)PRIVATE(this)->reader->m_data;
    for (unsigned long voxidx = 0; voxidx < NRVOXELS; voxidx++) {
      PRIVATE(this)->histogram[*voxptr++]++;
    }
  }
  else if (PRIVATE(this)->datatype == UNSIGNED_SHORT) {
    uint16_t * voxptr = (uint16_t *)PRIVATE(this)->reader->m_data;
    for (unsigned long voxidx = 0; voxidx < NRVOXELS; voxidx++) {
      PRIVATE(this)->histogram[*voxptr++]++;
    }
  }
  // unknown types caught by assert() further up

  histogram = PRIVATE(this)->histogram;

  return TRUE;
}

// *************************************************************************

SbBool
SoVolumeData::getMinMax(int & minval, int & maxval)
{
  // FIXME: implement. 20041007 mortene.
  SoDebugError::post("SoVolumeData::getMinMax",
                     "not yet implemented -- just a stub");
  return FALSE;
}

SoVolumeData *
SoVolumeData::subSetting(const SbBox3s &region)
{
  // FIXME: implement. 20041007 mortene.
  SoDebugError::post("SoVolumeData::subSetting",
                     "not yet implemented -- just a stub");
  return NULL;
}

void
SoVolumeData::updateRegions(const SbBox3s *region, int num)
{
  // FIXME: implement. 20041007 mortene.
  SoDebugError::post("SoVolumeData::updateRegions",
                     "not yet implemented -- just a stub");
}

/*!
  Force loading of given subregion. This function should usually not
  be of interest to the application programmer.

  \since SIM Voleon 2.0
*/
void
SoVolumeData::loadRegions(const SbBox3s * region, int num, SoState * state,
                          SoTransferFunction * node)
{
  // FIXME: implement. 20041008 mortene.
#if 0
  SoDebugError::post("SoVolumeData::loadRegions",
                     "not yet implemented -- just a stub");
#else
  // I believe this can safely be a no-op.
#endif
}

// *************************************************************************

SoVolumeData *
SoVolumeData::reSampling(const SbVec3s &dimensions,
                         SoVolumeData::SubMethod subMethod,
                         SoVolumeData::OverMethod overMethod)
{ 
  SoVolumeData * newdataset = new SoVolumeData;
  
  // Creating a temporary dataset which will hold the result
  const int datasize = dimensions[0] * dimensions[1] * dimensions[2];
  void * data;
  switch (PRIVATE(this)->datatype) {
  case UNSIGNED_BYTE: 
    data = new uint8_t[datasize];       
    break;
  case UNSIGNED_SHORT: 
    data = new uint16_t[datasize];
    break;
  default:
    assert(0 && "Unknown datatype");
  }
  
  SbVec3s volumeslices;
  void * discardptr;
  SoVolumeData::DataType type;
  const SbBool ok = this->getVolumeData(volumeslices, discardptr, type);
  assert(ok);

  // FIXME: Over sampling is not implemented yet (Not by TGS
  // either). We therefore crop the dimensions if oversampling is
  // requested as done by VolumeViz. (20040113 handegar)
  SbVec3s newdim = dimensions;
  if (volumeslices[0] < newdim[0]) newdim[0] = volumeslices[0];
  if (volumeslices[1] < newdim[1]) newdim[1] = volumeslices[1];
  if (volumeslices[2] < newdim[2]) newdim[2] = volumeslices[2];

  PRIVATE(this)->downSample(newdim, subMethod, data);
  
  newdataset->setVolumeData(newdim, data, PRIVATE(this)->datatype);
  newdataset->setVolumeSize(this->getVolumeSize());
  // FIXME: this next line looks superfluous? 20040229 mortene.
  newdataset->touch();

  return newdataset; 
}

// *************************************************************************

void
SoVolumeData::enableSubSampling(SbBool enable)
{
  // FIXME: implement. 20041007 mortene.
  SoDebugError::post("SoVolumeData::enableSubSampling",
                     "not yet implemented -- just a stub");
}

// \since SIM Voleon 2.0
SbBool
SoVolumeData::isSubSamplingEnabled(void) const
{
  // FIXME: implement. 20041007 mortene.
  return FALSE;
}

// *************************************************************************

void
SoVolumeData::enableAutoSubSampling(SbBool enable)
{
  // FIXME: implement. 20041007 mortene.
  SoDebugError::post("SoVolumeData::enableAutoSubSampling",
                     "not yet implemented -- just a stub");
}

// \since SIM Voleon 2.0
SbBool
SoVolumeData::isAutoSubSamplingEnabled(void) const
{
  // FIXME: implement. 20041007 mortene.
  return FALSE;
}

// *************************************************************************

void
SoVolumeData::enableAutoUnSampling(SbBool enable)
{
  // FIXME: implement. 20041007 mortene.
  SoDebugError::post("SoVolumeData::enableAutoUnSampling",
                     "not yet implemented -- just a stub");
}

// \since SIM Voleon 2.0
SbBool
SoVolumeData::isAutoUnSamplingEnabled(void) const
{
  // FIXME: implement. 20041007 mortene.
  return FALSE;
}

// *************************************************************************

void
SoVolumeData::unSample(void)
{
  // FIXME: implement. 20041007 mortene.
  SoDebugError::post("SoVolumeData::unSample",
                     "not yet implemented -- just a stub");
}

// *************************************************************************

void
SoVolumeData::setSubSamplingMethod(SubMethod method)
{
  // FIXME: implement. 20041007 mortene.
  SoDebugError::post("SoVolumeData::setSubSamplingMethod",
                     "not yet implemented -- just a stub");
}

// \since SIM Voleon 2.0
SoVolumeData::SubMethod
SoVolumeData::getSubSamplingMethod(void) const
{
  // FIXME: implement. 20041007 mortene.
  return SoVolumeData::NEAREST;
}

// *************************************************************************

void
SoVolumeData::setSubSamplingLevel(const SbVec3s &ROISampling,
                    const SbVec3s &secondarySampling)
{
  // FIXME: implement. 20041007 mortene.
  SoDebugError::post("SoVolumeData::setSubSamplingLevel",
                     "not yet implemented -- just a stub");
}

// \since SIM Voleon 2.0
void
SoVolumeData::getSubSamplingLevel(SbVec3s & roi, SbVec3s & secondary) const
{
  // FIXME: implement. 20041007 mortene.
  SoDebugError::post("SoVolumeData::getSubSamplingLevel",
                     "not yet implemented -- just a stub");
}

// *************************************************************************

void 
SoVolumeDataP::overSample(SbVec3s dimensions, SoVolumeData::OverMethod subMethod, void * data)
{
  // FIXME: implement. 20041007 mortene.
  SoDebugError::post("SoVolumeData::overSample",
                     "not yet implemented -- just a stub");
}

void 
SoVolumeDataP::downSample(SbVec3s dimensions, SoVolumeData::SubMethod subMethod, void * data)
{

  SbVec3s volumeslices;
  void * discardptr;
  SoVolumeData::DataType type;
  const SbBool ok = master->getVolumeData(volumeslices, discardptr, type);
  assert(ok);

  const float scalefactorx = ((float) volumeslices[0]) / (dimensions[0]);
  const float scalefactory = ((float) volumeslices[1]) / (dimensions[1]);
  const float scalefactorz = ((float) volumeslices[2]) / (dimensions[2]);
    
  for (int i=0; i<dimensions[0]; ++i) { // x
    for (int j=0; j<dimensions[1]; ++j) { // y
      for (int k=0; k<dimensions[2]; ++k) { // z

        const int xpos = (int) (scalefactorx * i);
        const int ypos = (int) (scalefactory * j);
        const int zpos = (int) (scalefactorz * k);   
        uint32_t resultvoxel = 0; 
        
        if (subMethod != SoVolumeData::NEAREST) { // Maximum and Average
         
          double averagevoxel[4] = {0, 0, 0, 0};

          for (int x=0; x<((int) scalefactorx); ++x) {
            for (int y=0; y<((int) scalefactory); ++y) {
              for (int z=0; z<((int) scalefactorz); ++z) {

                if (subMethod == SoVolumeData::AVERAGE) { // AVERAGE sampling
                  uint32_t val = master->getVoxelValue(SbVec3s(xpos+x, ypos+y, zpos+z));
                  averagevoxel[0] += (double) val;                                    
                  if (this->datatype != SoVolumeData::UNSIGNED_BYTE) {
                    averagevoxel[1] += (double) ((val>>8) & 0x000000ff);
                  }
                }
                else { // MAX sampling
                  resultvoxel = SbMax(resultvoxel, 
                                      master->getVoxelValue(SbVec3s(xpos+x, ypos+y, zpos+z)));
                }
              }
            }
          }

          if (subMethod == SoVolumeData::AVERAGE) { 

            float nr = scalefactorx * scalefactory * scalefactorz;
            switch (this->datatype) {
            case SoVolumeData::UNSIGNED_SHORT: nr = nr * 2; break;
            default: break;
            }

            resultvoxel = (uint32_t) (averagevoxel[3] / nr);
            resultvoxel = resultvoxel << 8;
            resultvoxel += (uint8_t) (averagevoxel[2] / nr);
            resultvoxel = resultvoxel << 8;
            resultvoxel += (uint8_t) (averagevoxel[1] / nr);
            resultvoxel = resultvoxel << 8;
            resultvoxel += (uint8_t) (averagevoxel[0] / nr);                         
          }
          
        } 
        else { // NEAREST sampling
          resultvoxel = master->getVoxelValue(SbVec3s(xpos, ypos, zpos)); 
        }
       
        const int index = (k * dimensions[0] * dimensions[1]) + (j * dimensions[0]) + i;

        switch (this->datatype) {
        case SoVolumeData::UNSIGNED_BYTE: 
          {
            uint8_t * tmp = (uint8_t *) data;            
            tmp[index] = (uint8_t) resultvoxel; 
            break; 
          }              
        case SoVolumeData::UNSIGNED_SHORT:        
          {
            uint16_t * tmp = (uint16_t *) data;   
            tmp[index] = (uint16_t) resultvoxel;
            break;
          }
        default: assert(FALSE);
        }
        
      }
    }
  }
  

}

// *************************************************************************

// FIXME: should perhaps also override readInstance(), see comments in
// Coin/src/nodes/SoFile.cpp. 20031009 mortene.

SbBool
SoVolumeDataP::readNamedFile(void)
{
  const SbString & filename = PUBLIC(this)->fileName.getValue();
  if (filename == SoVolumeDataP::UNDEFINED_FILE) {
    SoDebugError::postWarning("SoVolumeDataP::readNamedFile",
                              "Undefined filename in SoVolumeData");
    return FALSE;
  }

  SbStringList tmpstrlist = SoInput::getDirectories();
  SbStringList emptysubdirlist;
  const SbString fullfilename =
    SoInput::searchForFile(filename, tmpstrlist, emptysubdirlist);
  if (fullfilename == "") {
    SoDebugError::postWarning("SoVolumeDataP::readNamedFile",
                              "Could not find file '%s' anywhere.",
                              filename.getString());
    return FALSE;
  }

  // FIXME: need to detect file format and choose the correct
  // reader. 20031009 mortene.
  SoVRVolFileReader * filereader = new SoVRVolFileReader;
  filereader->setUserData((void *)fullfilename.getString());

  // FIXME: need all sorts of error checking; format, permission to
  // open, that the file is not corrupt, etc etc. The crappy interface
  // of the SoVolumeReader class (and its subclasses) does not permit
  // that, though (so that's the real problem to fix.) 20031009 mortene.

  if (this->reader) { /* FIXME: delete old! 20031009 mortene. */ }
  PUBLIC(this)->setReader(*filereader);

//   SoReadError::post(in, "Unable to read volume data file: ``%s''",
//                     fullfilename.getString());
  return TRUE;
}

void
SoVolumeDataP::filenameFieldModified(void * userdata, SoSensor * sensor)
{
  SoVolumeData * that = (SoVolumeData *)userdata;
  (void)PRIVATE(that)->readNamedFile();
}

// *************************************************************************
