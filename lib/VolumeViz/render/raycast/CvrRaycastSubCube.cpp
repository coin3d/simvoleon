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

#include "CvrRaycastSubCube.h"

#include <Inventor/SbLinear.h>
#include <Inventor/elements/SoViewportRegionElement.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoClipPlaneElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/actions/SoGLRenderAction.h>


#include <VolumeViz/render/common/CvrTextureObject.h>

#include <RenderManager.h>

using namespace CLVol;

CvrRaycastSubCube::CvrRaycastSubCube(const SoGLRenderAction * action,
                                     const CvrTextureObject * texobj,
                                     const SbVec3f & cubeorigo,
                                     const SbVec3s & cubesize,
                                     CLVol::RenderManager * rm)
  : dimensions(cubesize), origo(cubeorigo), textureobject(texobj), rendermanager(rm)
{
  this->clut = NULL;
  this->textureobject->ref();
}


CvrRaycastSubCube::~CvrRaycastSubCube()
{
  this->textureobject->unref();
}


void
CvrRaycastSubCube::setPalette(const CvrCLUT * newclut)
{
  assert(newclut != NULL);

  if (this->clut) { this->clut->unref(); }

  this->clut = newclut;
  this->clut->ref();
}


void
CvrRaycastSubCube::setRenderTarget(GLuint targetfbo,
                                   unsigned int viewportx,
                                   unsigned int viewporty,
                                   unsigned int viewportwidth,
                                   unsigned int viewportheight)
{
  this->rendermanager->setRenderTarget(targetfbo,
                                       viewportx,
                                       viewporty,
                                       viewportwidth,
                                       viewportheight);
}


void 
CvrRaycastSubCube::attachGLLayers(std::vector<GLuint> layerscolortexture,
                                  std::vector<GLuint> layersdepthtexture,
                                  unsigned int layerswidth,
                                  unsigned int layersheight)
{
  this->rendermanager->attachGLLayers(layerscolortexture,
                                      layersdepthtexture,
                                      layerswidth,
                                      layersheight);
}


void
CvrRaycastSubCube::detachGLResources()
{
  this->rendermanager->detachGLResources();
}


void
CvrRaycastSubCube::setTransferFunction(std::vector<CLVol::TransferFunctionPoint> & tf)
{
  this->rendermanager->setTransferFunction(tf);
}


void
CvrRaycastSubCube::render(const SoGLRenderAction * action, SbViewVolume adjustedviewvolume)
{
  assert(this->rendermanager && "No rendermanager initialized");
  SoState * state = action->getState();

  std::vector<GLfloat> clipplanes;
  clipplanes.clear();

  const SbMatrix mm = SoModelMatrixElement::get(state);
  const SbMatrix projectionmatrix = adjustedviewvolume.getMatrix();
  const SbMatrix pminv = (mm*projectionmatrix).inverse();  

  // FIXME: get the clipplanes stuff working. (20100910 handegar)
  /*
  const SoClipPlaneElement * cpe = SoClipPlaneElement::getInstance(state);
  int num = cpe->getNum();
  for (int i=0;i<num;++i) {
    SbPlane p = cpe->get(i, false);
    SbVec3f n = p.getNormal();
    clipplanes.push_back(n[0]);
    clipplanes.push_back(n[1]);
    clipplanes.push_back(n[2]);
    clipplanes.push_back(p.getDistanceFromOrigin());    
    printf("%d clipplane=[%f, %f, %f,  %f]\n", i, n[0], n[1], n[2], p.getDistanceFromOrigin());
  }
  */    
         
  GLfloat projmarray[16];
  GLfloat mminvarray[16];
  // Doh....  Theres got to be a better way... :-(
  for (int i=0;i<4;++i) {
    for (int j=0;j<4;++j) {
      projmarray[i*4 + j] = projectionmatrix[i][j];
      mminvarray[i*4 + j] = pminv[i][j];
    }
  }
   
  this->rendermanager->setVolumeTexture(this->textureobject->getGLTexture(action));
   
  this->rendermanager->render((const GLfloat *) &projmarray, 
                              (const GLfloat *) &mminvarray,
                              clipplanes);
}

