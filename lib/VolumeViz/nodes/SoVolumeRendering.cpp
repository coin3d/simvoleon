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

SoVolumeRendering::SoVolumeRendering(void)
{
  SO_NODE_CONSTRUCTOR(SoVolumeRendering);

  PRIVATE(this) = new SoVolumeRenderingP(this);

  SO_NODE_DEFINE_ENUM_VALUE(HW_Feature, HW_VOLUMEPRO);
  SO_NODE_DEFINE_ENUM_VALUE(HW_Feature, HW_3DTEXMAP);
  SO_NODE_DEFINE_ENUM_VALUE(HW_Feature, HW_TEXCOLORMAP);
  SO_NODE_DEFINE_ENUM_VALUE(HW_Feature, HW_TEXCOMPRESSION);

  // FIXME: missing a init call of the field? 20021106 mortene.
}

SoVolumeRendering::~SoVolumeRendering()
{
  delete PRIVATE(this);
}

// Doc from parent class.
void
SoVolumeRendering::initClass(void)
{
  // FIXME: is the last argument really correct? 20021106 mortene.
  SO_NODE_INIT_CLASS(SoVolumeRendering, SoNode, "VolumeRendering");

  SoVolumeData::initClass();
  SoVolumeDataElement::initClass();
  SoVolumeRender::initClass();
  SoROI::initClass();
  SoTransferFunction::initClass();
  SoTransferFunctionElement::initClass();

  SO_ENABLE(SoGLRenderAction, SoTransferFunctionElement);
  SO_ENABLE(SoGLRenderAction, SoVolumeDataElement);
}



// FIXME: These functions are still to be implemented.
// torbjorv 08282002

void
SoVolumeRendering::init(void)
{
}

SoVolumeRendering::HW_SupportStatus
SoVolumeRendering::isSupported(HW_Feature feature)
{
  return SoVolumeRendering::UNKNOWN;
}
