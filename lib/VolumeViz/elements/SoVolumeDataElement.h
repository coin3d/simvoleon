#ifndef COIN_SOVOLUMEDATAELEMENT_H
#define COIN_SOVOLUMEDATAELEMENT_H

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
#include <VolumeViz/nodes/SoVolumeData.h>

class SoVolumeData;
class SbVec3f;


class SIMVOLEON_DLL_API SoVolumeDataElement : public SoReplacedElement {
  typedef SoReplacedElement inherited;
  SO_ELEMENT_HEADER(SoVolumeDataElement);

public:
  static void initClass(void);

  virtual void init(SoState * state);
  static void setVolumeData(SoState * const state, SoNode * const node,
                            SoVolumeData * volumeData);

  SoVolumeData * getVolumeData(void) const;
  static const SoVolumeDataElement * getInstance(SoState * const state);

  // The following public functions are convenience methods,
  // collecting common code working on SoVolumeData instances.

  const SbVec3s getVoxelCubeDimensions(void) const;
  SoVolumeData::DataType getVoxelDataType(void) const;

  SbVec3s objectCoordsToIJK(const SbVec3f & objectpos) const;

  void getPageGeometry(const int axis, const int slicenr,
                       SbVec3f & origo,
                       SbVec3f & horizspan,
                       SbVec3f & verticalspan) const;
  
protected:
  virtual ~SoVolumeDataElement();
  SoVolumeData * nodeptr;

private:
  static void clean(void);
};

#endif // !COIN_SOVOLUMEDATAELEMENT_H
