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

  this->subpagesize[0] = 64;
  this->subpagesize[1] = 64;
  this->subpagesize[2] = 64;

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

void
CvrPageHandler::render(SoGLRenderAction * action, int numslices)
{
  SoState * state = action->getState();

  const SoVolumeDataElement * volumedataelement = SoVolumeDataElement::getInstance(state);
  assert(volumedataelement != NULL);
  SoVolumeData * volumedata = volumedataelement->getVolumeData();
  assert(volumedata != NULL);

  SbVec3f volmin, volmax;
  SbBox3f volumeSize = volumedata->getVolumeSize();
  volumeSize.getBounds(volmin, volmax);

#if CVR_DEBUG && 0 // debug
  SoDebugError::postInfo("SoVolumeRender::GLRender",
                         "volumeSize==[%f, %f, %f]",
                         volmax[0] - volmin[0],
                         volmax[1] - volmin[1],
                         volmax[2] - volmin[2]);
#endif // debug

  SbVec3f camvec;
  this->getViewVector(action, camvec);
  const int AXISIDX = this->getCurrentAxis(camvec);

  float depth;
  float depthAdder;

  // Render in reverse order?
  if (camvec[AXISIDX] < 0)  {
    depthAdder = -(volmax[AXISIDX] - volmin[AXISIDX]) / numslices;
    depth = volmax[AXISIDX];
  }
  else {
    depthAdder = (volmax[AXISIDX] - volmin[AXISIDX]) / numslices;
    depth = volmin[AXISIDX];
  }

  const SbBox2f QUAD = (AXISIDX == 2) ? // along Z?
    SbBox2f(volmin[0], volmin[1], volmax[0], volmax[1]) :
    ((AXISIDX ==  0) ? // along X?
     SbBox2f(volmin[2], volmin[1], volmax[2], volmax[1]) :
     // along Y
     SbBox2f(volmin[0], volmin[2], volmax[0], volmax[2]));

#if CVR_DEBUG && 0 // debug
  SoDebugError::postInfo("SoVolumeRender::GLRender",
                         "QUAD=[%f, %f] - [%f, %f] (AXISIDX==%d)",
                         QUAD.getMin()[0], QUAD.getMin()[1],
                         QUAD.getMax()[0], QUAD.getMax()[1],
                         AXISIDX);
#endif // debug

  glPushAttrib(GL_ALL_ATTRIB_BITS);

  glDisable(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

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

  glDisable(GL_CULL_FACE);

  for (int i = 0; i < numslices; i++) {
    // Find nearest integer page idx (as number of pages to render
    // need not match the number of actual volume data pages).
    int pageidx = (int)
      ((float(i)/float(numslices)) * float(this->voldatadims[AXISIDX]) + 0.5f);

    assert(pageidx >= 0);
    assert(pageidx < numslices);

    // If rendering in reverse order.
    if (depthAdder < 0) { pageidx = numslices - pageidx - 1; }

    this->renderOnePage(action, QUAD, depth, pageidx, AXISIDX);
    depth += depthAdder;
  }

  glPopAttrib();
}

void
CvrPageHandler::renderOnePage(SoGLRenderAction * action,
                              const SbBox2f & quad, float depth,
                              unsigned int pageidx, unsigned int axis)
{
  assert(pageidx < this->voldatadims[axis]);

  SbVec2f qmax, qmin;
  quad.getBounds(qmin, qmax);

  SbVec3f origo, horizspan, verticalspan;
  const float width = qmax[0] - qmin[0];
  const float height = qmax[1] - qmin[1];

  if (axis == 0) {
    origo = SbVec3f(depth, qmin[1], qmin[0]);
    horizspan = SbVec3f(0, 0, width);
    verticalspan = SbVec3f(0, height, 0);
  }
  else if (axis == 1) {
    // The last component is "flipped" to make the y-direction slices
    // not come out upside-down. FIXME: should really investigate if
    // this is the correct fix. 20021124 mortene.
    origo = SbVec3f(qmin[0], depth, qmax[1]);
    horizspan = SbVec3f(width, 0, 0);
    verticalspan = SbVec3f(0, 0, -height);
  }
  else if (axis == 2) {
    origo = SbVec3f(qmin[0], qmin[1], depth);
    horizspan = SbVec3f(width, 0, 0);
    verticalspan = SbVec3f(0, height, 0);
  }
  else assert(FALSE);

  Cvr2DTexPage * slice = this->getSlice(axis, pageidx);

#if CVR_DEBUG && 0 // debug
  SoDebugError::postInfo("CvrPageHandler::renderOnePage",
                         "origo==[%f, %f, %f]",
                         origo[0], origo[1], origo[2]);
#endif // debug

  slice->render(action, origo, horizspan, verticalspan,
                0 /*FIXME: PRIVATE(this)->tick*/);
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

#if 0 // TMP OBSOLETE

void
SoVolumeDataP::managePages(void)
{
  // FIXME: this functionality should really be part of a global
  // manager for the 2D texture pages (and for 3D textures later).
  // 20021120 mortene.

  // Keep both measures within maxlimits

  while (Cvr2DTexSubPage::totalTextureMemoryUsed() > this->maxtexmem) {
    this->releaseLRUPage();
  }

  while (Cvr2DTexSubPage::totalNrOfTexels() > this->maxnrtexels) {
    this->releaseLRUPage();
  }
}

void
SoVolumeDataP::releaseLRUPage(void)
{
  Cvr2DTexSubPage * lru_subpage = NULL;
  Cvr2DTexPage * lru_page = NULL;
  long lowesttick = LONG_MAX;

  // Searching for least recently used page.

  // FIXME: should really be stored in a heap data structure. 20021120 mortene.
  //
  // FIXME: update, just a double-linked list will do the trick --
  // just link a page used and move it to front. 20021124 mortene.

  for (int dim=0; dim < 3; dim++) {
    if (this->slices[dim]) {
      for (int i = 0; i < this->voldatadims[dim]; i++) {
        Cvr2DTexPage * page = this->slices[dim][i];
        if (page) {
          long tickval;
          Cvr2DTexSubPage * subpage = page->getLRUSubPage(tickval);
          SbBool newlow = (lru_subpage == NULL);
          if (!newlow) { newlow = subpage && (lowesttick > tickval); }
          if (newlow) {
            lru_page = page;
            lru_subpage = subpage;
            lowesttick = tickval;
          }
        }
      }
    }
  }

  lru_page->releaseSubPage(lru_subpage);
}

#endif // TMP OBSOLETE
