/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/

#ifndef COIN_SOVOLUMEDATA_H
#define COIN_SOVOLUMEDATA_H

#include <Inventor/fields/SoSFString.h>
#include <Inventor/fields/SoSFEnum.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/SbVec3f.h>
#include <Inventor/SbVec3s.h>
#include <Inventor/SbBox3f.h>
#include <Inventor/SbBox3s.h>
#include <Inventor/SbBox2s.h>
#include <Inventor/SbBox2f.h>
#include <VolumeViz/nodes/SoVolumeRendering.h>
#include <Inventor/nodes/SoSubNode.h>
#include <VolumeViz/misc/SoVolumeDataPage.h>

class SoVolumeReader;





class SoVolumeData : public SoVolumeRendering {
  typedef SoVolumeRendering inherited;

  SO_NODE_HEADER(SoVolumeData);

public:
  static void initClass(void);
  
  enum StorageHint {
    AUTO = 0x00000001,       
    TEX2D_MULTI = 0x00000002, 
    TEX2D = TEX2D_MULTI, 
    TEX3D = 0x00000004, 
    MEMORY = 0x00000008,
    VOLUMEPRO = 0x00000010, 
    TEX2D_SINGLE = 0x00000020,
    LOAD_ALL = 0x00000040,          // Builds as many pages as possible at 
    DYNAMIC_LOADING = 0x00000080,   // Only loads the pages used
  };

  enum DataType {
    UNSIGNED_BYTE, 
    UNSIGNED_SHORT,
    RGB,
    RGBA,
  };

  enum SubMethod {
    NEAREST, 
    MAX, 
    AVERAGE
  };

  enum OverMethod {
    NONE, 
    CONSTANT, 
    LINEAR, 
    CUBIC
  };

  enum Axis {
    X, 
    Y, 
    Z
  };


  // Fields
  SoSFString fileName;
  SoSFEnum storageHint;
  SoSFBool usePalettedTexture;
  SoSFBool useCompressedTexture;



  // Functions
  void setVolumeData( const SbVec3s &dimension, 
                      const void *data, 
                      SoVolumeData::DataType type=UNSIGNED_BYTE);

  void setVolumeSize(const SbBox3f &size);
  SbBox3f &getVolumeSize();
  SbVec3s & getDimensions();
  void GLRender(SoGLRenderAction * action);
  SoVolumeData(void);
  ~SoVolumeData();
  void renderOrthoSliceX( SoState * state,
                          int sliceIdx,
                          SbBox2f &slice, 
                          SbBox2f &mappingcoordinates, 
                          float x);
  void renderOrthoSliceY( SoState * state,
                          int sliceIdx,
                          SbBox2f &slice, 
                          SbBox2f &mappingcoordinates, 
                          float y);
  void renderOrthoSliceZ( SoState * state,
                          int sliceIdx,
                          SbBox2f &slice, 
                          SbBox2f &mappingcoordinates, 
                          float z);
  void setPageSize(int size);
  void setPageSize(SbVec3s &size);
  SbVec3s & getPageSize();
  void setTexMemorySize(int size);




  // FIXME: The following functions are still to be implemented. torbjorv 07122002
  SbBool getVolumeData(SbVec3s &dimension, void *&data, SoVolumeData::DataType &type);
  void setReader(SoVolumeReader &reader);
  SoVolumeReader * getReader();
  SbBool getMinMax(int &min, int &max);
  SbBool getHistogram(int &length, int *&histogram);
  SoVolumeData * subSetting(const SbBox3s &region);
  void updateRegions(const SbBox3s *region, int num);
  SoVolumeData * reSampling(const SbVec3s &dimension, 
                            SoVolumeData::SubMethod subMethod, 
                            SoVolumeData::OverMethod = NONE);
  void enableSubSampling(SbBool enable);
  void enableAutoSubSampling(SbBool enable);
  void enableAutoUnSampling(SbBool enable);
  void unSample();
  void setSubSamplingMethod(SubMethod method);
  void setSubSamplingLevel(const SbVec3s &ROISampling, const SbVec3s &secondarySampling);



private:
  friend class SoVolumeDataP;
  class SoVolumeDataP * pimpl;

};// SoVolumeData

#endif // !COIN_SOVOLUMEDATA_H
