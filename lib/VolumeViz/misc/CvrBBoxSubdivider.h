#ifndef CVR_BBOXSUBDIVIDER_H
#define CVR_BBOXSUBDIVIDER_H

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

#ifndef SIMVOLEON_INTERNAL
#error this is a private header file
#endif // !SIMVOLEON_INTERNAL

#include <Inventor/SbVec3f.h>
#include <Inventor/SbBox3f.h>
#include <Inventor/lists/SbList.h>


class CvrBBoxSubdivider {
public:
  int countIntersectingBoxesInDirection(SbBox3f mainbox, SbBox3f subbox, 
                                        SbMatrix steptranslation) const;
  int collectAllBoxesInside(SbBox3f mainbox, SbBox3f subbox, 
                            SbList<SbBox3f> & result,
                            bool clipboxes = true) const;
private:
  int collectBoxesInDirection(SbBox3f mainbox, SbBox3f subbox, 
                              SbMatrix steptranslation, SbList<SbBox3f> & result,
                              bool clipboxes = true) const;
  bool boxFullyInside(SbBox3f maincube, SbBox3f subcube) const;
  SbBox3f getUnionBox(SbBox3f box1, SbBox3f box2) const;
};


#endif // !CVR_BBOXSUBDIVIDER_H
