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
#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <VolumeViz/nodes/SoVolumeRender.h>
#include <VolumeViz/nodes/SoROI.h>

// FIXME: All extensionstuff should be moved to a proper home in Coin.
// torbjorv 08282002


PFNGLCOMPRESSEDTEXIMAGE3DARBPROC glCompressedTexImage3DARB;
PFNGLCOMPRESSEDTEXIMAGE2DARBPROC glCompressedTexImage2DARB;
PFNGLCOMPRESSEDTEXIMAGE1DARBPROC glCompressedTexImage1DARB;
PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC glCompressedTexSubImage3DARB;
PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC	glCompressedTexSubImage2DARB;
PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC	glCompressedTexSubImage1DARB;
PFNGLGETCOMPRESSEDTEXIMAGEARBPROC	glGetCompressedTexImageARB;


PFNGLCOLORTABLEEXTPROC glColorTableEXT;
PFNGLCOLORSUBTABLEEXTPROC glColorSubTableEXT;
PFNGLGETCOLORTABLEEXTPROC glGetColorTableEXT;
PFNGLGETCOLORTABLEPARAMETERIVEXTPROC glGetColorTableParameterivEXT;
PFNGLGETCOLORTABLEPARAMETERFVEXTPROC glGetColorTableParameterfvEXT;



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
  SoVolumeDataElement::initClass();
  SoVolumeRender::initClass();
  SoROI::initClass();
  SoTransferFunction::initClass();
  SoTransferFunctionElement::initClass();
}// initClass



// FIXME: These functions are still to be implemented. 
// torbjorv 08282002

void 
SoVolumeRendering::init()
{}

SoVolumeRendering::HW_SupportStatus 
SoVolumeRendering::isSupported(HW_Feature feature)
{ return UNKNOWN;}

