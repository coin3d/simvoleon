#ifndef SIMVOLEON_CVR3DTEXPAGE_H
#define SIMVOLEON_CVR3DTEXPAGE_H

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

#include <Inventor/SbVec3s.h>
#include <VolumeViz/nodes/SoVolumeRender.h>

class SoState;
class CvrCLUT;

// *************************************************************************

class Cvr3DTexCube {
  
public:
  Cvr3DTexCube(const SoGLRenderAction * action);
  ~Cvr3DTexCube();
  
  enum NonindexedSetType { FACE_SET, TRIANGLESTRIP_SET };
  enum IndexedSetType { INDEXEDFACE_SET, INDEXEDTRIANGLESTRIP_SET };

  void render(const SoGLRenderAction * action, const SbVec3f & origo,
              const unsigned int numslices);

  void renderObliqueSlice(const SoGLRenderAction * action,
                          const SbVec3f & origo,
                          const SbPlane plane);
 
  void renderIndexedSet(const SoGLRenderAction * action, const SbVec3f & origo,
                        const SbVec3f * vertexarray,
                        const int * indices,
                        const unsigned int numindices,
                        const enum IndexedSetType type);

  void renderNonindexedSet(const SoGLRenderAction * action,
                           const SbVec3f & origo,
                           const SbVec3f * vertexarray,
                           const int * numVertices,
                           const unsigned int listlength,
                           const enum NonindexedSetType type);
  
  void setPalette(const CvrCLUT * c);
  const CvrCLUT * getPalette(void) const;
  
  typedef SoVolumeRender::AbortCode SoVolumeRenderAbortCB(int totalslices, int thisslice, 
                                                          void * userdata);
  void setAbortCallback(SoVolumeRenderAbortCB * func, void * userdata);

private:
  class Cvr3DTexSubCubeItem * getSubCube(SoState * state, int col, int row, int depth);
  class Cvr3DTexSubCubeItem * buildSubCube(const SoGLRenderAction * action,
                                           int col, int row, int depth);   

  void releaseAllSubCubes(void);
  void releaseSubCube(const int row, const int col, const int depth);
  int calcSubCubeIdx(int row, int col, int depth) const;
  void renderResult(const SoGLRenderAction * action, 
                    SbList <Cvr3DTexSubCubeItem *> subcubelist);

  static SbVec3s clampSubCubeSize(const SbVec3s & size);

  class Cvr3DTexSubCubeItem ** subcubes;

  SbVec3s subcubesize;
  SbVec3s dimensions;

  SbBool rendersubcubeoutline;
  int nrcolumns;
  int nrrows;
  int nrdepths;

  SoVolumeRender::SoVolumeRenderAbortCB * abortfunc;
  void * abortfuncdata;

  const CvrCLUT * clut;
};

#endif // !SIMVOLEON_CVR3DTEXPAGE_H
