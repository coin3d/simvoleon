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
    AUTO,
    TEX2D_MULTI,
    TEX2D = TEX2D_MULTI,
    TEX3D,
    MEMORY,
    VOLUMEPRO,
    TEX2D_SINGLE
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
  SbBool getVolumeData(SbVec3s & dimension, void *& data,
                       SoVolumeData::DataType & type) const;

  void setVolumeSize(const SbBox3f & size);
  SbBox3f & getVolumeSize(void) const;

  void setPageSize(int size);
  void setPageSize(const SbVec3s & size);
  const SbVec3s & getPageSize(void) const;
  void setReader(SoVolumeReader * reader);

  void setTexMemorySize(int megatexels);
  void setTextureMemorySize(int texturememory);

  SoVolumeReader * getReader(void) const;

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

  virtual void doAction(SoAction * action);
  virtual void GLRender(SoGLRenderAction * action);
  virtual void callback(SoCallbackAction * action);
  virtual void getBoundingBox(SoGetBoundingBoxAction * action);
  virtual void pick(SoPickAction * action);

private:
  friend class SoVolumeDataP;
  class SoVolumeDataP * pimpl;
};

#endif // !COIN_SOVOLUMEDATA_H
