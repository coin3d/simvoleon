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
