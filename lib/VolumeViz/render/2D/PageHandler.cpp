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

#include <stdlib.h>
#include <assert.h>

#include <Inventor/C/tidbits.h>
#include <Inventor/C/glue/gl.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/SbTime.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/elements/SoGLLazyElement.h>

#include <VolumeViz/render/2D/CvrPageHandler.h>

#include <VolumeViz/elements/SoVolumeDataElement.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/readers/SoVolumeReader.h>
#include <VolumeViz/render/2D/Cvr2DTexPage.h>
#include <VolumeViz/nodes/SoTransferFunction.h>
#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <VolumeViz/misc/CvrUtil.h>
#include <VolumeViz/misc/CvrCLUT.h>
#include <VolumeViz/misc/CvrVoxelChunk.h>


CvrPageHandler::CvrPageHandler(const SbVec3s & voldatadims,
                               SoVolumeReader * reader)
{
  this->voldatadims[0] = voldatadims[0];
  this->voldatadims[1] = voldatadims[1];
  this->voldatadims[2] = voldatadims[2];

  this->subpagesize = SbVec3s(64, 64, 64);

  this->slices[0] = NULL;
  this->slices[1] = NULL;
  this->slices[2] = NULL;

  this->clut = NULL;

  this->reader = reader;

  // FIXME: Remove this option when we bump the major version
  // number. (20040615 handegar)
  this->flipvolumerendering = FALSE; // Render the 'old' way?, ie. the wrong way.
  const char * flipvolumeenvstr = coin_getenv("CVR_FLIP_Y_AXIS");
  if (flipvolumeenvstr) { this->flipvolumerendering = atoi(flipvolumeenvstr) > 0 ? TRUE : FALSE; }
 
}

CvrPageHandler::~CvrPageHandler()
{
  this->releaseAllSlices();
  if (this->clut) { this->clut->unref(); }
}

// Calculates direction from camera to center of object.
void
CvrPageHandler::getViewVector(SoGLRenderAction * action, SbVec3f & direction) const
{
  SoState * state = action->getState();

  const SbMatrix & mm = SoModelMatrixElement::get(state);
  SbMatrix imm = mm.inverse();

  const SbViewVolume & vv = SoViewVolumeElement::get(state);

  if (/* FIXME: ??? 20021111 mortene: */ 0 &&
      vv.getProjectionType() == SbViewVolume::PERSPECTIVE) {
    SbVec3f worldpos(0.0f, 0.0f, 0.0f);
    mm.multVecMatrix(worldpos, worldpos);
    direction = vv.getProjectionPoint() - worldpos;
    imm.multDirMatrix(direction, direction);
  }
  else { // orthographic
    direction = - vv.getProjectionDirection();
    imm.multDirMatrix(direction, direction);
  }
}

unsigned int
CvrPageHandler::getCurrentAxis(const SbVec3f & viewvec) const
{
  // This is a debugging backdoor: if the environment variable
  // CVR_LOCK_AXIS is set, we'll return the axis value (0 for X, 1 for
  // Y and 2 for Z) it sets, no matter what.
  static int lockaxis = -1;
  if (lockaxis == -1) {
    const char * envstr = coin_getenv("CVR_LOCK_AXIS");
    if (envstr) {
      lockaxis = atoi(envstr);
      assert(lockaxis >= 0 && lockaxis <= 2);
    }
    else lockaxis = -2;
  }
  if (lockaxis != -2) return lockaxis;


  SbVec3f abstoviewer;
  abstoviewer[0] = fabs(viewvec[0]);
  abstoviewer[1] = fabs(viewvec[1]);
  abstoviewer[2] = fabs(viewvec[2]);

  // Figures out which axis we are closest to be looking along:
  
  const SbBool renderalongX =
    (abstoviewer[0] >= abstoviewer[1]) &&
    (abstoviewer[0] >= abstoviewer[2]);

  const SbBool renderalongY =
    (abstoviewer[1] >= abstoviewer[0]) &&
    (abstoviewer[1] >= abstoviewer[2]);

  const SbBool renderalongZ =
    (abstoviewer[2] >= abstoviewer[0]) &&
    (abstoviewer[2] >= abstoviewer[1]);

  // ..more than one of the flags can be set, if camera is on a
  // principal axis of the volume's local coord system. This doesn't
  // matter, as we don't really care which render-direction is used
  // in border-cases.

  return (renderalongX ? 0 : (renderalongY ? 1 : 2));
}

