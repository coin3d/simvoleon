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

#include <VolumeViz/misc/CvrGradient.h>
#include <Inventor/SbVec3s.h>

// *************************************************************************

CvrGradient::CvrGradient(const uint8_t * buf, const SbVec3s & size, SbBool useFlippedYAxis)
{
  this->buf = buf;
  this->size = size;
  this->useFlippedYAxis = useFlippedYAxis;
}

unsigned int
CvrGradient::getVoxelIdx(int x, int y, int z)
{
  if (x < 0) x++; if (x >= size[0]) x--;
  if (y < 0) y++; if (y >= size[1]) y--;
  if (z < 0) z++; if (z >= size[2]) z--;

  if (useFlippedYAxis)
    return (z * (size[0]*size[1])) + (((size[1]-1) - y) * size[0]) + x;
  
  return (z * (size[0]*size[1])) + (size[0]*y) + x;
}

uint8_t
CvrGradient::getVoxel(int x, int y, int z)
{
  return buf[this->getVoxelIdx(x, y, z)];
}
