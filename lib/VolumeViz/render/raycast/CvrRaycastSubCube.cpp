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
#include <Inventor/elements/SoViewingMatrixElement.h>
#include <Inventor/elements/SoProjectionMatrixElement.h>
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
CvrRaycastSubCube::renderVolume(const SoGLRenderAction * action)
{
  assert(this->rendermanager && "No rendermanager initialized");
  SoState * state = action->getState();

  std::vector<GLfloat> clipplanes;
  clipplanes.clear();

  const SbMatrix mm = SoModelMatrixElement::get(state);
  const SbVec3s span = this->bbox.getSize();
  const SbVec3f origo = this->bbox.getCenter();  

  // transform unit cube to volume spatial represenation
  SbMatrix t, s;
  s.setScale(SbVec3f(span[0], span[1], span[2]));   
  t.setTranslate(SbVec3f(origo[0] - (span[0] + this->totalsize[0])/2.0f, 
                         origo[1] - (span[1] + this->totalsize[1])/2.0f, 
                         origo[2] - (span[2] + this->totalsize[2])/2.0f)); 


  const SbMatrix P = SoProjectionMatrixElement::get(state);
  const SbMatrix viewingmatrix = SoViewingMatrixElement::get(state);
  const SbMatrix M = mm*viewingmatrix;

  // do transformations (opposite order as left-handed is exposed)
  const SbMatrix PMi = (s*t*M*P).inverse();
  
  // inverse of world to cam space (for planes)
  const SbMatrix WtCi = (s*t*mm).inverse();
  
  const SoClipPlaneElement * cpe = SoClipPlaneElement::getInstance(state);
  const int num = cpe->getNum();
  for (int i=0;i<num;++i) {
    SbPlane p = cpe->get(i, true);
    p.transform( WtCi );
    
    SbVec3f n = p.getNormal();    
    clipplanes.push_back(n[0]);
    clipplanes.push_back(n[1]);
    clipplanes.push_back(n[2]);
    clipplanes.push_back(-p.getDistanceFromOrigin());
  }
  
  this->rendermanager->bindVoxelData(this->textureobject->getVoxelData()); 
  this->rendermanager->render((const GLfloat *) P[0],
                              (const GLfloat *) PMi[0],
                              clipplanes);  
}


void 
CvrRaycastSubCube::renderFaces(const SoGLRenderAction * action)
{
  assert(0 && "Not implemented yet");
}

