#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoRayPickAction.h>
#include <Inventor/elements/SoGLClipPlaneElement.h>
#include <Inventor/elements/SoLazyElement.h>
#include <Inventor/elements/SoCacheElement.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/SoPickedPoint.h>
#include <Inventor/system/gl.h>
#include <Inventor/C/tidbits.h>

#include <VolumeViz/nodes/SoOrthoSlice.h>

#include <VolumeViz/details/SoOrthoSliceDetail.h>
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

    if (SoOrthoSliceP::debug == -1) {
      SoOrthoSliceP::debug = coin_getenv("CVR_DEBUG_ORTHOSLICE") ? 1 : 0;
    }
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
  SbPlane getSliceAsPlane(SoAction * action) const;
  SbBool confirmValidInContext(SoState * state) const;

  SoColorPacker colorpacker;

  static int debug;

private:
  SbList<CachedPage*> cachedpages[3];
  SoOrthoSlice * master;
};

int SoOrthoSliceP::debug = -1;

#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

// *************************************************************************

/*!
  \var SoSFEnum SoOrthoSlice::axis

  Decides which plane the orthonormal slice will show. The plane will
  be the one \e perpendicular to the axis, i.e if this field is set to
  SoOrthoSlice::Z, the slice will lay in the X-Y plane.

  Default value is SoOrthoSlice::Z.
*/

/*!
  \var SoSFUInt32 SoOrthoSlice::sliceNumber

  Decides where the slice will be made through the volume.

  Default value is 0.
*/

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

SbPlane
SoOrthoSliceP::getSliceAsPlane(SoAction * action) const
{
  // Find the plane definition.
  const SoVolumeDataElement * volumedataelement =
    SoVolumeDataElement::getInstance(action->getState());
  assert(volumedataelement != NULL);

  // Note that the following code could be made faster by just setting
  // the planenormal to point along the correct axis, and find the
  // slice world coordinate point. This would be simple, as an
  // SoOrthoSlice is guaranteed to be normal to one of the principal
  // axes.
  //
  // Using the SoVolumeDataElement::getPageGeometry() method is rather
  // convenient, though.

  SbVec3f origo, horizspan, verticalspan;
  volumedataelement->getPageGeometry(PUBLIC(this)->axis.getValue(),
                                     PUBLIC(this)->sliceNumber.getValue(),
                                     origo, horizspan, verticalspan);

  SbVec3f planenormal = horizspan.cross(verticalspan);
  if (PUBLIC(this)->clippingSide.getValue() == SoOrthoSlice::BACK) {
    planenormal.negate();
  }

  return SbPlane(planenormal, origo);
}

// Doc from superclass.
void
SoOrthoSlice::doAction(SoAction * action)
{
  // The clipping is common for all action traversal.
  if (this->clipping.getValue()) {
    SoClipPlaneElement::add(action->getState(), this,
                            PRIVATE(this)->getSliceAsPlane(action));
  }
}

// Check whether or not everything is ok and valid for any action
// traversal.
SbBool
SoOrthoSliceP::confirmValidInContext(SoState * state) const
{
  // Fetching the current volumedata
  const SoVolumeDataElement * volumedataelement = SoVolumeDataElement::getInstance(state);
  // FIXME: allow missing SoVolumeData (just don't render, react to
  // picking or set up a non-empty bbox). 20031021 mortene.
  assert(volumedataelement != NULL);
  SoVolumeData * volumedata = volumedataelement->getVolumeData();
  // FIXME: as above. 20031021 mortene.
  assert(volumedata != NULL);

  const int axisidx = PUBLIC(this)->axis.getValue();
  if (axisidx < SoOrthoSlice::X || axisidx > SoOrthoSlice::Z) {
    SoDebugError::post("SoOrthoSliceP::confirmValidInContext",
                       "SoOrthoSlice::axis has invalid value; %d",
                       axisidx);
    return FALSE;
  }

  const int slicenr = PUBLIC(this)->sliceNumber.getValue();
  const short slices = volumedataelement->getVoxelCubeDimensions()[axisidx];
  if (slicenr < 0 || slicenr >= slices) {
    // I don't think this can legally happen, so assert if no slices
    // are available. mortene.
    assert(slices > 0);

    SoDebugError::post("SoOrthoSliceP::confirmValidInContext",
                       "SoOrthoSlice::sliceNumber value %d out of range "
                       "[0, %d]", slicenr, slices - 1);
    return FALSE;
  }

  return TRUE;
}

