/*!
  \class SoTransferFunction VolumeViz/nodes/SoTransferFunction.h
  \brief Contains the transfer function definition.
  \ingroup volviz
*/

#include <VolumeViz/nodes/SoTransferFunction.h>

#include <string.h>
#include <Inventor/C/tidbits.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoCallbackAction.h>
#include <Inventor/actions/SoPickAction.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/system/gl.h>
#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <VolumeViz/misc/CvrVoxelChunk.h>
#include <VolumeViz/render/2D/CvrTextureObject.h>

// *************************************************************************

SO_NODE_SOURCE(SoTransferFunction);

// *************************************************************************

class SoTransferFunctionP {
public:
  SoTransferFunctionP(SoTransferFunction * master) {
    this->master = master;

    // Init to lowest and highest uint16_t values.
    this->opaquethresholds[0] = 0;
    this->opaquethresholds[1] = (2 << 16) - 1;
  }

  int opaquethresholds[2];

private:
  SoTransferFunction * master;
};

#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

// *************************************************************************

SoTransferFunction::SoTransferFunction(void)
{
  SO_NODE_CONSTRUCTOR(SoTransferFunction);

  PRIVATE(this) = new SoTransferFunctionP(this);

  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, NONE);
  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, GREY);
  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, GRAY);
  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, TEMPERATURE);
  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, PHYSICS);
  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, STANDARD);
  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, GLOW);
  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, BLUE_RED);
  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, SEISMIC);
  SO_NODE_SET_SF_ENUM_TYPE(predefColorMap, PredefColorMap);

  SO_NODE_DEFINE_ENUM_VALUE(ColorMapType, ALPHA);
  SO_NODE_DEFINE_ENUM_VALUE(ColorMapType, LUM_ALPHA);
  SO_NODE_DEFINE_ENUM_VALUE(ColorMapType, RGBA);
  SO_NODE_SET_SF_ENUM_TYPE(colorMapType, ColorMapType);

  SO_NODE_ADD_FIELD(shift, (0));
  SO_NODE_ADD_FIELD(offset, (0));
  SO_NODE_ADD_FIELD(predefColorMap, (GREY));
  SO_NODE_ADD_FIELD(colorMapType, (RGBA));
  SO_NODE_ADD_FIELD(colorMap, (0));
}


SoTransferFunction::~SoTransferFunction()
{
  delete PRIVATE(this);
}


void
SoTransferFunction::initClass(void)
{
  SO_NODE_INIT_CLASS(SoTransferFunction, SoVolumeRendering, "SoVolumeRendering");

  SO_ENABLE(SoGLRenderAction, SoTransferFunctionElement);
  SO_ENABLE(SoCallbackAction, SoTransferFunctionElement);
  SO_ENABLE(SoPickAction, SoTransferFunctionElement);
}

void
SoTransferFunction::doAction(SoAction * action)
{
  SoState * s = action->getState();

  SoTransferFunctionElement::setTransferFunction(s, this);
  SoTransferFunctionElement::setTransparencyThresholds(s,
                                                       PRIVATE(this)->opaquethresholds[0],
                                                       PRIVATE(this)->opaquethresholds[1]);
}

void
SoTransferFunction::GLRender(SoGLRenderAction * action)
{
  this->doAction(action);
}

void
SoTransferFunction::callback(SoCallbackAction * action)
{
  this->doAction(action);
}

void
SoTransferFunction::pick(SoPickAction * action)
{
  this->doAction(action);
}

void
SoTransferFunction::reMap(int low, int high)
{
  assert(low <= high);

  PRIVATE(this)->opaquethresholds[0] = low;
  PRIVATE(this)->opaquethresholds[1] = high;
  
  // This is done to update our node-id, which should automatically
  // invalidate any 2D texture slices or 3D textures generated with
  // the previous colormap transfer. Also, our internal index tables
  // for speeding up transfers needs to be invalidated when these
  // values changes.
  //
  // These settings should really have been made public as fields, and
  // this would have been unnecessary. We can't do that without
  // breaking compatibility with TGS's VolumeViz, though.
  this->touch();
}
