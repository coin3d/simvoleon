/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/


#include <VolumeViz/nodes/SoVolumeRendering.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/elements/SoVolumeDataElement.h>
#include <VolumeViz/nodes/SoVolumeRender.h>

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif // HAVE_WINDOWS_H*/
#include <GL/gl.h>

// *************************************************************************

SO_NODE_SOURCE(SoVolumeRendering);

// *************************************************************************

class SoVolumeRenderingP{
public:
  SoVolumeRenderingP(SoVolumeRendering * master) {
    this->master = master;
  }

private:
  SoVolumeRendering * master;
};


#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

// *************************************************************************

/*!
  Constructor.
*/
SoVolumeRendering::SoVolumeRendering(void)
{
  SO_NODE_CONSTRUCTOR(SoVolumeRendering);

  PRIVATE(this) = new SoVolumeRenderingP(this);
  
  SO_NODE_DEFINE_ENUM_VALUE(HW_Feature, HW_VOLUMEPRO);
  SO_NODE_DEFINE_ENUM_VALUE(HW_Feature, HW_3DTEXMAP);
  SO_NODE_DEFINE_ENUM_VALUE(HW_Feature, HW_TEXCOLORMAP);
  SO_NODE_DEFINE_ENUM_VALUE(HW_Feature, HW_TEXCOMPRESSION);
}//Constructor



/*!
  Destructor.
*/
SoVolumeRendering::~SoVolumeRendering()
{
  delete PRIVATE(this);
}




// Doc from parent class.
void
SoVolumeRendering::initClass(void)
{
  static int first = 0;
  if (first == 1) return;
  first = 1;

  SO_NODE_INIT_CLASS(SoVolumeRendering, SoNode, "VolumeRendering");
  
  SoVolumeData::initClass();
  SoVolumeRender::initClass();
  SoVolumeDataElement::initClass();
}// initClass


void 
SoVolumeRendering::init()
{}

SoVolumeRendering::HW_SupportStatus 
SoVolumeRendering::isSupported(HW_Feature feature)
{ return UNKNOWN;}

