/**************************************************************************\
 *
 *  This file is part of the SIM Voleon visualization library.
 *  Copyright (C) by Kongsberg Oil & Gas Technologies.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  ("GPL") version 2 as published by the Free Software Foundation.
 *  See the file LICENSE.GPL at the root directory of this source
 *  distribution for additional information about the GNU GPL.
 *
 *  For using SIM Voleon with software that can not be combined with
 *  the GNU GPL, and for taking advantage of the additional benefits
 *  of our support services, please contact Kongsberg Oil & Gas
 *  Technologies about acquiring a SIM Voleon Professional Edition
 *  License.
 *
 *  See http://www.coin3d.org/ for more information.
 *
 *  Kongsberg Oil & Gas Technologies, Bygdoy Alle 5, 0257 Oslo, NORWAY.
 *  http://www.sim.no/  sales@sim.no  coin-support@coin3d.org
 *
\**************************************************************************/

#include "CvrRaycastVolume.h"

#include <Inventor/C/glue/gl.h>
#include <Inventor/system/gl.h>
#include <Inventor/elements/SoViewportRegionElement.h>
#include <Inventor/elements/SoGLLazyElement.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/SbVec3s.h>

#include <VolumeViz/render/raycast/CvrRaycastSubCube.h>

#include <RenderManager.h>


CvrRaycastVolume::CvrRaycastVolume(const SoGLRenderAction * action)
  : CvrRaycastRenderBase(action)
{
}


CvrRaycastVolume::~CvrRaycastVolume()
{
}


void 
CvrRaycastVolume::render(const SoGLRenderAction * action)
{
  SoState * state = action->getState();

  // This must be done, as we want to control stuff in the GL state
  // machine. Without it, state changes could trigger outside our
  // control.
  SoGLLazyElement::getInstance(state)->send(state, SoLazyElement::ALL_MASK);

  glPushAttrib(GL_ALL_ATTRIB_BITS);
  const uint32_t glctxid = action->getCacheContext();
  const cc_glglue * glw = cc_glglue_instance(glctxid);
  const SbViewportRegion & vpr = SoViewportRegionElement::get(state);
  const SbVec2s size = vpr.getWindowSize();  
 
  SbList<SubCube *> subcuberenderorder = this->processSubCubes(action);


  // FIXME: Needed? (20101022 handegar)
  /*
  glEnable(GL_DEPTH_TEST);      
  // FIXME: Use glglue for EXT calls (20100914 handegar)
  cc_glglue_glBindFramebuffer(glw, GL_FRAMEBUFFER, this->gllayerfbos[1]);        
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  */    
    
  for (int i=0;i<subcuberenderorder.getLength();++i) {    
    cc_glglue_glBindFramebuffer(glw, GL_READ_FRAMEBUFFER, 0);
    cc_glglue_glBindFramebuffer(glw, GL_DRAW_FRAMEBUFFER, this->gllayerfbos[1]);
    // FIXME: glBlitFramebuffer is not bound by glue. Must fix in
    // Coin. (20100914 handegar)

    // FIXME: Only blit the part of the viewport which were actually
    // used. This does not seem like a gpu-hog, however. (20101012
    // handegar)
    glBlitFramebuffer(0, 0, size[0], size[1], 0, 0, size[0], size[1],
                      GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    assert(glGetError() == GL_NO_ERROR);
    
    // -- copy depth from solid pass into depth of transparent pass
    cc_glglue_glBindFramebuffer(glw, GL_READ_FRAMEBUFFER, this->gllayerfbos[1]);
    cc_glglue_glBindFramebuffer(glw, GL_DRAW_FRAMEBUFFER, this->gllayerfbos[0]);
    // FIXME: glBlitFramebuffer is not bound by glue. Must fix in Coin. (20100914 handegar)
    glBlitFramebuffer(0, 0, size[0], size[1], 0, 0, size[0], size[1],
                      GL_DEPTH_BUFFER_BIT, GL_NEAREST);
            
    cc_glglue_glBindFramebuffer(glw, GL_FRAMEBUFFER, this->gllayerfbos[0]);
    glClear(GL_COLOR_BUFFER_BIT);

    glDisable(GL_BLEND);
    
    const SubCube * cubeitem = subcuberenderorder[i];
    assert(cubeitem);       
    assert(cubeitem->cube);       

    
    cubeitem->cube->renderVolume(action); 
  }

  glPopAttrib();  
  
}

