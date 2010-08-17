#ifndef SIMVOLEON_CVR3DTEXSUBCUBE_H
#define SIMVOLEON_CVR3DTEXSUBCUBE_H

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
