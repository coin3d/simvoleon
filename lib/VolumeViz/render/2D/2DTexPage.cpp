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
 *  Systems in Motion, Teknobyen, Abels Gate 5, 7030 Trondheim, NORWAY.
 *  <URL:http://www.sim.no/>.
 *
\**************************************************************************/

#include <VolumeViz/render/2D/Cvr2DTexPage.h>

#include <limits.h>
#include <string.h>

#include <Inventor/C/tidbits.h>
#include <Inventor/SbBox2s.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/system/gl.h>

#include <VolumeViz/elements/CvrVoxelBlockElement.h>
#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <VolumeViz/misc/CvrCLUT.h>
#include <VolumeViz/misc/CvrUtil.h>
#include <VolumeViz/misc/CvrVoxelChunk.h>
#include <VolumeViz/nodes/SoTransferFunction.h>
#include <VolumeViz/render/common/CvrTextureObject.h>
#include <VolumeViz/render/2D/Cvr2DTexSubPage.h>

// *************************************************************************

class Cvr2DTexSubPageItem {
public:
  Cvr2DTexSubPageItem(Cvr2DTexSubPage * p)
  {
    this->page = p;
  }

  Cvr2DTexSubPage * page;
  uint32_t volumedataid;
  SbBool invisible;
};

// *************************************************************************

Cvr2DTexPage::Cvr2DTexPage(const SoGLRenderAction * action,
                           const unsigned int axis,
                           const unsigned int sliceidx,
                           const SbVec2s & subpagetexsize)
{
  this->subpages = NULL;
  this->clut = NULL;

  assert(subpagetexsize[0] > 0);
  assert(subpagetexsize[1] > 0);
  assert(coin_is_power_of_two(subpagetexsize[0]));
  assert(coin_is_power_of_two(subpagetexsize[1]));

  this->sliceidx = sliceidx;
  this->axis = axis;
  this->subpagesize = subpagetexsize;

  const CvrVoxelBlockElement * vbelem = CvrVoxelBlockElement::getInstance(action->getState());
  const SbVec3s & dim = vbelem->getVoxelCubeDimensions();

  assert(dim[0] > 0);
  assert(dim[1] > 0);
  assert(dim[2] > 0);

  switch (this->axis) {
  case 0: // X-axis
    this->dimensions[0] = dim[2];
    this->dimensions[1] = dim[1];
    break;

  case 1: // Y-axis
    this->dimensions[0] = dim[0];
    this->dimensions[1] = dim[2];
    break;

  case 2: // Z-axis
    this->dimensions[0] = dim[0];
    this->dimensions[1] = dim[1];
    break;

  default:
    assert(FALSE);
    break;
  }

  this->nrcolumns = (this->dimensions[0] + this->subpagesize[0] - 1) / this->subpagesize[0];
  this->nrrows = (this->dimensions[1] + this->subpagesize[1] - 1) / this->subpagesize[1];

#if CVR_DEBUG && 0 // debug
  SoDebugError::postInfo("void Cvr2DTexPage::init",
                         "this->dimensions=[%d, %d], "
                         "this->nrcolumns==%d, this->nrrows==%d, "
                         "this->subpagesize=[%d, %d]",
                         this->dimensions[0], this->dimensions[1],
                         this->nrcolumns, this->nrrows,
                         this->subpagesize[0], this->subpagesize[1]);
#endif // debug

  assert(this->nrcolumns > 0);
  assert(this->nrrows > 0);
}

Cvr2DTexPage::~Cvr2DTexPage()
{
  this->releaseAllSubPages();
  if (this->clut) { this->clut->unref(); }
}

/*!
  Release resources used by a page in the slice.
*/
void
Cvr2DTexPage::releaseSubPage(const int row, const int col)
{
  const int idx = this->calcSubPageIdx(row, col);

  assert(this->subpages);
  Cvr2DTexSubPageItem * p = this->subpages[idx];
  if (p == NULL) { return; }

  this->subpages[idx] = NULL;
  delete p->page;
  delete p;
}

