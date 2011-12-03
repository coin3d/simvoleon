#ifndef SIMVOLEON_CVR3DTEXPAGE_H
#define SIMVOLEON_CVR3DTEXPAGE_H

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

#ifndef SIMVOLEON_INTERNAL
#error this is a private header file
#endif // !SIMVOLEON_INTERNAL

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

  void render(const SoGLRenderAction * action, unsigned int numslices);

  void renderObliqueSlice(const SoGLRenderAction * action,
                          const SbPlane plane);
 
  void renderIndexedSet(const SoGLRenderAction * action,
                        const SbVec3f * vertexarray,
                        const int * indices,
                        const unsigned int numindices,
                        const enum IndexedSetType type);

  void renderNonindexedSet(const SoGLRenderAction * action,
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
  class Cvr3DTexSubCubeItem * getSubCube(SoState * state, unsigned int col, unsigned int row, unsigned int depth);
  class Cvr3DTexSubCubeItem * buildSubCube(const SoGLRenderAction * action,
                                           const SbVec3f & origo,
                                           unsigned int col,
                                           unsigned int row,
                                           unsigned int depth);   

  void releaseAllSubCubes(void);
  void releaseSubCube(const unsigned int row, const unsigned int col, const unsigned int depth);
  unsigned int calcSubCubeIdx(unsigned int row, unsigned int col, unsigned int depth) const;
  void renderResult(const SoGLRenderAction * action, 
                    SbList <Cvr3DTexSubCubeItem *> & subcubelist);

  static SbVec3s clampSubCubeSize(const SbVec3s & size);

  class Cvr3DTexSubCubeItem ** subcubes;

  SbVec3s subcubesize;
  SbVec3s dimensions;
  SbVec3f origo;

  unsigned int nrcolumns;
  unsigned int nrrows;
  unsigned int nrdepths;

  SoVolumeRender::SoVolumeRenderAbortCB * abortfunc;
  void * abortfuncdata;

  const CvrCLUT * clut;
};

#endif // !SIMVOLEON_CVR3DTEXPAGE_H