unsigned int
CvrPageHandler::getCurrentAxis(SoGLRenderAction * action) const
{
  SbVec3f camvec;
  this->getViewVector(action, camvec);
  return this->getCurrentAxis(camvec);
}

// Compare with this->subpagesize. If they differ, destroy all pages
// along the changed axes. They will be automatically regenerated upon
// next rendering.
void
CvrPageHandler::comparePageSize(const SbVec3s & currsubpagesize)
{
  SbBool rebuildx = FALSE, rebuildy = FALSE, rebuildz = FALSE;

  if (currsubpagesize[0] != this->subpagesize[0]) rebuildy = rebuildz = TRUE;
  if (currsubpagesize[1] != this->subpagesize[1]) rebuildx = rebuildz = TRUE;
  if (currsubpagesize[2] != this->subpagesize[2]) rebuildx = rebuildy = TRUE;

  if (rebuildx || rebuildy || rebuildz) {
    this->subpagesize = currsubpagesize;

    // This is the quick and simple way to do it.
    if (rebuildx) this->releaseSlices(0);
    if (rebuildy) this->releaseSlices(1);
    if (rebuildz) this->releaseSlices(2);
    // A more optimal strategy when it comes to performance /might/ be
    // to just rearrange the textures within existing pages. This is
    // not so simple, though, as the textures are not stored anywhere
    // else but within OpenGL texture objects -- which means we would
    // have to use ~ 2x the memory to avoid simple destruction and
    // regeneration, as we do now.
    //
    // mortene.
  }
}

void
CvrPageHandler::setPalette(const CvrCLUT * c)
{
  // Change palette for all our pages.
  for (unsigned int axis = 0; axis < 3; axis++) {
    if (this->slices[axis] != NULL) {
      for (unsigned int i = 0; i < this->voldatadims[axis]; i++) {
        if (this->slices[axis][i]) { this->slices[axis][i]->setPalette(c); }
      }
    }
  }

  if (this->clut) { this->clut->unref(); }
  this->clut = c;
  this->clut->ref();
}

