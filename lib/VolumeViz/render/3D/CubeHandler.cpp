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
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/SbTime.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/elements/SoGLLazyElement.h>

#include <VolumeViz/render/3D/CvrCubeHandler.h>

#include <VolumeViz/elements/SoVolumeDataElement.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/readers/SoVolumeReader.h>
#include <VolumeViz/render/3D/Cvr3DTexCube.h>
#include <VolumeViz/nodes/SoTransferFunction.h>
#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <VolumeViz/misc/CvrUtil.h>
#include <VolumeViz/misc/CvrCLUT.h>
#include <VolumeViz/misc/CvrVoxelChunk.h>


CvrCubeHandler::CvrCubeHandler(const SbVec3s & voldatadims,
                               SoVolumeReader * reader)
{
  this->voldatadims[0] = voldatadims[0];
  this->voldatadims[1] = voldatadims[1];
  this->voldatadims[2] = voldatadims[2];
  this->volumecube = NULL;
  this->clut = NULL;

  this->reader = reader;
}

CvrCubeHandler::~CvrCubeHandler()
{
  if (this->clut) { this->clut->unref(); }
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

  if (this->clut) { this->clut->unref(); }
  this->clut = c;
  this->clut->ref();  
  this->volumecube->setPalette(c);

}

void
CvrCubeHandler::render(SoGLRenderAction * action, unsigned int numslices,
                       Cvr3DTexSubCube::Interpolation interpolation,
                       CvrCubeHandler::Composition composition,
                       SoVolumeRender::SoVolumeRenderAbortCB * abortfunc,
                       void * abortcbdata)
{
  SoState * state = action->getState();

  const SoVolumeDataElement * volumedataelement = SoVolumeDataElement::getInstance(state);
  assert(volumedataelement != NULL);
  SoVolumeData * volumedata = volumedataelement->getVolumeData();
  assert(volumedata != NULL);
 
  // FIXME: should have an assert-check that the volume dimensions
  // hasn't changed versus our this->voldatadims  
  if (this->volumecube == NULL) {  
    this->volumecube = new Cvr3DTexCube(this->reader);
  }

  const SoTransferFunctionElement * tfelement = SoTransferFunctionElement::getInstance(state);
  const CvrCLUT * c = CvrVoxelChunk::getCLUT(tfelement);
  if (this->clut != c) { this->setPalette(c); }

  // This must be done, as we want to control stuff in the GL state
  // machine. Without it, state changes could trigger outside our
  // control.
  SoGLLazyElement::getInstance(state)->send(state, SoLazyElement::ALL_MASK);

  glPushAttrib(GL_ALL_ATTRIB_BITS);

  glDisable(GL_LIGHTING);

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

    
  SbVec3s tmp = volumedataelement->getVoxelCubeDimensions();
  SbVec3f dimensions;
  dimensions[0] = (float) tmp[0];
  dimensions[1] = (float) tmp[1];
  dimensions[2] = (float) tmp[2];

  SbVec3f origo;
  origo = SbVec3f(-dimensions[0] / 2.0f, -dimensions[1] / 2.0f, -dimensions[2] / 2.0f);

  if (abortfunc != NULL) {
    this->volumecube->setAbortCallback(abortfunc, abortcbdata);
  }
  this->volumecube->render(action, origo, interpolation, numslices);

  glPopAttrib();
}
