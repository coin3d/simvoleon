/**************************************************************************\
 *
 *  This file is part of the SIM Voleon visualization library.
 *  Copyright (C) 2003-2004 by Systems in Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  ("GPL") version 2 as published by the Free Software Foundation.
 *  See the file LICENSE.GPL at the root directory of this source
 *  distribution for additional information about the GNU GPL.
 *
 *  For using SIM Voleon with software that can not be combined with
 *  the GNU GPL, and for taking advantage of the additional benefits
 *  of our support services, please contact Systems in Motion about
 *  acquiring a SIM Voleon Professional Edition License.
 *
 *  See <URL:http://www.coin3d.org/> for more information.
 *
 *  Systems in Motion, Teknobyen, Abels Gate 5, 7030 Trondheim, NORWAY.
 *  <URL:http://www.sim.no/>.
 *
\**************************************************************************/

/*!
  \class SoOrthoSlice VolumeViz/nodes/SoOrthoSlice.h
  \brief Render one orthogonal slice of the volume data.

  Insert a node of this type after an SoVolumeData node in the scene
  graph to render a single, axis-aligned slice from the full volume
  data set.

  \sa SoVolumeRender
*/

// *************************************************************************

#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoRayPickAction.h>
#include <Inventor/elements/SoGLClipPlaneElement.h>
#include <Inventor/elements/SoLazyElement.h>
#include <Inventor/elements/SoCacheElement.h>
#include <Inventor/elements/SoGLLazyElement.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/SoPickedPoint.h>
#include <Inventor/system/gl.h>
#include <Inventor/C/tidbits.h>
#include <Inventor/SbRotation.h>

#include <VolumeViz/nodes/SoOrthoSlice.h>

#include <VolumeViz/details/SoOrthoSliceDetail.h>
#include <VolumeViz/elements/CvrGLInterpolationElement.h>
#include <VolumeViz/elements/CvrPageSizeElement.h>
#include <VolumeViz/elements/CvrVoxelBlockElement.h>
#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/nodes/SoTransferFunction.h>
#include <VolumeViz/render/2D/Cvr2DTexPage.h>
#include <VolumeViz/misc/CvrCLUT.h>
#include <VolumeViz/misc/CvrVoxelChunk.h>
#include <VolumeViz/misc/CvrUtil.h>

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

  uint32_t volumedataid;
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
  Cvr2DTexPage * getPage(const SoGLRenderAction * action, const int axis, const int slice);
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

  SO_ENABLE(SoGLRenderAction, SoTransferFunctionElement);
  SO_ENABLE(SoGLRenderAction, SoGLClipPlaneElement);
  SO_ENABLE(SoGLRenderAction, SoModelMatrixElement);
  SO_ENABLE(SoGLRenderAction, SoLazyElement);

  SO_ENABLE(SoPickAction, SoTransferFunctionElement);
  SO_ENABLE(SoPickAction, SoClipPlaneElement);

  SO_ENABLE(SoGLRenderAction, CvrGLInterpolationElement);
}

// doc in super
SbBool
SoOrthoSlice::affectsState(void) const
{
  return this->clipping.getValue();
}

