#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoRayPickAction.h>
#include <Inventor/elements/SoGLClipPlaneElement.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/system/gl.h>

#include <VolumeViz/nodes/SoOrthoSlice.h>

#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <VolumeViz/elements/SoVolumeDataElement.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/nodes/SoTransferFunction.h>
#include <VolumeViz/render/2D/Cvr2DTexPage.h>
#include <VolumeViz/misc/CvrCLUT.h>
#include <VolumeViz/misc/CvrVoxelChunk.h>

// *************************************************************************

SO_NODE_SOURCE(SoOrthoSlice);

// *************************************************************************

class CachedPage {
public:
  CachedPage(Cvr2DTexPage * page, SoOrthoSlice * node)
  {
    this->page = page;
  }

  ~CachedPage()
  {
    delete this->page;
  }

  Cvr2DTexPage * getPage(void) const
  {
    return this->page;
  }

  const SoVolumeData * sovolumedata;
  Cvr2DTexPage * page;
};

// *************************************************************************

class SoOrthoSliceP {
public:
  SoOrthoSliceP(SoOrthoSlice * master)
  {
    this->master = master;
  }

  ~SoOrthoSliceP()
  {
    this->invalidatePageCache();
  }

  void invalidatePageCache(void)
  {
    for (int axis=0; axis < 3; axis++) {
      while (this->cachedpages[axis].getLength() > 0) {
        const int idx = this->cachedpages[axis].getLength() - 1;
        delete this->cachedpages[axis][idx];
        this->cachedpages[axis].remove(idx);
      }
    }
  }

  static void renderBox(SoGLRenderAction * action, SbBox3f box);

  Cvr2DTexPage * getPage(const int axis, const int slice, SoVolumeData * v);

private:
  SbList<CachedPage*> cachedpages[3];
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
  SO_ENABLE(SoPickAction, SoVolumeDataElement);

  SO_ENABLE(SoGLRenderAction, SoTransferFunctionElement);
  SO_ENABLE(SoPickAction, SoTransferFunctionElement);

  SO_ENABLE(SoGLRenderAction, SoGLClipPlaneElement);
  SO_ENABLE(SoPickAction, SoClipPlaneElement);
}

// doc in super
SbBool
SoOrthoSlice::affectsState(void) const
{
  return this->clipping.getValue();
}

// Doc from superclass.
void
SoOrthoSlice::doAction(SoAction * action)
{
  SoState * state = action->getState();

  // The clipping is common for all action traversal.
  if (this->clipping.getValue()) {
    // Find the plane definition.
    const SoVolumeDataElement * volumedataelement = SoVolumeDataElement::getInstance(state);
    assert(volumedataelement != NULL);

    SbVec3f origo, horizspan, verticalspan;
    volumedataelement->getPageGeometry(this->axis.getValue(),
                                       this->sliceNumber.getValue(),
                                       origo, horizspan, verticalspan);

    SbVec3f planenormal = horizspan.cross(verticalspan);
    if (this->clippingSide.getValue() == SoOrthoSlice::BACK) { planenormal.negate(); }
    const SbPlane sliceplane(planenormal, origo);

    SoClipPlaneElement::add(state, this, sliceplane);
  }
}

