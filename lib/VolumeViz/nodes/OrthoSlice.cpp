#include <VolumeViz/nodes/SoOrthoSlice.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoRayPickAction.h>
#include <VolumeViz/elements/SoVolumeDataElement.h>
#include <VolumeViz/elements/SoTransferFunctionElement.h>

// *************************************************************************

SO_NODE_SOURCE(SoOrthoSlice);

// *************************************************************************

class SoOrthoSliceP {
public:
  SoOrthoSliceP(SoOrthoSlice * master)
  {
    this->master = master;
  }

private:
  SoOrthoSlice * master;
};

#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

// *************************************************************************

SoOrthoSlice::SoOrthoSlice(void)
{
  SO_NODE_CONSTRUCTOR(SoOrthoSlice);

  PRIVATE(this) = new SoOrthoSliceP(this);

  SO_NODE_DEFINE_ENUM_VALUE(Axis, X);
  SO_NODE_DEFINE_ENUM_VALUE(Axis, Y);
  SO_NODE_DEFINE_ENUM_VALUE(Axis, Z);
  SO_NODE_SET_SF_ENUM_TYPE(axis, Axis);

  SO_NODE_DEFINE_ENUM_VALUE(Interpolation, NEAREST);
  SO_NODE_DEFINE_ENUM_VALUE(Interpolation, LINEAR);
  SO_NODE_SET_SF_ENUM_TYPE(interpolation, Interpolation);

  SO_NODE_DEFINE_ENUM_VALUE(AlphaUse, ALPHA_AS_IS);
  SO_NODE_DEFINE_ENUM_VALUE(AlphaUse, ALPHA_OPAQUE);
  SO_NODE_DEFINE_ENUM_VALUE(AlphaUse, ALPHA_BINARY);
  SO_NODE_SET_SF_ENUM_TYPE(alphaUse, AlphaUse);

  SO_NODE_DEFINE_ENUM_VALUE(ClippingSide, FRONT);
  SO_NODE_DEFINE_ENUM_VALUE(ClippingSide, BACK);
  SO_NODE_SET_SF_ENUM_TYPE(clippingSide, ClippingSide);

  SO_NODE_ADD_FIELD(sliceNumber, (0));
  SO_NODE_ADD_FIELD(axis, (Z));
  SO_NODE_ADD_FIELD(interpolation, (LINEAR));
  SO_NODE_ADD_FIELD(alphaUse, (ALPHA_BINARY));
  SO_NODE_ADD_FIELD(clippingSide, (BACK));
  SO_NODE_ADD_FIELD(clipping, (FALSE));
}

SoOrthoSlice::~SoOrthoSlice()
{
  delete PRIVATE(this);
}

// Doc from parent class.
void
SoOrthoSlice::initClass(void)
{
  SO_NODE_INIT_CLASS(SoOrthoSlice, SoShape, "SoShape");

  SO_ENABLE(SoGLRenderAction, SoVolumeDataElement);
  SO_ENABLE(SoGLRenderAction, SoTransferFunctionElement);

  SO_ENABLE(SoRayPickAction, SoVolumeDataElement);
  SO_ENABLE(SoRayPickAction, SoTransferFunctionElement);
}

// doc in super
SbBool
SoOrthoSlice::affectsState(void) const
{
  // FIXME: should return FALSE when clipping plane is inactive
  return TRUE;
}

void
SoOrthoSlice::GLRender(SoGLRenderAction * action)
{
  // FIXME: implement
}

void
SoOrthoSlice::rayPick(SoRayPickAction * action)
{
  // FIXME: implement
}

void
SoOrthoSlice::generatePrimitives(SoAction * action)
{
  // FIXME: implement
}

void
SoOrthoSlice::computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center)
{
  // FIXME: implement
}
