#ifndef SO_RAYCASTRENDERING_H
#define SO_RAYCASTRENDERING_H

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

#include <VolumeViz/C/basic.h>

#ifndef SIMVOLEON_INTERNAL
#error this is a private header file
#endif // !SIMVOLEON_INTERNAL

// FIXME: It is abit strange to use to 'So' prefix for this class as
// it is not a 'Scene Object', but this is how its done in Coin and
// VolumeViz with the SoDB and SoVolumeRendering classes. (20100824
// handegar)

class SIMVOLEON_DLL_API SoRaycastRendering {
public:
  static void init();
};

#endif // !SO_RAYCASTRENDERING_H
