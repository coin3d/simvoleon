#include <VolumeViz/render/2D/CvrPageHandler.h>

#include <VolumeViz/elements/SoVolumeDataElement.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/readers/SoVolumeReader.h>
#include <VolumeViz/render/2D/Cvr2DTexPage.h>

#include <Inventor/C/tidbits.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>

#include <stdlib.h>
#include <assert.h>

CvrPageHandler::CvrPageHandler(const SbVec3s & voldatadims,
                               SoVolumeReader * reader)
{
  this->voldatadims[0] = voldatadims[0];
  this->voldatadims[1] = voldatadims[1];
  this->voldatadims[2] = voldatadims[2];

  this->subpagesize = SbVec3s(64, 64, 64);

  this->slices[0] = NULL;
  this->slices[1] = NULL;
  this->slices[2] = NULL;

  this->reader = reader;
}

CvrPageHandler::~CvrPageHandler()
{
  this->releaseAllSlices();
}

// Calculates direction from camera to center of object.
void
CvrPageHandler::getViewVector(SoGLRenderAction * action, SbVec3f & direction) const
{
  SoState * state = action->getState();

  const SbMatrix & mm = SoModelMatrixElement::get(state);
  SbMatrix imm = mm.inverse();

  const SbViewVolume & vv = SoViewVolumeElement::get(state);

  if (/* FIXME: ??? 20021111 mortene: */ 0 &&
      vv.getProjectionType() == SbViewVolume::PERSPECTIVE) {
    SbVec3f worldpos(0.0f, 0.0f, 0.0f);
    mm.multVecMatrix(worldpos, worldpos);
    direction = vv.getProjectionPoint() - worldpos;
    imm.multDirMatrix(direction, direction);
  }
  else { // orthographic
    direction = - vv.getProjectionDirection();
    imm.multDirMatrix(direction, direction);
  }
}

unsigned int
CvrPageHandler::getCurrentAxis(const SbVec3f & viewvec) const
{
  // This is a debugging backdoor: if the environment variable
  // CVR_LOCK_AXIS is set, we'll return the axis value (0 for X, 1 for
  // Y and 2 for Z) it sets, no matter what.
  static int lockaxis = -1;
  if (lockaxis == -1) {
    const char * envstr = coin_getenv("CVR_LOCK_AXIS");
    if (envstr) {
      lockaxis = atoi(envstr);
      assert(lockaxis >= 0 && lockaxis <= 2);
    }
    else lockaxis = -2;
  }
  if (lockaxis != -2) return lockaxis;


  SbVec3f abstoviewer;
  abstoviewer[0] = fabs(viewvec[0]);
  abstoviewer[1] = fabs(viewvec[1]);
  abstoviewer[2] = fabs(viewvec[2]);

  // Figures out which axis we are closest to be looking along.

  SbBool renderalongX =
    (abstoviewer[0] >= abstoviewer[1]) &&
    (abstoviewer[0] >= abstoviewer[2]);

  SbBool renderalongY =
    (abstoviewer[1] >= abstoviewer[0]) &&
    (abstoviewer[1] >= abstoviewer[2]);

  SbBool renderalongZ =
    (abstoviewer[2] >= abstoviewer[0]) &&
    (abstoviewer[2] >= abstoviewer[1]);

  assert(((renderalongX ? 1 : 0) +
          (renderalongY ? 1 : 0) +
          (renderalongZ ? 1 : 0)) == 1);

  return (renderalongX ? 0 : (renderalongY ? 1 : 2));
}

unsigned int
CvrPageHandler::getCurrentAxis(SoGLRenderAction * action) const
{
  SbVec3f camvec;
  this->getViewVector(action, camvec);
  return this->getCurrentAxis(camvec);
}

// Compare with this->subpagesize. If they differ, destroy all pages
// along the changed axes. They will be automatically regenerated upon
// next rendering.
void
CvrPageHandler::comparePageSize(const SbVec3s & currsubpagesize)
{
  SbBool rebuildx = FALSE, rebuildy = FALSE, rebuildz = FALSE;

  if (currsubpagesize[0] != this->subpagesize[0]) rebuildy = rebuildz = TRUE;
  if (currsubpagesize[1] != this->subpagesize[1]) rebuildx = rebuildz = TRUE;
  if (currsubpagesize[2] != this->subpagesize[2]) rebuildx = rebuildy = TRUE;

  if (rebuildx || rebuildy || rebuildz) {
    this->subpagesize = currsubpagesize;

    // This is the quick and simple way to do it.
    if (rebuildx) this->releaseSlices(0);
    if (rebuildy) this->releaseSlices(1);
    if (rebuildz) this->releaseSlices(2);
    // A more optimal strategy when it comes to performance /might/ be
    // to just rearrange the textures within existing pages. This is
    // not so simple, though, as the textures are not stored anywhere
    // else but within OpenGL texture objects -- which means we would
    // have to use ~ 2x the memory to avoid simple destruction and
    // regeneration, as we do now.
    //
    // mortene.
  }
}

