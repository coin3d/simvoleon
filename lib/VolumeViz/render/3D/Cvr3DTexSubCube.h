#ifndef SIMVOLEON_CVR3DTEXSUBCUBE_H
#define SIMVOLEON_CVR3DTEXSUBCUBE_H

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

#include <Inventor/SbVec3s.h>
#include <Inventor/SbVec3f.h>
#include <Inventor/SbPlane.h>
#include <Inventor/lists/SbList.h>
#include <Inventor/SbClip.h>

class SbMatrix;
class SbViewVolume;
class SoGLRenderAction;

class CvrCLUT;
class CvrTextureObject;

// *************************************************************************

class Cvr3DTexSubCube {
public:
  Cvr3DTexSubCube(const SoGLRenderAction * action,
                  const CvrTextureObject * texobj,
                  const SbVec3f & cubeorigo,
                  const SbVec3s & cubesize);
  ~Cvr3DTexSubCube();

  void render(const SoGLRenderAction * action);

  // FIXME: do these need to be private? Investigate. 20040716 mortene.
  SbBool isPaletted(void) const;
  void setPalette(const CvrCLUT * newclut);

  void intersectSlice(const SbVec3f * sliceplanecorners);

  // FIXME: this should be obsoleted, use the one above? 20040916 mortene.
  void intersectSlice(const SbViewVolume & viewvolume, 
                      const float viewdistance, 
                      const SbMatrix &);

  void intersectFaceSet(const SbVec3f * vertexlist,
                        const int * numVertices,
                        const unsigned int length,
                        const SbMatrix & m);

  void intersectTriangleStripSet(const SbVec3f * vertexlist,
                                 const int * numVertices,
                                 const unsigned int length,
                                 const SbMatrix & m);
  
  void intersectIndexedFaceSet(const SbVec3f * vertexlist,
                               const int * indices,
                               const unsigned int numindices,
                               const SbMatrix & m);

  void intersectIndexedTriangleStripSet(const SbVec3f * vertexlist,
                                        const int * indices,
                                        const unsigned int numindices,
                                        const SbMatrix & m);

private:
  void renderSlices(const SoGLRenderAction * action, SbBool wireframe);
  void renderBBox(void) const;

  void activateCLUT(const SoGLRenderAction * action); 
  void deactivateCLUT(const SoGLRenderAction * action); 
 
  void clipPolygonAgainstCube(void);

  const CvrTextureObject * textureobject;
  const CvrCLUT * clut;

  SbVec3s dimensions;
  SbVec3f origo;

  struct subcube_slice {
    SbList <SbVec3f> texcoord; 
    SbList <SbVec3f> vertex;  
  };

  SbList <subcube_slice> volumeslices;
  unsigned int volumesliceslength;

  SbPlane clipplanes[6];
  SbClip clippoly;
};

#endif // !SIMVOLEON_CVR3DTEXSUBPAGE_H
