/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
\**************************************************************************/

#include <VolumeViz/misc/CvrGradient.h>
#include <Inventor/SbVec3f.h>
#include <Inventor/SbVec3s.h>

// *************************************************************************

CvrGradient::CvrGradient(const uint8_t * buf, const SbVec3s & size, SbBool useFlippedYAxis)
{
  this->buf = buf;
  this->size = size;
  this->useFlippedYAxis = useFlippedYAxis;
}

SbVec3f
CvrGradient::getGradientRangeCompressed(unsigned int x, unsigned int y, unsigned int z)
{
  SbVec3f g = this->getGradient(x, y, z);
  g *= 255.0f;
  g += SbVec3f(255.0f, 255.0f, 255.0f);
  g /= 2.0f;
  return g;
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
