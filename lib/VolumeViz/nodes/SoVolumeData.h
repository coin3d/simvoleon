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

class SoVolumeReader;
class SoState;
class SoTransferFunction;

class SoVolumeData : public SoVolumeRendering {
  typedef SoVolumeRendering inherited;

  SO_NODE_HEADER(SoVolumeData);

public:
  static void initClass(void);
  SoVolumeData(void);

  enum StorageHint {
    AUTO = 0x00000001,
    TEX2D_MULTI = 0x00000002,
    TEX2D = TEX2D_MULTI,
    TEX3D = 0x00000004,
    MEMORY = 0x00000008,
    VOLUMEPRO = 0x00000010,
    TEX2D_SINGLE = 0x00000020,
    // FIXME: do we really need the extensions? See doc about them in
    // the .cpp ("USER INTERACTION"). 20021107 mortene.
    LOAD_MAX = 0x00000040,          // Builds as many pages as possible at
    DYNAMIC_LOADING = 0x00000080,   // Only loads the pages used
  };

  enum SubMethod { NEAREST, MAX, AVERAGE };
  enum OverMethod { NONE, CONSTANT, LINEAR, CUBIC };

  // FIXME: "RGBA" not part of TGS Inventor. Find out if we really
  // need it. 20021107 mortene.
  //
  // FIXME: these really indicate the number of bytes pr voxel in
  // internal storage. Should at least set up synonym enum values that
  // reflects this fact better. 20021111 mortene.
  enum DataType { UNSIGNED_BYTE, UNSIGNED_SHORT, RGBA };

  SoSFString fileName;
  SoSFEnum storageHint;
  SoSFBool usePalettedTexture;
  SoSFBool useCompressedTexture;

  void setVolumeData(const SbVec3s & dimension, void * data,
                     SoVolumeData::DataType type = SoVolumeData::UNSIGNED_BYTE);

  void setVolumeSize(const SbBox3f & size);
  SbBox3f & getVolumeSize(void);
  SbVec3s & getDimensions(void);

  // FIXME: this (and perhaps much of the rest of the class) is not
  // part of the TGS VolumeViz API. 20021108 mortene.
  void renderOrthoSlice(SoState * state,
                        const SbBox2f & quad,
                        float depth,
                        int sliceIdx,
                        const SbBox2f & textureCoords,
                        SoTransferFunction * transferFunction,
                        int axis);

  void setPageSize(int size);
  void setPageSize(const SbVec3s & size);
  SbVec3s & getPageSize(void);
  void setTexMemorySize(int size);
  void setHWMemorySize(int size);
  void setReader(SoVolumeReader * reader);


  // FIXME: The following functions are still to be implemented.
  // torbjorv 07122002
  SbBool getVolumeData(SbVec3s & dimension, void *& data,
                       SoVolumeData::DataType & type);


  SoVolumeReader * getReader(void);
  SbBool getMinMax(int & min, int & max);
  SbBool getHistogram(int & length, int *& histogram);
  SoVolumeData * subSetting(const SbBox3s & region);
  void updateRegions(const SbBox3s * region, int num);
  SoVolumeData * reSampling(const SbVec3s & dimension,
                            SoVolumeData::SubMethod subMethod,
                            SoVolumeData::OverMethod = NONE);
  void enableSubSampling(SbBool enable);
  void enableAutoSubSampling(SbBool enable);
  void enableAutoUnSampling(SbBool enable);
  void unSample(void);
  void setSubSamplingMethod(SubMethod method);
  void setSubSamplingLevel(const SbVec3s & ROISampling,
                           const SbVec3s & secondarySampling);


protected:
  ~SoVolumeData();
  void GLRender(SoGLRenderAction * action);

private:
  friend class SoVolumeDataP;
  class SoVolumeDataP * pimpl;
};

#endif // !COIN_SOVOLUMEDATA_H
