/*!
  \class SoVolumeRendering VolumeViz/nodes/SoVolumeRendering.h
  \brief Abstract base class for all nodes related to volume rendering.
  \ingroup volviz

  The sole purpose of this class is really just to initialize the
  volume rendering framework.
*/
// FIXME: simplest programming example here on how to use the
// vol-rendering. 20021106 mortene.

#include <VolumeViz/nodes/SoVolumeRendering.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/elements/SoVolumeDataElement.h>
#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <VolumeViz/nodes/SoVolumeRender.h>
#include <VolumeViz/nodes/SoTransferFunction.h>
#include <VolumeViz/nodes/SoROI.h>
#include <VolumeViz/details/SoVolumeRenderDetail.h>
#include <VolumeViz/details/SoOrthoSliceDetail.h>
#include <VolumeViz/details/SoObliqueSliceDetail.h>
#include <VolumeViz/render/2D/CvrTextureObject.h>

#include <Inventor/actions/SoGLRenderAction.h>


// *************************************************************************

SO_NODE_SOURCE(SoVolumeRendering);

// *************************************************************************

class SoVolumeRenderingP {
public:
  static SbBool wasinitialized;
};

SbBool SoVolumeRenderingP::wasinitialized = FALSE;

#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

// *************************************************************************

SoVolumeRendering::SoVolumeRendering(void)
{
  SO_NODE_CONSTRUCTOR(SoVolumeRendering);

  PRIVATE(this) = NULL; // pimpl-class not yet needed
}

SoVolumeRendering::~SoVolumeRendering()
{
  delete PRIVATE(this);
}

/*!
  Does all necessary class initializations of the volume rendering
  system.

  Application programmers must call this method explicitly at the
  start of the application, before any volume rendering nodes are
  made. It must be invoked \e after SoXt::init() / SoQt::init() /
  SoWin::init(), though.
 */
void
SoVolumeRendering::init(void)
{
  if (SoVolumeRenderingP::wasinitialized) return;
  SoVolumeRenderingP::wasinitialized = TRUE;

  SoVolumeDataElement::initClass();
  SoTransferFunctionElement::initClass();

  SoVolumeRendering::initClass();
  SoVolumeData::initClass();
  SoROI::initClass();
  SoTransferFunction::initClass();

  SoVolumeRender::initClass();

  SoVolumeRenderDetail::initClass();
  SoOrthoSliceDetail::initClass();
  SoObliqueSliceDetail::initClass();

  // Internal classes follows:

  CvrTextureObject::initClass();
}

void
SoVolumeRendering::initClass(void)
{
  SO_NODE_INIT_CLASS(SoVolumeRendering, SoNode, "SoNode");
}

SoVolumeRendering::HW_SupportStatus
SoVolumeRendering::isSupported(HW_Feature feature)
{
  switch (feature) {

  case SoVolumeRendering::HW_VOLUMEPRO:
    return SoVolumeRendering::NO;

  case SoVolumeRendering::HW_3DTEXMAP:
    // FIXME: update this when support is in place. 20021106 mortene.
    return SoVolumeRendering::NO;

  case SoVolumeRendering::HW_TEXCOLORMAP:
    // FIXME: update this when support is in place. 20021106 mortene.
    return SoVolumeRendering::NO;

  case SoVolumeRendering::HW_TEXCOMPRESSION:
    // FIXME: update this when support is in place. 20021106 mortene.
    return SoVolumeRendering::NO;

  default:
    assert(FALSE && "unknown feature");
    break;
  }

  return SoVolumeRendering::NO;
}