void
SoOrthoSlice::GLRender(SoGLRenderAction * action)
{
  SoState * state = action->getState();

  // FIXME: need to make sure we're not cached in a renderlist

  // FIXME: rendering needs to be delayed to get order correctly.

  SbBox3f slicebox;
  SbVec3f dummy;
  this->computeBBox(action, slicebox, dummy);

  // debug
  SoOrthoSliceP::renderBox(action, slicebox);

  // Fetching the current volumedata
  const SoVolumeDataElement * volumedataelement = SoVolumeDataElement::getInstance(state);
  assert(volumedataelement != NULL);
  SoVolumeData * volumedata = volumedataelement->getVolumeData();
  assert(volumedata != NULL);

  const int axisidx = this->axis.getValue();
  const int slicenr = this->sliceNumber.getValue();

  Cvr2DTexPage * texpage = PRIVATE(this)->getPage(axisidx, slicenr, volumedata);

  const SoTransferFunctionElement * tfelement = SoTransferFunctionElement::getInstance(state);
  CvrCLUT * c = new CvrCLUT(*CvrVoxelChunk::getCLUT(tfelement));
  c->setAlphaUse((CvrCLUT::AlphaUse)this->alphaUse.getValue());

  c->ref();
  const CvrCLUT * pageclut = texpage->getPalette();
  if ((pageclut == NULL) || (*pageclut != *c)) { texpage->setPalette(c); }
  c->unref();

  SbVec3f origo, horizspan, verticalspan;
  volumedataelement->getPageGeometry(axisidx, slicenr,
                                     origo, horizspan, verticalspan);

  glPushAttrib(GL_ALL_ATTRIB_BITS);

  glDisable(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // FIXME: ugly cast
  Cvr2DTexSubPage::Interpolation ip =
    (Cvr2DTexSubPage::Interpolation)this->interpolation.getValue();

  texpage->render(action, origo, horizspan, verticalspan, ip);

  glPopAttrib();


  // Common clipping plane handling. Do this after rendering, so the
  // slice itself isn't clipped.
  SoOrthoSlice::doAction(action);
}

Cvr2DTexPage *
SoOrthoSliceP::getPage(const int axis, const int slice, SoVolumeData * voldata)
{
  while (this->cachedpages[axis].getLength() <= slice) {
    this->cachedpages[axis].append(NULL);
  }

  CachedPage * cp = this->cachedpages[axis][slice];

  // Check validity.
  if (cp && (cp->sovolumedata != voldata)) { delete cp; cp = NULL; }

  if (!cp) {
    SbVec3s pagesize = voldata->getPageSize();
    // Pagesize according to axis: X => [Z, Y], Y => [X, Z], Z => [X, Y].
    SbVec2s subpagesize =
      SbVec2s(pagesize[(axis == 0) ? 2 : 0], pagesize[(axis == 1) ? 2 : 1]);

    Cvr2DTexPage * page =
      new Cvr2DTexPage(voldata->getReader(), axis, slice, subpagesize);

    this->cachedpages[axis][slice] = cp = new CachedPage(page, PUBLIC(this));
    cp->sovolumedata = voldata;
  }

  return cp->getPage();
}

void
SoOrthoSlice::rayPick(SoRayPickAction * action)
{
  // FIXME: implement pick
  {
  }

  // Common clipping plane handling.
  SoOrthoSlice::doAction(action);
}

void
SoOrthoSlice::generatePrimitives(SoAction * action)
{
  // FIXME: implement
}

// doc in super
void
SoOrthoSlice::computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center)
{
  SoState * state = action->getState();

  const SoVolumeDataElement * volumedataelement =
    SoVolumeDataElement::getInstance(state);
  if (volumedataelement == NULL) return;
  SoVolumeData * volumedata = volumedataelement->getVolumeData();
  if (volumedata == NULL) return;

  SbBox3f vdbox = volumedata->getVolumeSize();
  SbVec3f bmin, bmax;
  vdbox.getBounds(bmin, bmax);

  SbVec3s dimensions;
  void * data;
  SoVolumeData::DataType type;
  SbBool ok = volumedata->getVolumeData(dimensions, data, type);
  assert(ok);

  const int axisidx = (int)axis.getValue();
  const int slice = this->sliceNumber.getValue();
  const float depth = (float)fabs(bmax[axisidx] - bmin[axisidx]);

  bmin[axisidx] = bmax[axisidx] =
    (bmin[axisidx] + (depth / dimensions[axisidx]) * slice);

  vdbox.setBounds(bmin, bmax);
  box.extendBy(vdbox);
  center = vdbox.getCenter();
}

// *************************************************************************

// Render 3D box.
//
// FIXME: should make bbox debug rendering part of SoSeparator /
// SoGroup. 20030305 mortene.
void
SoOrthoSliceP::renderBox(SoGLRenderAction * action, SbBox3f box)
{
  SbVec3f bmin, bmax;
  box.getBounds(bmin, bmax);

  glPushAttrib(GL_ALL_ATTRIB_BITS);

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);
  glColor4ub(0xff, 0xff, 0xff, 0xff);
  glLineStipple(1, 0xffff);
  glLineWidth(2);

  glBegin(GL_LINES);

  glVertex3f(bmin[0], bmin[1], bmin[2]);
  glVertex3f(bmin[0], bmin[1], bmax[2]);

  glVertex3f(bmax[0], bmin[1], bmin[2]);
  glVertex3f(bmax[0], bmin[1], bmax[2]);

  glVertex3f(bmax[0], bmax[1], bmin[2]);
  glVertex3f(bmax[0], bmax[1], bmax[2]);

  glVertex3f(bmin[0], bmax[1], bmin[2]);
  glVertex3f(bmin[0], bmax[1], bmax[2]);


  glVertex3f(bmin[0], bmin[1], bmin[2]);
  glVertex3f(bmax[0], bmin[1], bmin[2]);

  glVertex3f(bmin[0], bmin[1], bmax[2]);
  glVertex3f(bmax[0], bmin[1], bmax[2]);

  glVertex3f(bmin[0], bmax[1], bmax[2]);
  glVertex3f(bmax[0], bmax[1], bmax[2]);

  glVertex3f(bmin[0], bmax[1], bmin[2]);
  glVertex3f(bmax[0], bmax[1], bmin[2]);


  glVertex3f(bmin[0], bmin[1], bmin[2]);
  glVertex3f(bmin[0], bmax[1], bmin[2]);

  glVertex3f(bmin[0], bmin[1], bmax[2]);
  glVertex3f(bmin[0], bmax[1], bmax[2]);

  glVertex3f(bmax[0], bmin[1], bmax[2]);
  glVertex3f(bmax[0], bmax[1], bmax[2]);

  glVertex3f(bmax[0], bmin[1], bmin[2]);
  glVertex3f(bmax[0], bmax[1], bmin[2]);

  glEnd();

  glPopAttrib();
}

// *************************************************************************
