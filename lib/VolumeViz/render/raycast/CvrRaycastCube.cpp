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

#include "CvrRaycastCube.h"

#include <Inventor/actions/SoGLRenderAction.h>


CvrRaycastCube::CvrRaycastCube(const SoGLRenderAction * action)
{
}


CvrRaycastCube::~CvrRaycastCube()
{
}


void 
CvrRaycastCube::render(const SoGLRenderAction * action, float quality)
{
  /*
    TODO:
    * Fetch data from element.
    * Split into subcubes according to size-limitations
      * Cache subcubes for later reuse
    * Sort Subcubes according to distance from camera
    * Setup CLVol
    * Render all subcubes
    * render CLVol texture result?
      * Maybe this must be done higher up in the chain.

   */

}
