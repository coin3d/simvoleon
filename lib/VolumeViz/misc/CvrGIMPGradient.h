#ifndef COIN_CVRGIMPGRADIENT_H
#define COIN_CVRGIMPGRADIENT_H

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

#include <Inventor/system/inttypes.h>


class CvrGIMPGradientSegment {
public:
  float left, middle, right;
  float left_RGBA[4];
  float right_RGBA[4];
  int type;
  int color;
};

class CvrGIMPGradient {
public:
  int nrsegments;
  class CvrGIMPGradientSegment * segments;

  static CvrGIMPGradient * read(const char * buffer);
  void convertToIntArray(uint8_t intgradient[256][4]) const;
};

#endif // !COIN_CVRGIMPGRADIENT_H
