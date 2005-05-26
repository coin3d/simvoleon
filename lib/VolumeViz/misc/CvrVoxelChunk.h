#ifndef SIMVOLEON_CVRVOXELCHUNK_H
#define SIMVOLEON_CVRVOXELCHUNK_H

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
 *  Systems in Motion, Postboks 1283, Pirsenteret, 7462 Trondheim, NORWAY.
 *  <URL:http://www.sim.no/>.
 *
\**************************************************************************/

#include <Inventor/SbVec3s.h>
#include <Inventor/SbBox3s.h>
#include <VolumeViz/nodes/SoTransferFunction.h>
#include <VolumeViz/misc/CvrCLUT.h>

class CvrTextureObject;
class SoGLRenderAction;
class SoTransferFunctionElement;
class SbBox2s;

// *************************************************************************

class CvrVoxelChunk {
public:
  CvrVoxelChunk(const SbVec3s & dimensions, unsigned int bytesprvoxel,
                const void * buffer = NULL);
  ~CvrVoxelChunk();

  void transfer(const SoGLRenderAction * action, const CvrCLUT * clut, CvrTextureObject * texobj, SbBool & invisible) const;

  const void * getBuffer(void) const;
  const uint8_t * getBuffer8(void) const;
  const uint16_t * getBuffer16(void) const;

  unsigned int bufferSize(void) const;

  const SbVec3s & getDimensions(void) const;
  unsigned int getUnitSize(void) const;

  void dumpToPPM(const char * filename) const;

  // FIXME: move to CvrCLUT?
  static CvrCLUT * getCLUT(const SoTransferFunctionElement * e, CvrCLUT::AlphaUse alphause);

  CvrVoxelChunk * buildSubPage(const unsigned int axisidx, const int pageidx,
                               const SbBox2s & cutslice);

  CvrVoxelChunk * buildSubCube(const SbBox3s & cubecut);

private:
  void transfer2D(const SoGLRenderAction * action, const CvrCLUT * clut, CvrTextureObject * texobj, SbBool & invisible) const;
  void transfer3D(const SoGLRenderAction * action, const CvrCLUT * clut, CvrTextureObject * texobj, SbBool & invisible) const;
  
  CvrVoxelChunk * buildSubPageX(const int pageidx, const SbBox2s & cutslice);
  CvrVoxelChunk * buildSubPageY(const int pageidx, const SbBox2s & cutslice);
  CvrVoxelChunk * buildSubPageZ(const int pageidx, const SbBox2s & cutslice);

  static CvrCLUT * makeCLUT(const SoTransferFunctionElement * e, CvrCLUT::AlphaUse alphause);
  static SbDict * CLUTdict;

  static uint8_t PREDEFGRADIENTS[SoTransferFunction::SEISMIC + 1][256][4];
  static void initPredefGradients(void);

  SbBool destructbuffer;
  const void * voxelbuffer;
  SbVec3s dimensions;
  unsigned int unitsize;
};

// *************************************************************************

#endif // !SIMVOLEON_CVRVOXELCHUNK_H
