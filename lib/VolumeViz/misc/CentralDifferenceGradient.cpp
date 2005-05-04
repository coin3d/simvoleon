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

#include <VolumeViz/misc/CvrCentralDifferenceGradient.h>
#include <Inventor/SbVec3f.h>
#include <Inventor/SbVec3s.h>

// *************************************************************************

SbVec3f
CvrCentralDifferenceGradient::getGradient(unsigned int vx, unsigned int vy, unsigned int vz)
{
  //return SbVec3f(0,0,0);
  int x = (int) vx;
  int y = (int) vy;
  int z = (int) vz;
  
  // Calculate gradient using the central difference algorithm
  SbVec3f g;
  g[0] = getVoxel(x-1, y, z) - getVoxel(x+1, y, z);
  g[1] = getVoxel(x, y-1, z) - getVoxel(x, y+1, z);
  g[2] = getVoxel(x, y, z-1) - getVoxel(x, y, z+1);
  if (g.length() > 0) {
    g.normalize();
  }
  return g;
}
