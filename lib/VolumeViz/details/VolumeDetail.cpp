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

/*!
  \class SoVolumeDetail SoVolumeDetail.h VolumeViz/details/SoVolumeDetail.h
  \brief The SoVolumeDetail stores ray intersection information through a volume.

  An SoVolumeDetail contains the information about a ray
  intersecting of a volume rendered with the SoVolumeRender node.

  The detail contains a "profile" of voxel values through the volume,
  where the profile is defined by a start point and an end point.
*/

#include <stddef.h>
#include <string.h>

#include <Inventor/SbName.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/SoPickedPoint.h>
#include <Inventor/actions/SoRayPickAction.h>
#include <Inventor/actions/SoSearchAction.h>

#include <VolumeViz/details/SoVolumeDetail.h>
#include <VolumeViz/misc/CvrUtil.h>
#include <VolumeViz/misc/CvrCLUT.h>
#include <VolumeViz/misc/CvrVoxelChunk.h>
#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <VolumeViz/elements/CvrVoxelBlockElement.h>
#include <VolumeViz/nodes/SoVolumeData.h>

// *************************************************************************

class SoVolumeDetailP {
public:
  SoVolumeDetailP(SoVolumeDetail * master) {
    this->master = master;
  }

  ~SoVolumeDetailP() {}

  // Override default assignment operator, as we don't want to
  // overwrite the "master" member.
  SoVolumeDetailP & operator=(const SoVolumeDetailP & c) {
    this->voxelinfolist = c.voxelinfolist;
    return *this;
  }

  void addVoxelIntersection(const SbVec3f & voxelcoord,
                            const SbVec3s & voxelindex,
                            unsigned int voxelvalue,
                            uint8_t rgba[4]);
  
  void setVoxelValue(const SbVec3s & voxelpos, uint8_t value, 
                     const CvrVoxelBlockElement * elem);

  class VoxelInfo {
  public:
    SbVec3f voxelcoord;
    SbVec3s voxelindex;
    unsigned int voxelvalue;
    uint8_t rgba[4];
  };

  SbList<VoxelInfo> voxelinfolist;
  SoVolumeDetail * master;

  // Note: if adding more members, remember to update the copy
  // operator above.
};


#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

// *************************************************************************

SO_DETAIL_SOURCE(SoVolumeDetail);

// *************************************************************************


SoVolumeDetail::SoVolumeDetail(void)
{
  assert(SoVolumeDetail::getClassTypeId() != SoType::badType());
  PRIVATE(this) = new SoVolumeDetailP(this);
}


SoVolumeDetail::~SoVolumeDetail()
{
  delete PRIVATE(this);
}


// doc in super
void
SoVolumeDetail::initClass(void)
{
  SO_DETAIL_INIT_CLASS(SoVolumeDetail, SoDetail);
}


// doc in super
SoDetail *
SoVolumeDetail::copy(void) const
{
  SoVolumeDetail * copy = new SoVolumeDetail();
  *(PRIVATE(copy)) = *(PRIVATE(this));
  return copy;
}


// *************************************************************************

/*!
  Sets start and end points of ray intersecting the volume in the
  \a profile argument. The points returned are in object-space
  coordinates (i.e. in the same geometry unit system as polygon-based
  geometry in a scene).
*/
void
SoVolumeDetail::getProfileObjectPos(SbVec3f profile[2]) const
{
  const unsigned int nrprofilepoints = PRIVATE(this)->voxelinfolist.getLength();
  assert(nrprofilepoints >= 2);
  profile[0] = PRIVATE(this)->voxelinfolist[0].voxelcoord;
  profile[1] = PRIVATE(this)->voxelinfolist[nrprofilepoints - 1].voxelcoord;
}


/*!
  Sets start and end points of ray intersecting the volume in the \a
  profile argument. The points returned are in voxel-space
  coordinates (i.e. within the dimensions of the voxel block).

  Returns total number of voxels intersected and stored in the
  profile. This can be used to iterate over the full profile with the
  \a getProfileValue() method.

  If \a profile is \c NULL, the points will not be set, but the
  correct number of total profile points will still be returned.
*/
int
SoVolumeDetail::getProfileDataPos(SbVec3s profile[2]) const
{
  const unsigned int nrprofilepoints = PRIVATE(this)->voxelinfolist.getLength();
  assert(nrprofilepoints >= 2);

  if (profile != NULL) {
    profile[0] = PRIVATE(this)->voxelinfolist[0].voxelindex;
    profile[1] = PRIVATE(this)->voxelinfolist[nrprofilepoints - 1].voxelindex;
  }

  return nrprofilepoints;
}


