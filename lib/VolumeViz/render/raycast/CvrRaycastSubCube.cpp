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
#include <VolumeViz/render/raycast/CvrRaycastTexture.h>

#include <RenderManager.h>

using namespace CLVol;

CvrRaycastSubCube::CvrRaycastSubCube(const SoGLRenderAction * action,
                                     const CvrRaycastTexture * texobj,
                                     const SbBox3s cubebbox,
                                     const SbVec3s totalsize,
                                     CLVol::RenderManager * rm)
  : textureobject(texobj), bbox(cubebbox), totalsize(totalsize), rendermanager(rm)
{
  this->textureobject->ref();
}


CvrRaycastSubCube::~CvrRaycastSubCube()
{
  this->textureobject->unref();
}


void 
CvrRaycastSubCube::setTransferFunction(std::vector<CLVol::TransferFunctionPoint> & tf)
{
  // FIXME: Cleanup so that this const-cast becomes obsolete. (20101011 handegar)
  CvrRaycastTexture * rt = (CvrRaycastTexture *) this->textureobject;
  rt->setTransferFunction(tf);
}


void
CvrRaycastSubCube::render(const SoGLRenderAction * action, SbViewVolume adjustedviewvolume)
{
  assert(this->rendermanager && "No rendermanager initialized");
  SoState * state = action->getState();

  std::vector<GLfloat> clipplanes;
  clipplanes.clear();

  SbMatrix t, s;
  const SbMatrix mm = SoModelMatrixElement::get(state);
  const SbVec3s span = this->bbox.getSize();
  const SbVec3f origo = this->bbox.getCenter();  
  s.setScale(SbVec3f(span[0], span[1], span[2]));   
  t.setTranslate(SbVec3f(origo[0] - (span[0] + this->totalsize[0])/2.0f,
                         origo[1] - (span[1] + this->totalsize[1])/2.0f,
                         origo[2] - (span[2] + this->totalsize[2])/2.0f));     
  const SbMatrix projectionmatrix = adjustedviewvolume.getMatrix();
  const SbMatrix pminv = (s*t*mm*projectionmatrix).inverse();  
  

  // FIXME: get the clipplanes stuff working. (20100910 handegar)
  /*
  const SoClipPlaneElement * cpe = SoClipPlaneElement::getInstance(state);
  const int num = cpe->getNum();
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
   
  this->rendermanager->bindVoxelData(this->textureobject->getVoxelData());     
  this->rendermanager->render((const GLfloat *) &projmarray, 
                              (const GLfloat *) &mminvarray,
                              clipplanes);  
}

