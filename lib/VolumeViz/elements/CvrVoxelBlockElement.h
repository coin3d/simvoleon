#ifndef SIMVOLEON_CVRVOXELBLOCKELEMENT_H
#define SIMVOLEON_CVRVOXELBLOCKELEMENT_H

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

#include <Inventor/elements/SoReplacedElement.h>
#include <Inventor/SbVec3s.h>
#include <Inventor/SbBox3f.h>

// *************************************************************************

class CvrVoxelBlockElement : public SoReplacedElement {
  typedef SoReplacedElement inherited;
  SO_ELEMENT_HEADER(CvrVoxelBlockElement);

public:
  static void set(SoState * state, SoNode * node, unsigned int bytesprvoxel,
                  const SbVec3s & voxelcubedims, const uint8_t * voxels,
                  const SbBox3f & unitdimensionsbox);

  unsigned int getBytesPrVoxel(void) const;
  const SbVec3s & getVoxelCubeDimensions(void) const;
  const uint8_t * getVoxels(void) const;

  const SbBox3f & getUnitDimensionsBox(void) const;

  // The following public functions are convenience methods,
  // collecting common code working on the data from SoVolumeData.

  SbVec3s objectCoordsToIJK(const SbVec3f & objectpos) const;

  void getPageGeometry(const int axis, const int slicenr,
                       SbVec3f & origo,
                       SbVec3f & horizspan,
                       SbVec3f & verticalspan) const;

  uint32_t getVoxelValue(const SbVec3s & voxelpos) const;


  // Standard element class machinery:

  static void initClass(void);
  virtual void init(SoState * state);
  static const CvrVoxelBlockElement * getInstance(SoState * const state);

  virtual SbBool matches(const SoElement * element) const;
  virtual SoElement * copyMatchInfo() const;

protected:
  virtual ~CvrVoxelBlockElement();

private:
  unsigned int bytesprvoxel;
  SbVec3s voxelcubedims;
  const uint8_t * voxels;
  SbBox3f unitdimensionsbox;
};

// *************************************************************************

#endif // !SIMVOLEON_CVRVOXELBLOCKELEMENT_H
