/**************************************************************************\
 *
 *  This file is part of the SIM Voleon visualization library.
 *  Copyright (C) 2003-2004 by Systems in Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  ("GPL") version 2 as published by the Free Software Foundation.
 *  See the file LICENSE.GPL at the root directory of this source
 *  distribution for additional information about the GNU GPL.
 *
 *  For using SIM Voleon with software that can not be combined with
 *  the GNU GPL, and for taking advantage of the additional benefits
 *  of our support services, please contact Systems in Motion about
 *  acquiring a SIM Voleon Professional Edition License.
 *
 *  See <URL:http://www.coin3d.org/> for more information.
 *
 *  Systems in Motion, Teknobyen, Abels Gate 5, 7030 Trondheim, NORWAY.
 *  <URL:http://www.sim.no/>.
 *
\**************************************************************************/

/*!
  \class SoVolumeData VolumeViz/nodes/SoVolumeData.h
  \brief The main interface for setting up volume data sets.

  This node class provides the interface for setting up the voxel data
  to be rendered. For a complete, stand-alone usage example, see the
  SIM Voleon main page documentation.

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

  Regeneration of textures etc for visualization will then be done
  automatically by the SIM Voleon rendering system.
*/

#include <limits.h>
#include <float.h> // FLT_MAX

#include <VolumeViz/nodes/SoVolumeData.h>

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

#include <VolumeViz/elements/SoVolumeDataElement.h>
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

  Default value is \c TRUE. Apart from debugging purposes, there is
  really no good reason to set this field to \c FALSE.
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

    // Our default size (0 == unlimited).
    this->maxnrtexels = 0;

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
  SO_NODE_DEFINE_ENUM_VALUE(StorageHint, MEMORY);
  SO_NODE_DEFINE_ENUM_VALUE(StorageHint, VOLUMEPRO);
  SO_NODE_DEFINE_ENUM_VALUE(StorageHint, TEX2D_SINGLE);
  SO_NODE_SET_SF_ENUM_TYPE(storageHint, StorageHint);

  SO_NODE_ADD_FIELD(fileName, (SoVolumeDataP::UNDEFINED_FILE));
  SO_NODE_ADD_FIELD(storageHint, (SoVolumeData::AUTO));
  SO_NODE_ADD_FIELD(usePalettedTexture, (TRUE));
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
        // This is really the wiser thing to do, as it is much easier
        // to work with something you know is of unit size.
        //
        // FIXME: we should migrate to using this approach everywhere
        // in our Voleon-dependent software (like RIMS and the
        // SoGuiExamples). 20040217 mortene.
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
  will be mapped to \e decreasing positions along the \e Y axis,
  making up slices. Slices will be mapped to \e increasing Z axis
  positions.
*/
void
SoVolumeData::setVolumeData(const SbVec3s & dimensions,
                            void * data,
                            SoVolumeData::DataType type)
{
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


  PRIVATE(this)->VRMemReader = new SoVRMemReader;
  PRIVATE(this)->VRMemReader->setData(dimensions, data, type);

  this->setReader(PRIVATE(this)->VRMemReader);

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
                            SoVolumeData::DataType & type) const
{
  if (PRIVATE(this)->reader == NULL) { return FALSE; }

  dimensions = SbVec3s(PRIVATE(this)->dimensions);
  data = PRIVATE(this)->reader->m_data;
  type = PRIVATE(this)->datatype;
  return TRUE;
}

/*!
  Returns "raw" value of voxel at given position.
 */
uint32_t
SoVolumeData::getVoxelValue(const SbVec3s & voxelpos) const
{
  assert(voxelpos[0] < PRIVATE(this)->dimensions[0]);
  assert(voxelpos[1] < PRIVATE(this)->dimensions[1]);
  assert(voxelpos[2] < PRIVATE(this)->dimensions[2]);

  uint8_t * voxptr = (uint8_t *)PRIVATE(this)->reader->m_data;
  int advance = 0;
  const unsigned int dim[3] = { // so we don't overflow a short
    PRIVATE(this)->dimensions[0], PRIVATE(this)->dimensions[1],  PRIVATE(this)->dimensions[2]
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

void
SoVolumeData::setReader(SoVolumeReader * reader)
{
  PRIVATE(this)->reader = reader;

  SbBox3f dummyvolbox;
  reader->getDataChar(dummyvolbox,
                      PRIVATE(this)->datatype,
                      PRIVATE(this)->dimensions);

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
    PRIVATE(this)->dimensions[0], PRIVATE(this)->dimensions[1],  PRIVATE(this)->dimensions[2]
  };

  switch (PRIVATE(this)->datatype) {
  case UNSIGNED_BYTE: length = (1 << 8); break;
  case UNSIGNED_SHORT: length = (1 << 16); break;
  default: assert(FALSE); break;
  }

  PRIVATE(this)->histogram = new int[length];
  PRIVATE(this)->histogramlength = length;

  for (unsigned int blankidx = 0; blankidx < (unsigned int)length; blankidx++) {
    PRIVATE(this)->histogram[blankidx] = 42;
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

  return TRUE;
}

/****************** UNIMPLEMENTED FUNCTIONS ******************************/
// FIXME: Implement these functions. torbjorv 08282002


SbBool
SoVolumeData::getMinMax(int & minval, int & maxval)
{
  return FALSE;
}

SoVolumeData *
SoVolumeData::subSetting(const SbBox3s &region)
{ return NULL; }

void
SoVolumeData::updateRegions(const SbBox3s *region, int num)
{}

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
  assert(this->getVolumeData(volumeslices, discardptr, type));

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

void 
SoVolumeDataP::overSample(SbVec3s dimensions, SoVolumeData::OverMethod subMethod, void * data)
{
  assert(FALSE && "Oversampling not implemented yet.");
}

void 
SoVolumeDataP::downSample(SbVec3s dimensions, SoVolumeData::SubMethod subMethod, void * data)
{

  SbVec3s volumeslices;
  void * discardptr;
  SoVolumeData::DataType type;
  assert(master->getVolumeData(volumeslices, discardptr, type));

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
  PUBLIC(this)->setReader(filereader);

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
