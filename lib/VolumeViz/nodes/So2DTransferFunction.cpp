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

#include "So2DTransferFunction.h"

#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoCallbackAction.h>
#include <Inventor/actions/SoPickAction.h>
#include <Inventor/sensors/SoFieldSensor.h>

#include <VolumeViz/elements/So2DTransferFunctionElement.h>

SO_NODE_SOURCE(So2DTransferFunction);

// *************************************************************************

struct TransferPt {
  SbColor4f c;
  float p;
};

SbList<TransferPt> GrayColormap;
SbList<TransferPt> TemperatureColormap;
SbList<TransferPt> PhysicsColormap;
SbList<TransferPt> StandardColormap;
SbList<TransferPt> GlowColormap;
SbList<TransferPt> BlueRedColormap;
SbList<TransferPt> SeismicColormap;

void 
create_predef_colormaps(void) 
{
  TransferPt pt;
  // Gray/Grey
  pt.c[0]=0; pt.c[1]=0; pt.c[2]=0; pt.c[3]=0; pt.p=0.0f; GrayColormap.append(pt);
  pt.c[0]=1; pt.c[1]=1; pt.c[2]=1; pt.c[3]=1; pt.p=1.0f; GrayColormap.append(pt);
  // Temperature
  pt.c[0]=1; pt.c[1]=1; pt.c[2]=1; pt.c[3]=1; pt.p=0.0f; TemperatureColormap.append(pt);
  pt.c[0]=0; pt.c[1]=0; pt.c[2]=1; pt.c[3]=1; pt.p=1.0f/3; TemperatureColormap.append(pt);
  pt.c[0]=1; pt.c[1]=0; pt.c[2]=0; pt.c[3]=1; pt.p=2.0f/3; TemperatureColormap.append(pt);
  pt.c[0]=1; pt.c[1]=1; pt.c[2]=0; pt.c[3]=1; pt.p=1.0f; TemperatureColormap.append(pt);
  // Physics
  pt.c[0]=0; pt.c[1]=0; pt.c[2]=0; pt.c[3]=1; pt.p=0.0f; PhysicsColormap.append(pt);
  pt.c[0]=0; pt.c[1]=0; pt.c[2]=1; pt.c[3]=1; pt.p=1.0f/5; PhysicsColormap.append(pt);
  pt.c[0]=0; pt.c[1]=1; pt.c[2]=1; pt.c[3]=1; pt.p=2.0f/5; PhysicsColormap.append(pt);
  pt.c[0]=0; pt.c[1]=1; pt.c[2]=0; pt.c[3]=1; pt.p=3.0f/5; PhysicsColormap.append(pt);
  pt.c[0]=1; pt.c[1]=1; pt.c[2]=0; pt.c[3]=1; pt.p=4.0f/5; PhysicsColormap.append(pt);
  pt.c[0]=1; pt.c[1]=0; pt.c[2]=0; pt.c[3]=1; pt.p=1.0f; PhysicsColormap.append(pt);
  // Standard
  pt.c[0]=1; pt.c[1]=0; pt.c[2]=0; pt.c[3]=1; pt.p=0.0f; StandardColormap.append(pt);
  pt.c[0]=1; pt.c[1]=1; pt.c[2]=0; pt.c[3]=1; pt.p=1.0f/5; StandardColormap.append(pt);
  pt.c[0]=0; pt.c[1]=1; pt.c[2]=0; pt.c[3]=1; pt.p=2.0f/5; StandardColormap.append(pt);
  pt.c[0]=0; pt.c[1]=1; pt.c[2]=1; pt.c[3]=1; pt.p=3.0f/5; StandardColormap.append(pt);
  pt.c[0]=0; pt.c[1]=0; pt.c[2]=1; pt.c[3]=1; pt.p=4.0f/5; StandardColormap.append(pt);
  pt.c[0]=1; pt.c[1]=0; pt.c[2]=0; pt.c[3]=1; pt.p=1.0f; StandardColormap.append(pt);
  // Glow
  pt.c[0]=0; pt.c[1]=0; pt.c[2]=0; pt.c[3]=1; pt.p=0.0f; GlowColormap.append(pt);
  pt.c[0]=1; pt.c[1]=0; pt.c[2]=0; pt.c[3]=1; pt.p=1.0f/3; GlowColormap.append(pt);
  pt.c[0]=1; pt.c[1]=1; pt.c[2]=0; pt.c[3]=1; pt.p=2.0f/3; GlowColormap.append(pt);
  pt.c[0]=1; pt.c[1]=1; pt.c[2]=1; pt.c[3]=1; pt.p=1.0f; GlowColormap.append(pt);  
  // BlueRed
  pt.c[0]=0; pt.c[1]=0; pt.c[2]=1; pt.c[3]=1; pt.p=0.0f; BlueRedColormap.append(pt);
  pt.c[0]=1; pt.c[1]=0; pt.c[2]=0; pt.c[3]=1; pt.p=1.0f; BlueRedColormap.append(pt);
  // Seismic
  pt.c[0]=0; pt.c[1]=0; pt.c[2]=1; pt.c[3]=1; pt.p=0.0f; SeismicColormap.append(pt);
  pt.c[0]=0.5f; pt.c[1]=0; pt.c[2]=0; pt.c[3]=0.5f; pt.p=1.0f/2; SeismicColormap.append(pt);
  pt.c[0]=1; pt.c[1]=0; pt.c[2]=0; pt.c[3]=1; pt.p=1.0f; SeismicColormap.append(pt);
}


