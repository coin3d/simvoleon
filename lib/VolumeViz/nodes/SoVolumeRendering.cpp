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
#include <VolumeViz/misc/SoVolumeDataPage.h>
#include <VolumeViz/misc/SoVolumeDataSlice.h>
#include <VolumeViz/nodes/SoROI.h>
#include <Inventor/actions/SoGLRenderAction.h>


// *************************************************************************

SO_NODE_SOURCE(SoVolumeRendering);

// *************************************************************************

class SoVolumeRenderingP {
public:
};

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
 */
void
SoVolumeRendering::init(void)
{
  SoVolumeDataElement::initClass();
  SoTransferFunctionElement::initClass();

  SoVolumeRendering::initClass();
  SoVolumeData::initClass();
  SoROI::initClass();
  SoTransferFunction::initClass();

  SoVolumeRender::initClass();
}

void
SoVolumeRendering::initClass(void)
{
  SO_NODE_INIT_CLASS(SoVolumeRendering, SoNode, "SoNode");

  SO_ENABLE(SoGLRenderAction, SoTransferFunctionElement);
  SO_ENABLE(SoGLRenderAction, SoVolumeDataElement);
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