void
CvrPageHandler::render(SoGLRenderAction * action, unsigned int numslices)
{
  SoState * state = action->getState();

  const SoVolumeDataElement * volumedataelement = SoVolumeDataElement::getInstance(state);
  assert(volumedataelement != NULL);
  SoVolumeData * volumedata = volumedataelement->getVolumeData();
  assert(volumedata != NULL);

  this->comparePageSize(volumedata->getPageSize());

  SbVec3f spacemin, spacemax;
  SbBox3f spacesize = volumedata->getVolumeSize();
  spacesize.getBounds(spacemin, spacemax);

  const SbVec3f SCALE((spacemax[0] - spacemin[0]) / this->voldatadims[0],
                      (spacemax[1] - spacemin[1]) / this->voldatadims[1],
                      (spacemax[2] - spacemin[2]) / this->voldatadims[2]);

  SbVec3f camvec;
  this->getViewVector(action, camvec);
  const int AXISIDX = this->getCurrentAxis(camvec);

#if CVR_DEBUG && 0 // debug
  SoDebugError::postInfo("CvrPageHandler::render",
                         "spacesize==[%f, %f, %f] [%f, %f, %f], "
                         "this->voldatadims==[%d, %d, %d], "
                         "SCALE=[%f, %f, %f], "
                         "axis==%c, numslices==%d",
                         spacemin[0], spacemin[1], spacemin[2],
                         spacemax[0], spacemax[1], spacemax[2],
                         this->voldatadims[0], this->voldatadims[1], this->voldatadims[2],
                         SCALE[0], SCALE[1], SCALE[2],
                         AXISIDX == 0 ? 'X' : (AXISIDX == 1 ? 'Y' : 'Z'),
                         numslices);
#endif // debug

  float depth = spacemin[AXISIDX];
  float depthprslice = (spacemax[AXISIDX] - spacemin[AXISIDX]) / numslices;

  // Render in reverse order?
  if (camvec[AXISIDX] < 0)  {
    depth = spacemax[AXISIDX];
    depthprslice = -depthprslice;
  }

  const SbBox2f QUAD = (AXISIDX == 2) ? // along Z?
    SbBox2f(spacemin[0], spacemin[1], spacemax[0], spacemax[1]) :
    ((AXISIDX ==  0) ? // along X?
     SbBox2f(spacemin[2], spacemin[1], spacemax[2], spacemax[1]) :
     // along Y
     SbBox2f(spacemin[0], spacemin[2], spacemax[0], spacemax[2]));

  const SbVec2f QUADSCALE = (AXISIDX == 2) ?
    SbVec2f(SCALE[0], SCALE[1]) :
    ((AXISIDX == 0) ?
     SbVec2f(SCALE[2], SCALE[1]) :
     SbVec2f(SCALE[0], SCALE[2]));

#if CVR_DEBUG && 0 // debug
  SoDebugError::postInfo("CvrPageHandler::render",
                         "QUAD=[%f, %f] - [%f, %f] (AXISIDX==%d)",
                         QUAD.getMin()[0], QUAD.getMin()[1],
                         QUAD.getMax()[0], QUAD.getMax()[1],
                         AXISIDX);
#endif // debug

  SbVec2f qmax, qmin;
  QUAD.getBounds(qmin, qmax);

  SbVec3f origo, horizspan, verticalspan;
  const float width = qmax[0] - qmin[0];
  const float height = qmax[1] - qmin[1];


  // "origo" should point at the upper left corner of the page to
  // render. horizspan should point rightwards, and verticalspan
  // downwards.

  if (AXISIDX == 0) {
    origo = SbVec3f(depth, qmax[1], qmin[0]);
    horizspan = SbVec3f(0, 0, width);
    verticalspan = SbVec3f(0, -height, 0);
  }
  else if (AXISIDX == 1) {
    // The last component is "flipped" to make the y-direction slices
    // not come out upside-down. FIXME: should really investigate if
    // this is the correct fix. 20021124 mortene.
    origo = SbVec3f(qmin[0], depth, qmin[1]);
    horizspan = SbVec3f(width, 0, 0);
    verticalspan = SbVec3f(0, 0, height);
  }
  else if (AXISIDX == 2) {
    origo = SbVec3f(qmin[0], qmax[1], depth);
    horizspan = SbVec3f(width, 0, 0);
    verticalspan = SbVec3f(0, -height, 0);
  }
  else assert(FALSE);


#if CVR_DEBUG && 0 // debug
  SoDebugError::postInfo("CvrPageHandler::render",
                         "origo==[%f, %f, %f]  "
                         "horizspan=[%f, %f, %f] verticalspan=[%f, %f, %f]",
                         origo[0], origo[1], origo[2],
                         horizspan[0], horizspan[1], horizspan[2],
                         verticalspan[0], verticalspan[1], verticalspan[2]);
#endif // debug

  glPushAttrib(GL_ALL_ATTRIB_BITS);

  glDisable(GL_LIGHTING);
  // FIXME: shouldn't we disable lots of stuff here (3D textures, for
  // instance)? 20021128 mortene.
  glEnable(GL_TEXTURE_2D);
  glPolygonMode(GL_FRONT, GL_FILL);

  // FIXME: this is a reversion of a change that pederb made on
  // 20021104 that made the CoinVol/testcode/example program fail
  // (nothing gets drawn). Need to check with pederb what he tried to
  // accomplish with the change. (The log message says "Switched to
  // alpha test rendering instead of blending.") 20021109 mortene.
#if 1
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#else
  // FIXME: add proper transparency test. For now I've just disabled
  // blending and enabled glAlphaTest instead (looks better, and delayed
  // rendering is not required). pederb, 2002-11-04
  // glEnable(GL_BLEND);
  // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // this is to enable alpha test
  glAlphaFunc(GL_GREATER, 0.5f);
  glEnable(GL_ALPHA_TEST);
#endif

  // FIXME: what's this good for? 20021128 mortene.
  glDisable(GL_CULL_FACE);

  for (unsigned int i = 0; i < numslices; i++) {
    // Find nearest integer page idx (as number of pages to render
    // need not match the number of actual volume data pages).
    unsigned int pageidx = (unsigned int)
      ((float(i)/float(numslices)) * float(this->voldatadims[AXISIDX]) + 0.5f);
    // If rendering in reverse order.
    if (depthprslice < 0) { pageidx = numslices - pageidx - 1; }
    // Pages along Y-axis is in opposite order of those along X- and
    // Z-axis.
    if (AXISIDX == 1)  { pageidx = numslices - pageidx - 1; }

    assert(pageidx < numslices);
    assert(pageidx < this->voldatadims[AXISIDX]);

    // Note: even if this is the same page as the last one (numSlices
    // in SoVolumeRender can be larger than the actual dimensions), we
    // should still render it at the new depth, as that can give
    // better rendering quality of the volume.
    Cvr2DTexPage * page = this->getSlice(AXISIDX, pageidx);
    origo[AXISIDX] = depth;
    page->render(action, origo, horizspan, verticalspan, QUADSCALE);

    depth += depthprslice;
  }

  glPopAttrib();
}