/*!
  Returns voxel value at the given index along the ray intersection
  profile.

  If \a pos is not \c NULL, sets \a pos argument to voxel-space
  coordinates for voxel.

  If \a objpos is not \c NULL, sets \a objpos argument to object-space
  coordinates for voxel.

  The \a flag argument is unused and only included for compatibility
  with TGS Inventor's VolumeViz extension. (Where it seems to be
  undocumented.)
*/
unsigned int
SoVolumeDetail::getProfileValue(int index,
                                      SbVec3s * pos, SbVec3f * objpos,
                                      SbBool flag) const
{
  assert(index >= 0 && index < PRIVATE(this)->voxelinfolist.getLength());

  if (pos) { *pos = PRIVATE(this)->voxelinfolist[index].voxelindex; }
  if (objpos) { *objpos = PRIVATE(this)->voxelinfolist[index].voxelcoord; }
  return PRIVATE(this)->voxelinfolist[index].voxelvalue;
}


/*!
  Fills in the information about the first voxel along the pick ray
  intersection which is not completely transparent.

  Returns \c FALSE if all voxels along pick ray was fully transparent,
  otherwise \c TRUE.

  Sets \a value to the value of the voxel.

  If \a pos is not \c NULL, sets \a pos argument to voxel-space
  coordinates for the first voxel with some opaqueness.

  If \a objpos is not \c NULL, sets \a objpos argument to object-space
  coordinates for the first voxel with some opaqueness.

  The \a flag argument is unused and only included for compatibility
  with TGS Inventor's VolumeViz extension. (Where it seems to be
  undocumented.)
*/
SbBool
SoVolumeDetail::getFirstNonTransparentValue(unsigned int * value,
                                                  SbVec3s * pos,
                                                  SbVec3f * objpos,
                                                  SbBool flag) const
{
  int idx = 0;
  for (idx=0; idx < PRIVATE(this)->voxelinfolist.getLength(); idx++) {
    if (PRIVATE(this)->voxelinfolist[idx].rgba[3] != 0x00) break;
  }

  if (idx == PRIVATE(this)->voxelinfolist.getLength()) { return FALSE; }

  if (pos) { *pos = PRIVATE(this)->voxelinfolist[idx].voxelindex; }
  if (objpos) { *objpos = PRIVATE(this)->voxelinfolist[idx].voxelcoord; }
  if (value) { *value = PRIVATE(this)->voxelinfolist[idx].voxelvalue; }
  return TRUE;
}