// Find the plane definition.
SbPlane
SoOrthoSliceP::getSliceAsPlane(SoAction * action) const
{
  const CvrVoxelBlockElement * vbelem =
    CvrVoxelBlockElement::getInstance(action->getState());

  const int axis = PUBLIC(this)->axis.getValue();

  // Finding the plane normal is straight forward -- it's the same as
  // the SoOrthoSlice axis:

  SbVec3f planenormal(axis == SoOrthoSlice::X ? 1 : 0,
                      axis == SoOrthoSlice::Y ? 1 : 0,
                      axis == SoOrthoSlice::Z ? 1 : 0);

  if (PUBLIC(this)->clippingSide.getValue() == SoOrthoSlice::FRONT) {
    planenormal.negate();
  }

  // Finding a point in the plane:

  const SbBox3f & spacesize = vbelem->getUnitDimensionsBox();
  SbVec3f spacemin, spacemax;
  spacesize.getBounds(spacemin, spacemax);

  SbVec3f origo = spacesize.getCenter();

  const SbVec3s & dimensions = vbelem->getVoxelCubeDimensions();
  const float depthprslice = (spacemax[axis] - spacemin[axis]) / dimensions[axis];
  const float depth = spacemin[axis] + PUBLIC(this)->sliceNumber.getValue() * depthprslice;
  origo[axis] = depth;

  // Return local unit coordinate system plane:

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
  const CvrVoxelBlockElement * vbelem = CvrVoxelBlockElement::getInstance(state);
  assert(vbelem != NULL);

  const int axisidx = PUBLIC(this)->axis.getValue();
  if (axisidx < SoOrthoSlice::X || axisidx > SoOrthoSlice::Z) {
    SoDebugError::post("SoOrthoSliceP::confirmValidInContext",
                       "SoOrthoSlice::axis has invalid value; %d",
                       axisidx);
    return FALSE;
  }

  const int slicenr = PUBLIC(this)->sliceNumber.getValue();
  const short slices = vbelem->getVoxelCubeDimensions()[axisidx];
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

  // Fetching the current volumedata information.
  const CvrVoxelBlockElement * vbelem = CvrVoxelBlockElement::getInstance(state);
  // FIXME: should have a warning message upon missing
  // SoVolumeData. 20040719 mortene.
  if (vbelem == NULL) { return; }

  if (!PRIVATE(this)->confirmValidInContext(state)) { return; }

  // Render at the end, in case the volume is partly (or fully)
  // transparent.
  //
  // FIXME: this makes rendering a bit slower, so we should perhaps
  // keep a flag around to know whether or not this is actually
  // necessary. 20040212 mortene.
  if (!action->isRenderingDelayedPaths()) {
    action->addDelayedPath(action->getCurPath()->copy());

    // The clip plane should still affect subsequent geometry in
    // non-delayed traversal:
    SoOrthoSlice::doAction(action);

    return;
  }

  state->push();

  // FIXME: just delaying rendering to a later render pass like below
  // will get rendering of SoOrthoSlice instances correct versus
  // non-transparent geometry in the scene, but several ortho slices
  // overlapping will come out a lot worse than with this
  // disabled. (Because depthbuffer-testing is disabled..?)
  //
  // To get everything to be internally correct, all faces from
  // SIMVoleon should be clipped against each other before rendered.
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

  // Extract volume placement and scale information, and place it on
  // the model matrix stack. This lets the subsequent render code work
  // with a 1x1x1 size volume in unit coordinates, without
  // e.g. bothering about any scale and/or translation embedded in the
  // SoVolumeData::volumeboxmin/max fields.
  {
    const SbBox3f & localbox = vbelem->getUnitDimensionsBox();
    const SbVec3f
      localspan((localbox.getMax()[0] - localbox.getMin()[0]),
                (localbox.getMax()[1] - localbox.getMin()[1]),
                (localbox.getMax()[2] - localbox.getMin()[2]));

    const SbVec3f localtrans =
      (localbox.getMax() - localbox.getMin()) / 2.0f + localbox.getMin();

    SbMatrix m;
    m.setTransform(localtrans, SbRotation::identity(), localspan);
    SoModelMatrixElement::mult(state, this, m);
  }


  const int axisidx = this->axis.getValue();
  const int slicenr = this->sliceNumber.getValue();

  int pageslice = slicenr;

  // This is done to support client code depending on an old bug: data
  // along the Y axis used to be rendered flipped.
  if (CvrUtil::useFlippedYAxis() && (axisidx == Y)) {
    const short ydim = vbelem->getVoxelCubeDimensions()[Y];
    pageslice = (ydim - 1) - pageslice;
  }

  Cvr2DTexPage * texpage =
    PRIVATE(this)->getPage(action, axisidx, pageslice);

  const SoTransferFunctionElement * tfelement = SoTransferFunctionElement::getInstance(state);
  CvrCLUT * c = new CvrCLUT(*CvrVoxelChunk::getCLUT(tfelement));
  c->setAlphaUse((CvrCLUT::AlphaUse)this->alphaUse.getValue());

  c->ref();
  const CvrCLUT * pageclut = texpage->getPalette();
  if ((pageclut == NULL) || (*pageclut != *c)) { texpage->setPalette(c); }
  c->unref();

  // This must be done, as we want to control stuff in the GL state
  // machine. Without it, state changes could trigger outside our
  // control.
  SoGLLazyElement::getInstance(state)->send(state, SoLazyElement::ALL_MASK);

  // FIXME: we do state->push() now, so this isn't really necessary,
  // is it? 20040223 mortene.
  glPushAttrib(GL_ALL_ATTRIB_BITS);

  glDisable(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D);
#if 0 // disabled this, so SoDrawStyle influences SoOrthoSlice, as expected
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  GLenum interp;
  switch (this->interpolation.getValue()) {
  case NEAREST: interp = GL_NEAREST; break;
  case LINEAR: interp = GL_LINEAR; break;
  default: assert(FALSE && "invalid value in interpolation field"); break;
  }
  CvrGLInterpolationElement::set(state, interp);

  SbVec3f origo, horizspan, verticalspan;
  vbelem->getPageGeometry(axisidx, slicenr, origo, horizspan, verticalspan);

  texpage->render(action, origo, horizspan, verticalspan);

  // Even though we do state->pop(), we must also push/pop low-level
  // GL.
  glPopAttrib();

  state->pop();

  // Common clipping plane handling. Do this after rendering, so the
  // slice itself isn't clipped, and outside the state push/pop, so
  // the clip plane will affect subsequent traversal.
  SoOrthoSlice::doAction(action);
}

