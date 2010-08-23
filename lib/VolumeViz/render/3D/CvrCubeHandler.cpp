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

#include <VolumeViz/render/3D/CvrCubeHandler.h>

#include <stdlib.h>
#include <assert.h>

#include <Inventor/C/glue/gl.h>
#include <Inventor/C/tidbits.h>
#include <Inventor/SbTime.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/elements/SoGLLazyElement.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/errors/SoDebugError.h>

#include <VolumeViz/elements/CvrVoxelBlockElement.h>
#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <VolumeViz/elements/CvrLightingElement.h>
#include <VolumeViz/misc/CvrCLUT.h>
#include <VolumeViz/misc/CvrUtil.h>
#include <VolumeViz/misc/CvrVoxelChunk.h>
#include <VolumeViz/nodes/SoTransferFunction.h>
#include <VolumeViz/render/3D/Cvr3DTexCube.h>

// *************************************************************************

// FIXME: this is modelled on the CvrPageHandler class used for
// 2D-texturing. This class should not be needed, though, as there is
// always just a single Cvr3DTexCube instance.
//
// (For 2D textures, there are several sets of Cvr2DTexPage instances,
// one set for each principal axis, and each set contains all slices
// for that axis.)
//
// Clean this up. 20040728 mortene.

// *************************************************************************

CvrCubeHandler::CvrCubeHandler(void)
{
  this->volumecube = NULL;
  this->clut = NULL;
  this->voxelblockelementnodeid = 0;
}

CvrCubeHandler::~CvrCubeHandler()
{
  delete this->volumecube;
}

// Calculates direction from camera to center of object.
void
CvrCubeHandler::getViewVector(SoGLRenderAction * action, SbVec3f & direction) const
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


void
CvrCubeHandler::setPalette(const CvrCLUT * c)
{
  assert(this->volumecube && "'volumecube' object is not initialized.");
  assert(c != NULL);
  this->volumecube->setPalette(c);
  this->clut = c;
}

