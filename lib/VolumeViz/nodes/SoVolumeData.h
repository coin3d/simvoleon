#ifndef SIMVOLEON_SOVOLUMEDATA_H
#define SIMVOLEON_SOVOLUMEDATA_H

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

#include <Inventor/fields/SoSFString.h>
#include <Inventor/fields/SoSFEnum.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFVec3f.h>
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


class SIMVOLEON_DLL_API SoVolumeData : public SoVolumeRendering {
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

  enum DataType { UNSIGNED_BYTE, UNSIGNED_SHORT };

  SoSFString fileName;
  SoSFEnum storageHint;
  SoSFBool usePalettedTexture;
  SoSFBool useSharedPalettedTexture;
  SoSFBool useCompressedTexture;

  void setVolumeData(const SbVec3s & dimension, void * data,
                     SoVolumeData::DataType type = SoVolumeData::UNSIGNED_BYTE,
                     int significantbits = 0);
  SbBool getVolumeData(SbVec3s & dimension, void *& data,
                       SoVolumeData::DataType & type,
                       int * significantbits = NULL) const;

  uint32_t getVoxelValue(const SbVec3s & voxelpos) const;

  void setVolumeSize(const SbBox3f & size);
  SbBox3f getVolumeSize(void) const;

  void setPageSize(int size);
  void setPageSize(const SbVec3s & size);
  const SbVec3s & getPageSize(void) const;

  void setReader(SoVolumeReader & reader);
  SoVolumeReader * getReader(void) const;

  void setTexMemorySize(int megatexels);
  int getTexMemorySize(void) const;

  SbBool getMinMax(int & minval, int & maxval);
  SbBool getHistogram(int & length, int *& histogram);

  SoVolumeData * subSetting(const SbBox3s & region);
  void updateRegions(const SbBox3s * region, int num);
  void loadRegions(const SbBox3s * region, int num, SoState * state, SoTransferFunction * node);

  SoVolumeData * reSampling(const SbVec3s & dimension,
                            SoVolumeData::SubMethod subMethod,
                            SoVolumeData::OverMethod = NONE);

  void enableSubSampling(SbBool enable);
  SbBool isSubSamplingEnabled(void) const;

  void enableAutoSubSampling(SbBool enable);
  SbBool isAutoSubSamplingEnabled(void) const;

  void enableAutoUnSampling(SbBool enable);
  SbBool isAutoUnSamplingEnabled(void) const;

  void unSample(void);

  void setSubSamplingMethod(SubMethod method);
  SubMethod getSubSamplingMethod(void) const;

  void setSubSamplingLevel(const SbVec3s & roi, const SbVec3s & secondary);
  void getSubSamplingLevel(SbVec3s & roi, SbVec3s & secondary) const;


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

  // These were added to make it possible to control the world-space
  // volume size from an iv-file. They basically provide the same
  // functionality as the setVolumeSize()/getVolumeSize() functions.
  SoSFVec3f volumeboxmin;
  SoSFVec3f volumeboxmax;
};

#endif // !SIMVOLEON_SOVOLUMEDATA_H
