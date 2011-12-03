#ifndef COIN_SOOBLIQUESLICEDETAIL_H
#define COIN_SOOBLIQUESLICEDETAIL_H

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

#include <Inventor/details/SoDetail.h>
#include <Inventor/details/SoSubDetail.h>
#include <Inventor/SbVec3f.h>
#include <Inventor/SbVec3s.h>
#include <VolumeViz/C/basic.h>


class SIMVOLEON_DLL_API SoObliqueSliceDetail : public SoDetail {
  typedef SoDetail inherited;

  SO_DETAIL_HEADER(SoObliqueSliceDetail);

public:
  static void initClass(void);
  SoObliqueSliceDetail(void);
  virtual ~SoObliqueSliceDetail();
 
  virtual SoDetail * copy(void) const;

  const SbVec3f & getValueObjectPos(void) const;
  const SbVec3s & getValueDataPos(void) const;
  unsigned int getValue(void) const;

private:
  SbVec3f objectcoords;
  SbVec3s ijkcoords;
  unsigned int voxelvalue;

  // FIXME: should rather use a setDetails() function. 20041008 mortene.
  friend class SoObliqueSlice;
};

#endif // !COIN_SOOBLIQUESLICEDETAIL_H
