#ifndef COIN_SOVOLUMEDETAIL_H
#define COIN_SOVOLUMEDETAIL_H

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


#include <Inventor/misc/SoState.h>
#include <Inventor/nodes/SoNode.h>
#include <Inventor/details/SoDetail.h>
#include <Inventor/details/SoSubDetail.h>
#include <Inventor/SbLinear.h>

#include <VolumeViz/C/basic.h>

class SIMVOLEON_DLL_API SoVolumeDetail : public SoDetail {
  typedef SoDetail inherited;

  SO_DETAIL_HEADER(SoVolumeDetail);

public:
  static void initClass(void);
  SoVolumeDetail(void);
  virtual ~SoVolumeDetail();
 
  virtual SoDetail * copy(void) const;

  void getProfileObjectPos(SbVec3f profile[2]) const;
  int getProfileDataPos(SbVec3s profile[2] = 0) const;
  unsigned int getProfileValue(int index,
                               SbVec3s * pos = 0, SbVec3f * objpos = 0,
                               SbBool flag = FALSE) const;

  SbBool getFirstNonTransparentValue(unsigned int * value,
                                     SbVec3s * pos = 0, SbVec3f * objpos = 0,
                                     SbBool flag = FALSE) const;


  // NOTE: The TGS VolumeViz signature of this function differs from
  // the following. We consider their solution to be unoptimal, and
  // have therefore changed the input arguments. This function is
  // anyway unlikely to be of any interest to the application
  // programmer.
  void setDetails(const SbVec3f raystart, const SbVec3f rayend, 
                  SoState * state, SoNode * caller);

private:
  class SoVolumeDetailP * pimpl;
  friend class SoVolumeDetailP;
};

#endif // !COIN_SOVOLUMEDETAIL_H