void
SoOrthoSlice::GLRender(SoGLRenderAction * action)
{
  SoState * state = action->getState();
  if (!PRIVATE(this)->confirmValidInContext(state)) { return; }

  state->push();

  // FIXME: just delaying rendering to a later render pass like below
  // will get rendering of SoOrthoSlice instances correct versus
  // non-transparent geometry in the scene, but several ortho slices
  // overlapping will come out a lot worse than with this
  // disabled. (Because depthbuffer-testing is disabled..?)
  //
  // To get everything to be internally correct, all faces from
  // SimVoleon should be clipped against each other before rendered.
  //
  // 20031030 mortene.
#if 0
  // Use SoShape::shouldGLRender() to automatically handle multi-pass
  // rendering to get transparency correct, plus it should also make
  // bounding box rendering and view frustum culling work as expected.
  //
  // First invoke SoLazyElement::setMaterials(), so we are not faulty
  // getting short-cutted due to a fully transparent current material
  // on the state-stack.

  const SbColor col(1, 1, 1); // dummy value
  // fully opaque, since the textures contain the transparency (and we
  // don't want it to accumulate):
  const float trans = 0.01f;
  SoLazyElement::setMaterials(state, this,
			      SoLazyElement::ALL_COLOR_MASK |
			      SoLazyElement::TRANSPARENCY_MASK,
			      &PRIVATE(this)->colorpacker,
			      &col, 1, &trans, 1,
			      col, col, col, 0.2f, TRUE);
  
  const SbBool shouldrender = this->shouldGLRender(action);
  if (!shouldrender) {
    state->pop();
    return;
  }
#endif

  // Make sure we're not cached in a renderlist.
  SoCacheElement::invalidate(state);

  SbBox3f slicebox;
  SbVec3f dummy;
  this->computeBBox(action, slicebox, dummy);

  // debug
  if (SoOrthoSliceP::debug) { SoOrthoSliceP::renderBox(action, slicebox); }

  // Fetching the current volumedata
  const SoVolumeDataElement * volumedataelement = SoVolumeDataElement::getInstance(state);
  SoVolumeData * volumedata = volumedataelement->getVolumeData();

  const int axisidx = this->axis.getValue();
  const int slicenr = this->sliceNumber.getValue();

  // FIXME: investigate why this seems necessary (at least with the
  // RIMS application). 20031023 mortene.
  const short ydim = volumedataelement->getVoxelCubeDimensions()[Y];
  Cvr2DTexPage * texpage =
    PRIVATE(this)->getPage(axisidx,
                           (axisidx == Y) ? ((ydim - 1) - slicenr) : slicenr,
                           volumedata);

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

  state->pop();
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
  if (!this->shouldRayPick(action)) return;

  SoState * state = action->getState();
  if (!PRIVATE(this)->confirmValidInContext(state)) { return; }

  const SoVolumeDataElement * volumedataelement = SoVolumeDataElement::getInstance(state);
  assert(volumedataelement != NULL);
  const SoVolumeData * volumedata = volumedataelement->getVolumeData();
  if (volumedata == NULL) { return; }


  this->computeObjectSpaceRay(action);
  const SbLine & ray = action->getLine();

  const SbPlane sliceplane = PRIVATE(this)->getSliceAsPlane(action);

  SbVec3f intersection;
  if (sliceplane.intersect(ray, intersection) && // returns FALSE if parallel
      action->isBetweenPlanes(intersection)) {

    SbVec3s ijk = volumedataelement->objectCoordsToIJK(intersection);

    const SbVec3s voxcubedims = volumedataelement->getVoxelCubeDimensions();
    const SbBox3s voxcubebounds(SbVec3s(0, 0, 0), voxcubedims - SbVec3s(1, 1, 1));

    if (voxcubebounds.intersect(ijk)) {

      SoPickedPoint * pp = action->addIntersection(intersection);
      // if NULL, something else is obstructing the view to the
      // volume, the app programmer only want the nearest, and we
      // don't need to continue our intersection tests
      if (pp == NULL) return;
      pp->setObjectNormal(sliceplane.getNormal());

      SoOrthoSliceDetail * detail = new SoOrthoSliceDetail;
      pp->setDetail(detail, this);

      detail->objectcoords = intersection;
      detail->ijkcoords = ijk;
      detail->voxelvalue = volumedata->getVoxelValue(ijk);
    }
  }

  // Common clipping plane handling.
  SoOrthoSlice::doAction(action);
}

void
SoOrthoSlice::generatePrimitives(SoAction * action)
{
  // FIXME: implement

#if CVR_DEBUG && 1 // debug
  static SbBool warn = TRUE;
  if (warn) {
    SoDebugError::postInfo("SoOrthoSlice::generatePrimitives",
                           "not yet implemented");
    warn = FALSE;
  }
#endif // debug
}

// doc in super
void
SoOrthoSlice::computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center)
{
  SoState * state = action->getState();
  if (!PRIVATE(this)->confirmValidInContext(state)) { return; }

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