/*!
  Release resources used by a page in the slice.
*/
void
Cvr2DTexPage::releaseSubPage(Cvr2DTexSubPage * page)
{
  assert(page != NULL);
  assert(this->subpages != NULL);

  for (int row = 0; row < this->nrrows; row++) {
    for (int col = 0; col < this->nrcolumns; col++) {
      const int idx = this->calcSubPageIdx(row, col);
      Cvr2DTexSubPageItem * p = this->subpages[idx];
      if (p->page == page) {
        this->releaseSubPage(row, col);
        return;
      }
    }
  }
  assert(FALSE && "couldn't find page");
}

void
Cvr2DTexPage::releaseAllSubPages(void)
{
  if (this->subpages == NULL) return;

  for (int row = 0; row < this->nrrows; row++) {
    for (int col = 0; col < this->nrcolumns; col++) {
      this->releaseSubPage(row, col);
    }
  }

  delete[] this->subpages;
  this->subpages = NULL;
}

// Renders arbitrary positioned quad, textured for the page (slice)
// represented by this object. Automatically loads all pages needed.
void
Cvr2DTexPage::render(const SoGLRenderAction * action,
                     const SbVec3f & origo,
                     const SbVec3f & horizspan, const SbVec3f & verticalspan)
{
  const cc_glglue * glglue = cc_glglue_instance(action->getCacheContext());

  // Find the "local 3D-space" size of each subpage.

  const SbVec2s & sub = this->subpagesize;
  const SbVec2s & dim = this->dimensions;
  const SbVec3f subpagewidth = horizspan * float(sub[0]) / float(dim[0]);
  const SbVec3f subpageheight = verticalspan * float(sub[1]) / float(dim[1]);

  SoState * state = action->getState();

  // Render all subpages making up the full page.

  for (int rowidx = 0; rowidx < this->nrrows; rowidx++) {
    for (int colidx = 0; colidx < this->nrcolumns; colidx++) {

      Cvr2DTexSubPage * page = NULL;
      Cvr2DTexSubPageItem * pageitem = this->getSubPage(state, colidx, rowidx);
      if (pageitem == NULL) { pageitem = this->buildSubPage(action, colidx, rowidx); }
      assert(pageitem != NULL);
      if (pageitem->invisible) continue;
      assert(pageitem->page != NULL);

      SbVec3f upleft = origo +
        // horizontal shift to correct column
        subpagewidth * colidx +
        // vertical shift to correct row
        subpageheight * rowidx;

      // FIXME: should do view frustum culling on each page as an
      // optimization measure (both for rendering speed and texture
      // memory usage). 20021121 mortene.

      pageitem->page->render(action, upleft, subpagewidth, subpageheight);
    }
  }
}


int
Cvr2DTexPage::calcSubPageIdx(int row, int col) const
{
  assert((row >= 0) && (row < this->nrrows));
  assert((col >= 0) && (col < this->nrcolumns));

  return (row * this->nrcolumns) + col;
}