void
CvrCubeHandler::render(SoGLRenderAction * action, CvrCLUT::AlphaUse alphause, unsigned int numslices,
                       CvrCubeHandler::Composition composition,
                       SoVolumeRender::SoVolumeRenderAbortCB * abortfunc,
                       void * abortcbdata)
{
  if (CvrUtil::doDebugging() && FALSE) {
    SoDebugError::postInfo("CvrCubeHandler::render",
                           "numslices==%u", numslices);
  }

  SoState * state = action->getState();
  const CvrVoxelBlockElement * vbelem = CvrVoxelBlockElement::getInstance(state);
  assert(vbelem != NULL);

  // Fetch light settings to detect if light has changed
  const CvrLightingElement * lightelem = CvrLightingElement::getInstance(action->getState());
  assert(lightelem != NULL);  
  const SbBool lighting = lightelem->useLighting(action->getState());
  SbVec3f lightDir;
  float lightIntensity;
  lightelem->get(action->getState(), lightDir, lightIntensity);
  SbBool usePaletteTextures = CvrCLUT::usePaletteTextures(action);

  // Has the dataelement changed since last time?
  // FIXME: Is this test too strict? Not all components in the voxel
  // block element will demand a reconstruction of the 3DTexCube
  // object (20040806 handegar)  
  if ((this->voxelblockelementnodeid != vbelem->getNodeId()) ||
      (this->lighting != lighting ||
       (!usePaletteTextures && (this->lightDirection != lightDir || this->lightIntensity != lightIntensity))) ||
      (this->volumecube == NULL)){
    delete this->volumecube;
    this->clut = NULL;
    this->voxelblockelementnodeid = vbelem->getNodeId();
    this->lighting = lighting;
    this->lightDirection = lightDir;
    this->lightIntensity = lightIntensity;
    this->volumecube = new Cvr3DTexCube(action);
  }

  const SoTransferFunctionElement * tfelement = SoTransferFunctionElement::getInstance(state);
  const CvrCLUT * c = CvrVoxelChunk::getCLUT(tfelement, alphause);
  if (this->clut != c) { this->setPalette(c); }

  // This must be done, as we want to control stuff in the GL state
  // machine. Without it, state changes could trigger outside our
  // control.
  SoGLLazyElement::getInstance(state)->send(state, SoLazyElement::ALL_MASK);

  glPushAttrib(GL_ALL_ATTRIB_BITS);

  glDisable(GL_LIGHTING);
  glEnable(GL_TEXTURE_3D);

  // FIXME: how does the blending cooperate with the other geometry in
  // a Coin scene graph? Do we need to delay rendering? 20021109 mortene.
  //
  // UPDATE: yes, we do, or geometry that is traversed after this will
  // not be rendered through the transparent parts because it fails
  // the Z-buffer test.

  glEnable(GL_BLEND);

  if (composition == CvrCubeHandler::ALPHA_BLENDING) {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }
  else {
    const cc_glglue * glglue = cc_glglue_instance(action->getCacheContext());
    if (!cc_glglue_has_blendequation(glglue)) {
      static SbBool first = TRUE;
      if (first) {
        SoDebugError::postWarning("CvrCubeHandler::render",
                                  "Your OpenGL driver is not capable of "
                                  "showing data sets in MAX_INTENSITY or "
                                  "SUM_INTENSITY composition.");
        first = FALSE;
      }
      // Fall back on ALPHA_BLENDING.
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else {
      if (composition == CvrCubeHandler::MAX_INTENSITY) {
        cc_glglue_glBlendEquation(glglue, GL_MAX);
        // Note: if we ever find a way of doing this composition mode
        // without depending on the *optional* OGL1.2+ API-function
        // glBlendEquation(), the Doxygen documentation of
        // SoVolumeRender::MAX_INTENSITY should be updated.
      }
      else {
        assert(composition == CvrCubeHandler::SUM_INTENSITY &&
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

  if (abortfunc != NULL) { this->volumecube->setAbortCallback(abortfunc, abortcbdata); }
  this->volumecube->render(action, numslices);

  glPopAttrib();
}


void
CvrCubeHandler::renderObliqueSlice(SoGLRenderAction * action,
                                   SoObliqueSlice::AlphaUse alphause,
                                   SbPlane plane)
{
  SoState * state = action->getState();

  const CvrVoxelBlockElement * vbelem = CvrVoxelBlockElement::getInstance(state);
  assert(vbelem != NULL);
 
  if (this->volumecube == NULL) { this->volumecube = new Cvr3DTexCube(action); }

  const SoTransferFunctionElement * tfelement = SoTransferFunctionElement::getInstance(state);
  CvrCLUT::AlphaUse clutalphause;
  switch(alphause) {
    case SoObliqueSlice::ALPHA_AS_IS: clutalphause = CvrCLUT::ALPHA_AS_IS; break;
    case SoObliqueSlice::ALPHA_OPAQUE: clutalphause = CvrCLUT::ALPHA_OPAQUE; break;
    case SoObliqueSlice::ALPHA_BINARY: clutalphause = CvrCLUT::ALPHA_BINARY; break;
    default: assert(0 && "invalid alphause value"); break;
  }
  CvrCLUT * c = CvrVoxelChunk::getCLUT(tfelement, clutalphause);
  if (this->clut != c) {  
    this->setPalette(c);  
  }

  // This must be done, as we want to control stuff in the GL state
  // machine. Without it, state changes could trigger outside our
  // control.
  SoGLLazyElement::getInstance(state)->send(state, SoLazyElement::ALL_MASK);

  glPushAttrib(GL_ALL_ATTRIB_BITS);

  glDisable(GL_LIGHTING);
  glEnable(GL_TEXTURE_3D);

  // FIXME: how does the blending cooperate with the other geometry in
  // a Coin scene graph? Do we need to delay rendering? 20021109 mortene.
  //
  // UPDATE: yes, we do, or geometry that is traversed after this will
  // not be rendered through the transparent parts because it fails
  // the Z-buffer test.

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  assert(glGetError() == GL_NO_ERROR);
   
  this->volumecube->renderObliqueSlice(action, plane);

  glPopAttrib();
}