Cvr2DTexPage *
SoOrthoSliceP::getPage(const SoGLRenderAction * action,
                       const int axis, const int slice)
{
  while (this->cachedpages[axis].getLength() <= slice) {
    this->cachedpages[axis].append(NULL);
  }

  SoState * state = action->getState();
  const CvrVoxelBlockElement * vbelem = CvrVoxelBlockElement::getInstance(state);

  CachedPage * cp = this->cachedpages[axis][slice];

  // Check validity.
  if (cp && (cp->volumedataid != vbelem->getNodeId())) { delete cp; cp = NULL; }

  if (!cp) {
    const SbVec3s & pagesize = CvrPageSizeElement::get(state);
    // Pagesize according to axis: X => [Z, Y], Y => [X, Z], Z => [X, Y].
    SbVec2s subpagesize =
      SbVec2s(pagesize[(axis == 0) ? 2 : 0], pagesize[(axis == 1) ? 2 : 1]);

    Cvr2DTexPage * page = new Cvr2DTexPage(action, axis, slice, subpagesize);

    this->cachedpages[axis][slice] = cp = new CachedPage(page, PUBLIC(this));
    cp->volumedataid = vbelem->getNodeId();
  }

  return cp->getPage();
}

void
SoOrthoSlice::rayPick(SoRayPickAction * action)
{
  if (!this->shouldRayPick(action)) return;

  SoState * state = action->getState();
  if (!PRIVATE(this)->confirmValidInContext(state)) { return; }

  const CvrVoxelBlockElement * vbelem = CvrVoxelBlockElement::getInstance(state);
  if (vbelem == NULL) { return; }

  this->computeObjectSpaceRay(action);
  const SbLine & ray = action->getLine();

  const SbPlane sliceplane = PRIVATE(this)->getSliceAsPlane(action);

  SbVec3f intersection;
  if (sliceplane.intersect(ray, intersection) && // returns FALSE if parallel
      action->isBetweenPlanes(intersection)) {

    SbVec3s ijk = vbelem->objectCoordsToIJK(intersection);

    const SbVec3s & voxcubedims = vbelem->getVoxelCubeDimensions();
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
      detail->voxelvalue = vbelem->getVoxelValue(ijk);

      if (CvrUtil::useFlippedYAxis()) {
        static SbBool flag = FALSE;
        if (!flag) {
          SoDebugError::postWarning("SoOrthoSlice::rayPick", 
                                    "RayPick'ing will not be correct for SoOrthoSlice when the "
                                    "obsolete CVR_USE_FLIPPED_Y_AXIS flag is active.");
          flag = TRUE;
        }
      }

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

  const CvrVoxelBlockElement * vbelem = CvrVoxelBlockElement::getInstance(state);
  if (vbelem == NULL) return;

  SbBox3f vdbox = vbelem->getUnitDimensionsBox();
  SbVec3f bmin, bmax;
  vdbox.getBounds(bmin, bmax);

  const SbVec3s & dimensions = vbelem->getVoxelCubeDimensions();

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