/*!
  \VOLEONINTERNAL
  
  Used to set raypick details.  
  NOTE: This method takes different arguments than the TGS equivalent.

*/
void 
SoVolumeDetail::setDetails(const SbVec3f raystart, const SbVec3f rayend, 
                           SoState * state, SoNode * caller)
{
  SoRayPickAction * action = (SoRayPickAction *) state->getAction();
  const CvrVoxelBlockElement * vbelem = CvrVoxelBlockElement::getInstance(state);
  const SoTransferFunctionElement * transferfunctionelement =
    SoTransferFunctionElement::getInstance(state);

  // Find objectspace-dimensions of a voxel.
  const SbBox3f & objbbox = vbelem->getUnitDimensionsBox();
  const SbVec3s & voxcubedims = vbelem->getVoxelCubeDimensions();

  SbVec3f mincorner, maxcorner;
  objbbox.getBounds(mincorner, maxcorner);

  const SbVec3f size = maxcorner - mincorner;
  const SbVec3f voxelsize(size[0] / voxcubedims[0],
                          size[1] / voxcubedims[1],
                          size[2] / voxcubedims[2]);

  // Calculate maximum number of voxels that could possibly be touched
  // by the ray.
  const SbVec3f rayvec = (rayend - raystart);
  const float minvoxdim = SbMin(voxelsize[0], SbMin(voxelsize[1], voxelsize[2]));
  const unsigned int maxvoxinray = (unsigned int)(rayvec.length() / minvoxdim + 1);  
  const SbVec3f stepvec = (rayvec / (float)maxvoxinray) / 2.0f;
  const SbBox3s voxelbounds(SbVec3s(0, 0, 0), voxcubedims - SbVec3s(1, 1, 1));

  SbVec3s ijk, lastijk(-1, -1, -1);
  SoPickedPoint * pickedpoint = NULL;
  CvrCLUT * clut = NULL;
  SbBool opaquevoxelhit = FALSE;

  for (SbVec3f objectcoord = raystart;
       (objectcoord - raystart).length() < (rayend - raystart).length();
       objectcoord += stepvec) {

    // FIXME: we're not hitting the voxels in an exact manner with the
    // intersection testing (it seems we're slightly off in the
    // x-direction, at least), as can be seen from the
    // SoGuiExamples/volumerendering/raypick example (either that or
    // it could be the actual 2D texture-slice rendering that is
    // wrong). 20030220 mortene.
    //
    // UPDATE: this might have been fixed now, at least I found and
    // fixed an offset bug in the objectCoordsToIJK() method
    // today. 20030320 mortene.

    // check if coords is not inside action's volume for picking
    if (!action->isBetweenPlanes(objectcoord)) { continue; }

    ijk = vbelem->objectCoordsToIJK(objectcoord);

    // voxel out of bounds?
    if (!voxelbounds.intersect(ijk)) { continue; }
    // ray already touched this voxel?
    if (ijk == lastijk) { continue; }

    if (CvrUtil::debugRayPicks()) {
      SoDebugError::postInfo("SoVolumeDetail::setDetails",
                             "ray touched new voxel ijk=<%d, %d, %d>",
                             ijk[0], ijk[1], ijk[2]);
    }

    clut = CvrVoxelChunk::getCLUT(transferfunctionelement, CvrCLUT::ALPHA_AS_IS);
    clut->ref();
    uint8_t rgba[4];

    // FIXME: As SIMVoleon does not support 16bits voxels 100% yet,
    // we'll have to scale down the value to 8bit if needed before
    // passing on the data. (20100806 handegar)
    const uint32_t voxelvalue = vbelem->getVoxelValue(ijk) >> 8*(vbelem->getBytesPrVoxel() - 1);

    clut->lookupRGBA(voxelvalue, rgba);
     
    if (pickedpoint == NULL) {                
      if (rgba[3] != 0) {
        pickedpoint = action->addIntersection(objectcoord);        
        opaquevoxelhit = TRUE;
        // if NULL, something else is obstructing the view to the
        // volume, the app programmer only want the nearest, and we
        // don't need to continue our intersection tests
        if (pickedpoint == NULL) return;
      }
      // FIXME: should fill in the normal vector of the pickedpoint:
      //  ->setObjectNormal(<voxcube-side-normal>);
      // 20030320 mortene.       
    }


    if (CvrUtil::debugRayPicks()) { // Draw a voxel-line through the volume
      static uint8_t raypickdebugcounter = 0;
      PRIVATE(this)->setVoxelValue(ijk, 255 - (raypickdebugcounter++ & 2), vbelem);
      if (pickedpoint) {
        SoPath * path = pickedpoint->getPath();          
        SoSearchAction sa;
        sa.setType(SoVolumeData::getClassTypeId());
        sa.setInterest(SoSearchAction::LAST);
        sa.apply(path);          
        SoPath * result = sa.getPath();
        assert(result && "Could not find a SoVolumeData node in path.");
        SoVolumeData * vd = (SoVolumeData *) result->getTail();
        vd->touch(); // Update volume data
      }
    }

    PRIVATE(this)->addVoxelIntersection(objectcoord, ijk, voxelvalue, rgba);
    lastijk = ijk;      
  }

 
  if (clut) clut->unref();
  
  if (pickedpoint) {   
    if (opaquevoxelhit) { 
      // Visible voxels were hit
      pickedpoint->setDetail(this, caller); 
    } 
    else {
      // No visible voxels were hit. Truncate voxel list.
      PRIVATE(this)->voxelinfolist.truncate(0); 
      action->reset();
    } 
  } 
}


void
SoVolumeDetailP::addVoxelIntersection(const SbVec3f & voxelcoord,
                                      const SbVec3s & voxelindex,
                                      unsigned int voxelvalue,
                                      uint8_t rgba[4])
{
  VoxelInfo vxinfo;
  vxinfo.voxelcoord = voxelcoord;
  vxinfo.voxelindex = voxelindex;
  vxinfo.voxelvalue = voxelvalue;
  (void)memcpy(vxinfo.rgba, rgba, sizeof(uint8_t) * 4);

  this->voxelinfolist.append(vxinfo);
}


// *************************************************************************


// For debugging purposes
void
SoVolumeDetailP::setVoxelValue(const SbVec3s & voxelpos, uint8_t value, 
                               const CvrVoxelBlockElement * elem)
{
  SbVec3s voxelcubedims = elem->getVoxelCubeDimensions();

  assert(voxelpos[0] < voxelcubedims[0]);
  assert(voxelpos[1] < voxelcubedims[1]);
  assert(voxelpos[2] < voxelcubedims[2]);

  uint8_t * voxptr = (uint8_t *) elem->getVoxels(); // Cast the const away

  int advance = 0;
  const unsigned int dim[3] = { // so we don't overflow a short
    static_cast<unsigned int>(voxelcubedims[0]),
    static_cast<unsigned int>(voxelcubedims[1]),
    static_cast<unsigned int>(voxelcubedims[2])
  };
  advance += voxelpos[2] * dim[0] * dim[1];
  advance += voxelpos[1] * dim[0];
  advance += voxelpos[0];

  advance *= elem->getBytesPrVoxel();
  voxptr += advance;

  switch (elem->getBytesPrVoxel()) {
  case 1: *voxptr = value; break;
  case 2: *((uint16_t *)voxptr) = value; break;
  default: assert(FALSE); break;
  }
}

#undef PRIVATE
#undef PUBLIC