void
CvrPageHandler::render(SoGLRenderAction * action, unsigned int numslices,
                       Cvr2DTexSubPage::Interpolation interpolation,
                       CvrPageHandler::Composition composition,
                       SoVolumeRender::SoVolumeRenderAbortCB * abortfunc,
                       void * abortcbdata)
{
  SoState * state = action->getState();

  const SoVolumeDataElement * volumedataelement = SoVolumeDataElement::getInstance(state);
  assert(volumedataelement != NULL);
  SoVolumeData * volumedata = volumedataelement->getVolumeData();
  assert(volumedata != NULL);

  this->comparePageSize(volumedata->getPageSize());

  // FIXME: should have an assert-check that the volume dimensions
  // hasn't changed versus our this->voldatadims


  const SoTransferFunctionElement * tfelement = SoTransferFunctionElement::getInstance(state);
  CvrCLUT * c = CvrVoxelChunk::getCLUT(tfelement);
  if (this->clut != c) { this->setPalette(c); }

  // This must be done, as we want to control stuff in the GL state
  // machine. Without it, state changes could trigger outside our
  // control.
  SoGLLazyElement::getInstance(state)->send(state, SoLazyElement::ALL_MASK);

  // FIXME: do this by the proper Coin mechanisms (i.e. simply
  // state->push()? check with pederb). 20040220 mortene.
  glPushAttrib(GL_ALL_ATTRIB_BITS);

  glDisable(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D);
#if 0 // disabled this, so SoDrawStyle influences SoVolumeRender, as expected
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif

  // FIXME: how does the blending cooperate with the other geometry in
  // a Coin scene graph? Do we need to delay rendering? 20021109 mortene.
  //
  // UPDATE: yes, we do, or geometry that is traversed after this will
  // not be rendered through the transparent parts because it fails
  // the Z-buffer test.

  glEnable(GL_BLEND);

  if (composition == CvrPageHandler::ALPHA_BLENDING) {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }
  else {
    const cc_glglue * glglue = cc_glglue_instance(action->getCacheContext());
    if (!cc_glglue_has_blendequation(glglue)) {
      static SbBool first = TRUE;
      if (first) {
        SoDebugError::postWarning("CvrPageHandler::render",
                                  "Your OpenGL driver is not capable of "
                                  "showing data sets in MAX_INTENSITY or "
                                  "SUM_INTENSITY composition.");
        first = FALSE;
      }
      // Fall back on ALPHA_BLENDING.
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else {
      if (composition == CvrPageHandler::MAX_INTENSITY) {
        cc_glglue_glBlendEquation(glglue, GL_MAX);
        // Note: if we ever find a way of doing this composition mode
        // without depending on the *optional* OGL1.2+ API-function
        // glBlendEquation(), the Doxygen documentation of
        // SoVolumeRender::MAX_INTENSITY should be updated.
      }
      else {
        assert(composition == CvrPageHandler::SUM_INTENSITY &&
               "invalid composition");
        glBlendFunc(GL_ONE, GL_ONE);
        cc_glglue_glBlendEquation(glglue, GL_FUNC_ADD);
        // Note: if we ever find a way of doing this composition mode
        // without depending on the *optional* OGL1.2+ API-function
        // glBlendEquation(), the Doxygen documentation of
        // SoVolumeRender::SUM_INTENSITY should be updated.
        
        // FIXME: did find another way of doing this, but it still
        // involves the imaging subset of OGL1.2 (promoted from
        // EXT_blend_color). This should do the same as the blendfunc
        // above:
        //
        //    glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE)
        //    glBlendColor(1.f, 1.f, 1.f, 1.f/number_of_slices)
        //
        // (Also available as extension: GL_CONSTANT_ALPHA_EXT and
        // glBlendColorEXT()).
        //
        // Should check if there are any advantages over this method
        // rather than using glBlendEquation() (like performance, more
        // wide-spread extension support, ...).
        //
        // 20030121 mortene.
      }
    }
  }

  assert(glGetError() == GL_NO_ERROR);

  // FIXME: what's this good for? 20021128 mortene.
  glDisable(GL_CULL_FACE);

  SbTime renderstart = SbTime::getTimeOfDay(); // for debugging

  SbVec3f camvec;
  this->getViewVector(action, camvec);
  const unsigned int AXISIDX = this->getCurrentAxis(camvec);
  const unsigned int DEPTH = this->voldatadims[AXISIDX];

  SbVec3f origo, horizspan, verticalspan;
  
  for (unsigned int i = 0; i < numslices; i++) {
    // Find nearest integer page idx (as number of pages to render
    // need not match the number of actual volume data pages).
    const float fraction = float(i) / float(numslices); // fraction of rendering

    // FIXME: By removing the '-1' in the commented line below, the
    // last slice will be rendered. But this is probably due to a
    // 'float' rounding feature. Can we assure that the result will be
    // the same on all platforms and for all compilers? (20040315
    // handegar) 
    //unsigned int pageidx = (unsigned int) (fraction * float(DEPTH - 1) + 0.5f);
    unsigned int pageidx = (unsigned int) (fraction * float(DEPTH) + 0.5f);

    assert(pageidx < DEPTH);

    // If rendering in reverse order.
    if (camvec[AXISIDX] < 0) { pageidx = DEPTH - pageidx - 1; }
    assert(pageidx < DEPTH);

    SoVolumeRender::AbortCode abortcode =
      (abortfunc == NULL) ?
      SoVolumeRender::CONTINUE :
      abortfunc(numslices, i + 1, abortcbdata);

    if (abortcode == SoVolumeRender::ABORT) break;

    if (abortcode == SoVolumeRender::CONTINUE) {
      volumedataelement->getPageGeometry(AXISIDX, pageidx,
                                         origo, horizspan, verticalspan);
      
      if (!this->flipvolumerendering) {
        // FIXME: The flip should have been done inside
        // 'getPageGeometry', but it is 'const'. Should this change
        // when we bump the version number. (20040615 handegar)        
        if (AXISIDX != 1) {
          verticalspan = -verticalspan;          
          origo[1] = -origo[1];
        }        
      } else {        
        // Pages along Y-axis is in opposite order of those along X- and
        // Z-axis.     
        // FIXME: Remove this option when we change the major
        // version number. (20040615 handegar)
        if (AXISIDX == 1) { pageidx = DEPTH - pageidx - 1; }
      }

      // Note: even if this is the same page as the last one
      // (numSlices in SoVolumeRender can be larger than the actual
      // dimensions), we should still render it at the new depth, as
      // that can give better rendering quality of the volume.

      Cvr2DTexPage * page = this->getSlice(AXISIDX, pageidx);
      page->render(action, origo, horizspan, verticalspan, interpolation);
    }
    else {
      assert((abortcode == SoVolumeRender::SKIP) &&
             "invalid return value from SoVolumeRender::setAbortCallback() method");
    }
  }

#if CVR_DEBUG && 0 // debug
  SbTime renderend = SbTime::getTimeOfDay();
  SbTime rendertime = renderend - renderstart;
  SoDebugError::postInfo("CvrPageHandler::render",
                         "rendered slices along axis %d in %f seconds",
                         AXISIDX, rendertime.getValue());
#endif // debug

  glPopAttrib();
}

Cvr2DTexPage *
CvrPageHandler::getSlice(const unsigned int AXISIDX, unsigned int sliceidx)
{
  assert(AXISIDX <= 2);
  assert(sliceidx < this->voldatadims[AXISIDX]);

#if 0 // debug
  SoDebugError::postInfo("CvrPageHandler::getSlice", "axis==%c sliceidx==%d",
                         AXISIDX == 0 ? 'X' : (AXISIDX == 1 ? 'Y' : 'Z'),
                         sliceidx);
#endif // debug

  // First Cvr2DTexSubPage ever for this axis?
  if (this->slices[AXISIDX] == NULL) {
    this->slices[AXISIDX] = new Cvr2DTexPage*[this->voldatadims[AXISIDX]];
    for (unsigned int i=0; i < this->voldatadims[AXISIDX]; i++) {
      this->slices[AXISIDX][i] = NULL;
    }
  }

  if (this->slices[AXISIDX][sliceidx] == NULL) {
    // Pagesize according to axis: X => [Z, Y], Y => [X, Z], Z => [X, Y].
    SbVec2s pagesize = SbVec2s(this->subpagesize[(AXISIDX == 0) ? 2 : 0],
                               this->subpagesize[(AXISIDX == 1) ? 2 : 1]);
    
    Cvr2DTexPage * newslice =
      new Cvr2DTexPage(this->reader, AXISIDX, sliceidx, pagesize);
    newslice->setPalette(this->clut);
    
    this->slices[AXISIDX][sliceidx] = newslice;
  }
  
  return this->slices[AXISIDX][sliceidx];
}

void
CvrPageHandler::releaseAllSlices(void)
{
  for (unsigned int i = 0; i < 3; i++) { this->releaseSlices(i); }
}

void
CvrPageHandler::releaseSlices(const unsigned int AXISIDX)
{
  assert(AXISIDX <= 2);
  if (this->slices[AXISIDX] == NULL) return;

  // debug
  if (CVR_DEBUG && CvrUtil::doDebugging()) {
    SoDebugError::postInfo("CvrPageHandler::releaseSlices", "%c at %f",
                           AXISIDX == 0 ? 'X' : (AXISIDX == 1 ? 'Y' : 'Z'),
                           SbTime::getTimeOfDay().getValue());
  }

  for (unsigned int i = 0; i < this->voldatadims[AXISIDX]; i++) {
    delete this->slices[AXISIDX][i];
    this->slices[AXISIDX][i] = NULL;
  }

  delete[] this->slices[AXISIDX];
  this->slices[AXISIDX] = NULL;
}