// *************************************************************************


class So2DTransferFunctionP {
public:
  So2DTransferFunctionP(So2DTransferFunction * master) {
    this->master = master;
    this->predefsensor = new SoFieldSensor(So2DTransferFunctionP::predefCB, this);
    this->predefsensor->attach(&master->predefColorMap);    

  }

  ~So2DTransferFunctionP() {
    delete predefsensor;
  }

  static void predefCB(void * data, SoSensor * sensor);
  SoFieldSensor * predefsensor;
  So2DTransferFunction * master;
};

#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

void
So2DTransferFunctionP::predefCB(void * data, SoSensor * sensor)
{
  So2DTransferFunctionP * thisp = (So2DTransferFunctionP *) data;
  int t = PUBLIC(thisp)->predefColorMap.getValue();
  
  SbList<TransferPt> tf;
  switch(t) {
  case So2DTransferFunction::GRAY: tf = GrayColormap; break;
  case So2DTransferFunction::TEMPERATURE: tf = TemperatureColormap; break;
  case So2DTransferFunction::PHYSICS: tf = PhysicsColormap; break;
  case So2DTransferFunction::STANDARD: tf = StandardColormap; break;
  case So2DTransferFunction::GLOW: tf = GlowColormap; break;
  case So2DTransferFunction::BLUE_RED: tf = BlueRedColormap; break;
  case So2DTransferFunction::SEISMIC: tf = SeismicColormap; break;
  default: return;
  }

  PUBLIC(thisp)->colors.setNum(0);
  PUBLIC(thisp)->colorPoints.setNum(0);
  for (int i=0;i<tf.getLength();++i) {
    PUBLIC(thisp)->colors.set1Value(i, tf[i].c);
    PUBLIC(thisp)->colorPoints.set1Value(i, tf[i].p);
  }
}


// *************************************************************************


So2DTransferFunction::So2DTransferFunction()
{
  SO_NODE_CONSTRUCTOR(So2DTransferFunction);  

  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, GREY);
  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, GRAY);
  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, TEMPERATURE);
  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, PHYSICS);
  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, STANDARD);
  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, GLOW);
  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, BLUE_RED);
  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, SEISMIC);
  SO_NODE_SET_SF_ENUM_TYPE(predefColorMap, PredefColorMap);

  SO_NODE_ADD_FIELD(colors, (SbColor4f(0.7f, 0.7f, 0.7f, 1.0f)));
  SO_NODE_ADD_FIELD(colorPoints, (0.0f));
  SO_NODE_ADD_FIELD(gradientPoints, (SbVec2f(0, 0)));  

  PRIVATE(this) = new So2DTransferFunctionP(this);
}


So2DTransferFunction::~So2DTransferFunction()
{
  delete PRIVATE(this);
}


void
So2DTransferFunction::initClass(void)
{
  SO_NODE_INIT_CLASS(So2DTransferFunction, SoNode, "SoNode");

  SO_ENABLE(SoGLRenderAction, So2DTransferFunctionElement);
  SO_ENABLE(SoCallbackAction, So2DTransferFunctionElement);
  SO_ENABLE(SoPickAction, So2DTransferFunctionElement);

  create_predef_colormaps();
}


void
So2DTransferFunction::doAction(SoAction * action)
{
  SoState * s = action->getState();
  So2DTransferFunctionElement::setTransferFunction(s, this);
}


void
So2DTransferFunction::GLRender(SoGLRenderAction * action)
{
  this->doAction(action);
}


void
So2DTransferFunction::callback(SoCallbackAction * action)
{
  this->doAction(action);
}


void
So2DTransferFunction::pick(SoPickAction * action)
{
  this->doAction(action);
}

#undef PRIVATE
#undef PUBLIC
