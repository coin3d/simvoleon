#ifndef SIMVOLEON_CVRVOXELBLOCKELEMENT_H
#define SIMVOLEON_CVRVOXELBLOCKELEMENT_H

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