// Builds a page if it doesn't exist. Rebuilds it if it does exist.
Cvr2DTexSubPageItem *
Cvr2DTexPage::buildSubPage(const SoGLRenderAction * action, int col, int row)
{
  // FIXME: optimalization idea; *crop* textures for 100%
  // transparency. 20021124 mortene.

  // FIXME: optimalization idea; detect 100% similar neighboring
  // pages, and make pages able to map to several "slice indices". Not
  // sure if this can be much of a gain -- but look into it. 20021124 mortene.

  assert(this->getSubPage(action->getState(), col, row) == NULL);

  // First Cvr2DTexSubPage ever in this slice?
  if (this->subpages == NULL) {
    this->subpages = new Cvr2DTexSubPageItem*[this->nrrows * this->nrcolumns];
    for (int i=0; i < this->nrrows; i++) {
      for (int j=0; j < this->nrcolumns; j++) {
        const int idx = this->calcSubPageIdx(i, j);
        this->subpages[idx] = NULL;
      }
    }
  }

  SbVec2s subpagemin(col * this->subpagesize[0], row * this->subpagesize[1]);
  SbVec2s subpagemax((col + 1) * this->subpagesize[0],
                     (row + 1) * this->subpagesize[1]);
  subpagemax[0] = SbMin(subpagemax[0], this->dimensions[0]);
  subpagemax[1] = SbMin(subpagemax[1], this->dimensions[1]);

#if CVR_DEBUG && 0 // debug
  SoDebugError::postInfo("Cvr2DTexPage::buildSubPage",
                         "subpagemin=[%d, %d] subpagemax=[%d, %d]",
                         subpagemin[0], subpagemin[1],
                         subpagemax[0], subpagemax[1]);
#endif // debug

  const SbBox2s subpagecut(subpagemin, subpagemax);

  // Size of the texture that we're actually using. Will be less than
  // this->subpagesize on datasets where dimensions are not all power
  // of two, or where dimensions are smaller than this->subpagesize.
  const SbVec2s texsize(subpagemax - subpagemin);

  const CvrTextureObject * texobj =
    CvrTextureObject::create(action, texsize, subpagecut,
                             this->axis, this->sliceidx);
  // if NULL is returned, it means all voxels are fully transparent

  Cvr2DTexSubPage * page = NULL;
  if (texobj) {
    page = new Cvr2DTexSubPage(action, texobj, this->subpagesize, texsize);
    page->setPalette(this->clut);
  }

  SoState * state = action->getState();
  const CvrVoxelBlockElement * vbelem = CvrVoxelBlockElement::getInstance(state);

  Cvr2DTexSubPageItem * pitem = new Cvr2DTexSubPageItem(page);
  pitem->volumedataid = vbelem->getNodeId();
  pitem->invisible = (texobj == NULL);

  const int idx = this->calcSubPageIdx(row, col);
  this->subpages[idx] = pitem;

  return pitem;
}

// *******************************************************************

void
Cvr2DTexPage::setPalette(const CvrCLUT * c)
{
  if (this->clut) { this->clut->unref(); }
  this->clut = c;
  this->clut->ref();

  if (this->subpages == NULL) return;

  // Change palette for all subpages.
  for (int col=0; col < this->nrcolumns; col++) {
    for (int row=0; row < this->nrrows; row++) {
      const int idx = this->calcSubPageIdx(row, col);
      Cvr2DTexSubPageItem * subp = this->subpages[idx];

      // No slice was yet made in this position.
      if (subp == NULL) { continue; }

      // Only if invisible should there be no page allocated.
      assert(subp->invisible || subp->page);

      // If this hits, the page was RGBA and/or previously invisible.
      // That may change when setting a new palette, so remove the old
      // page.
      if (subp->invisible || !subp->page->isPaletted()) {
        this->releaseSubPage(row, col);
        continue;
      }

      // If paletted and previously visible, we simply migrate the new
      // palette to all sub-pages.
      subp->page->setPalette(this->clut);
    }
  }
}

// *******************************************************************

const CvrCLUT *
Cvr2DTexPage::getPalette(void) const
{
  return this->clut;
}

// *******************************************************************

Cvr2DTexSubPageItem *
Cvr2DTexPage::getSubPage(SoState * state, int col, int row)
{
  if (this->subpages == NULL) return NULL;

  assert((col >= 0) && (col < this->nrcolumns));
  assert((row >= 0) && (row < this->nrrows));
  
  const int idx = this->calcSubPageIdx(row, col);
  Cvr2DTexSubPageItem * subp = this->subpages[idx];

  if (subp) {
    const CvrVoxelBlockElement * vbelem = CvrVoxelBlockElement::getInstance(state);
    uint32_t volumedataid = vbelem->getNodeId();

    if (subp->volumedataid != volumedataid) {
      // FIXME: it could perhaps be a decent optimalization to store a
      // checksum value along with the subpage, to use for comparison
      // to see if this subpage really changed. 20030220 mortene.
      this->releaseSubPage(row, col);
      return NULL;
    }
  }

  return subp;
}
