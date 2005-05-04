#ifndef SIMVOLEON_CVRGRADIENT_H
#define SIMVOLEON_CVRGRADIENT_H

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

#include <Inventor/SbBasic.h>
#include <Inventor/SbVec3s.h>

class SbVec3f;

// *************************************************************************

class CvrGradient {
public:
  CvrGradient(const uint8_t * buf, const SbVec3s & size, SbBool useFlippedYAxis);

  SbVec3f getGradientRangeCompressed(unsigned int x, unsigned int y, unsigned int z);
  virtual SbVec3f getGradient(unsigned int x, unsigned int y, unsigned int z) = 0;

protected:
  uint8_t getVoxel(int x, int y, int z);
  
private:
  unsigned int getVoxelIdx(int x, int y, int z);
  const uint8_t * buf;
  SbVec3s size;
  SbBool useFlippedYAxis;
};

// *************************************************************************

#endif // !SIMVOLEON_CVRGRADIENT_H
