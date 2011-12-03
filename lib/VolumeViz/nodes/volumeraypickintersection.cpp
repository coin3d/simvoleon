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

#include "volumeraypickintersection.h"
#include <VolumeViz/elements/CvrVoxelBlockElement.h>

/*
  Shared code. 
  Currently used by SoVolumeRender::rayPick and SoVolumeSkin::rayPick
*/

SbBool cvr_volumeraypickintersection(SoRayPickAction * action, SbVec3f intersections[2])
{

  SoState * state = action->getState(); 
  const CvrVoxelBlockElement * vbelem = CvrVoxelBlockElement::getInstance(state);
  if (vbelem == NULL) { return FALSE; }
  
  const SbBox3f & objbbox = vbelem->getUnitDimensionsBox();
  const SbLine ray = action->getLine();
  
  SbVec3f mincorner, maxcorner;
  objbbox.getBounds(mincorner, maxcorner);
    
  SbPlane sides[6] = {
    SbPlane(SbVec3f(0, 0, 1), mincorner[2]), // front face
    SbPlane(SbVec3f(0, 0, 1), maxcorner[2]), // back face
    SbPlane(SbVec3f(1, 0, 0), mincorner[0]), // left face
    SbPlane(SbVec3f(1, 0, 0), maxcorner[0]), // right face
    SbPlane(SbVec3f(0, 1, 0), mincorner[1]), // bottom face
    SbPlane(SbVec3f(0, 1, 0), maxcorner[1])  // top face
  };
    
  static int cmpindices[6][2] = {{0, 1}, {1, 2}, {0, 2}};
    
  unsigned int nrintersect = 0;
    
  for (unsigned int i=0; i < (sizeof(sides) / sizeof(sides[0])); i++) {
    SbVec3f intersectpt;
    if (sides[i].intersect(ray, intersectpt)) {
      const int axisidx0 = cmpindices[i / 2][0];
      const int axisidx1 = cmpindices[i / 2][1];
        
      if ((intersectpt[axisidx0] >= mincorner[axisidx0]) &&
          (intersectpt[axisidx0] <= maxcorner[axisidx0]) &&
          (intersectpt[axisidx1] >= mincorner[axisidx1]) &&
          (intersectpt[axisidx1] <= maxcorner[axisidx1])) {
        intersections[nrintersect++] = intersectpt;
        // Break if we happen to hit more than three sides (could
        // perhaps happen in borderline cases).
        if (nrintersect == 2) break;
      }
    }
  }

  // Borderline case, ignore pick attempt.
  if (nrintersect < 2) { return FALSE; }

  // Sort so first index is the one closest to ray start.
  if ((ray.getPosition() - intersections[0]).sqrLength() >
      (ray.getPosition() - intersections[1]).sqrLength()) {
    SbSwap(intersections[0], intersections[1]);
  }
    
  return TRUE;
    
}
