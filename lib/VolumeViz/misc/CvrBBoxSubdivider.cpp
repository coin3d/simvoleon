/**************************************************************************\
 *
 *  This file is part of the SIM Voleon visualization library.
 *  Copyright (C) by Kongsberg Oil & Gas Technologies.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  ("GPL") version 2 as published by the Free Software Foundation.
 *  See the file LICENSE.GPL at the root directory of this source
 *  distribution for additional information about the GNU GPL.
 *
 *  For using SIM Voleon with software that can not be combined with
 *  the GNU GPL, and for taking advantage of the additional benefits
 *  of our support services, please contact Kongsberg Oil & Gas
 *  Technologies about acquiring a SIM Voleon Professional Edition
 *  License.
 *
 *  See http://www.coin3d.org/ for more information.
 *
 *  Kongsberg Oil & Gas Technologies, Bygdoy Alle 5, 0257 Oslo, NORWAY.
 *  http://www.sim.no/  sales@sim.no  coin-support@coin3d.org
 *
\**************************************************************************/

#include "CvrBBoxSubdivider.h"
#include <Inventor/SbLinear.h>


bool
CvrBBoxSubdivider::boxFullyInside(SbBox3f mainbox, SbBox3f subbox) const
{
  assert(mainbox.intersect(subbox) && 
         "Not inside at all");

  float b[6];  
  subbox.getBounds(b[0], b[1], b[2], b[3], b[4], b[5]);
  
  SbVec3f corners[8];
  corners[0] = SbVec3f(b[0], b[1], b[2]);
  corners[1] = SbVec3f(b[0], b[1], b[5]);
  corners[2] = SbVec3f(b[0], b[4], b[5]);
  corners[3] = SbVec3f(b[0], b[4], b[2]);
  corners[4] = SbVec3f(b[3], b[1], b[2]);
  corners[5] = SbVec3f(b[3], b[1], b[5]);
  corners[6] = SbVec3f(b[3], b[4], b[5]);
  corners[7] = SbVec3f(b[3], b[4], b[2]);
  
  // Check all 8 corners
  for (int i=0;i<8;++i) {
    if (!mainbox.intersect(corners[i])) {
      return false;
    }
  }

  return true;
}


int
CvrBBoxSubdivider::countIntersectingBoxesInDirection(SbBox3f mainbox, SbBox3f subbox, 
                                                     SbMatrix steptranslation) const
{
  SbList<SbBox3f> tmp;
  return collectBoxesInDirection(mainbox, subbox, steptranslation, tmp, true);
}


int
CvrBBoxSubdivider::collectBoxesInDirection(SbBox3f mainbox, SbBox3f subbox, 
                                           SbMatrix steptranslation,
                                           SbList<SbBox3f> & result,
                                           bool clipboxes) const
{
  int numboxes = 0;
  while(true) {
    if (!mainbox.intersect(subbox)) 
      break; // We are outside
    SbBox3f b;
    if (clipboxes && !this->boxFullyInside(mainbox, subbox))
      b = this->getUnionBox(mainbox, subbox);     
    else 
      b = subbox;

    if (b.hasVolume()) {
      result.append(b);
      ++numboxes;
    }

    subbox.transform(steptranslation);
  }

  return numboxes;
}


SbBox3f
CvrBBoxSubdivider::getUnionBox(SbBox3f box1, SbBox3f box2) const
{
  assert(box1.intersect(box2) && "The boxes don't intersect at all.");
  
  float a[6];  
  box1.getBounds(a[0], a[1], a[2], a[3], a[4], a[5]);
  float b[6];  
  box2.getBounds(b[0], b[1], b[2], b[3], b[4], b[5]);
  
  for (int i=0;i<3;++i) {
    b[i] = a[i] > b[i] ? a[i] : b[i];
    b[i+3] = a[i+3] < b[i+3] ? a[i+3] : b[i+3];
  }

  box2.setBounds(b[0], b[1], b[2], b[3], b[4], b[5]);
  return box2;
}


int 
CvrBBoxSubdivider::collectAllBoxesInside(SbBox3f mainbox, SbBox3f subbox, 
                                         SbList<SbBox3f> & result,
                                         bool clipboxes) const
{
  float width, height, depth;
  subbox.getSize(width, height, depth);

  int totalnr = 0;

  SbBox3f b = subbox;
  SbMatrix t = SbMatrix::identity();
  t.setTranslate(SbVec3f(0, height, 0));
  int numy = this->countIntersectingBoxesInDirection(mainbox, subbox, t);

  b = subbox;
  t.setTranslate(SbVec3f(0, 0, depth));
  int numz = this->countIntersectingBoxesInDirection(mainbox, subbox, t);

  SbMatrix start = SbMatrix::identity();
  t.setTranslate(SbVec3f(width, 0, 0));

  for (int k=0;k<numz;++k) {
    for (int j=0;j<numy;++j) {
      SbBox3f b = subbox;
      start.setTranslate(SbVec3f(0, height*j, depth*k));
      b.transform(start);
      totalnr += this->collectBoxesInDirection(mainbox, b, t, result, clipboxes);
    }
  }

  return totalnr;
}