Cvr2DTexPage *
CvrPageHandler::getSlice(const unsigned int AXISIDX, unsigned int sliceidx)
{
  assert(AXISIDX <= 2);
  assert(sliceidx < this->voldatadims[AXISIDX]);

#if 0 // debug
  SoDebugError::postInfo("CvrPageHandler::getSlice", "axis==%c sliceidx==%d",
                         AXISIDX == 0 ? 'X' : (AXISIDX == 1 ? 'Y' : 'Z'),
                         sliceidx);
#endif // debug

  // First Cvr2DTexSubPage ever for this axis?
  if (this->slices[AXISIDX] == NULL) {
    this->slices[AXISIDX] = new Cvr2DTexPage*[this->voldatadims[AXISIDX]];
    for (unsigned int i=0; i < this->voldatadims[AXISIDX]; i++) {
      this->slices[AXISIDX][i] = NULL;
    }
  }

  if (this->slices[AXISIDX][sliceidx] == NULL) {
    Cvr2DTexPage * newslice = new Cvr2DTexPage;

    SbVec2s pagesize =
      AXISIDX == 0 ?
      SbVec2s(this->subpagesize[2], this->subpagesize[1]) :
      (AXISIDX == 1 ?
       SbVec2s(this->subpagesize[0], this->subpagesize[2]) :
       SbVec2s(this->subpagesize[0], this->subpagesize[1]));

    assert(coin_is_power_of_two(this->subpagesize[0]));
    assert(coin_is_power_of_two(this->subpagesize[1]));
    assert(coin_is_power_of_two(this->subpagesize[2]));

    newslice->init(this->reader, sliceidx, AXISIDX, pagesize);

    this->slices[AXISIDX][sliceidx] = newslice;
  }

  return this->slices[AXISIDX][sliceidx];
}

void
CvrPageHandler::releaseAllSlices(void)
{
  for (unsigned int i = 0; i < 3; i++) { this->releaseSlices(i); }
}

void
CvrPageHandler::releaseSlices(const unsigned int AXISIDX)
{
  if (this->slices[AXISIDX] == NULL) return;

  for (unsigned int i = 0; i < this->voldatadims[AXISIDX]; i++) {
    delete this->slices[AXISIDX][i];
    this->slices[AXISIDX][i] = NULL;
  }

  delete[] this->slices[AXISIDX];
}
