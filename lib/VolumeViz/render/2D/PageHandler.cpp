#include <VolumeViz/render/2D/CvrPageHandler.h>
#include <VolumeViz/render/2D/Cvr2DTexPage.h>
#include <VolumeViz/readers/SoVolumeReader.h>
#include <Inventor/C/tidbits.h>
#include <Inventor/errors/SoDebugError.h>
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

void
CvrPageHandler::renderOrthoSlice(SoState * state,
                                 const SbBox2f & quad,
                                 float depth,
                                 int sliceIdx,
                                 // axis: 0, 1, 2 for X, Y or Z axis.
                                 unsigned int axis)
{
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
    origo = SbVec3f(qmin[0], depth, qmin[1]);
    horizspan = SbVec3f(width, 0, 0);
    verticalspan = SbVec3f(0, 0, height);
  }
  else if (axis == 2) {
    origo = SbVec3f(qmin[0], qmin[1], depth);
    horizspan = SbVec3f(width, 0, 0);
    verticalspan = SbVec3f(0, height, 0);
  }
  else assert(FALSE);

  Cvr2DTexPage * slice = this->getSlice(axis, sliceIdx);

#if CVR_DEBUG && 0 // debug
  SoDebugError::postInfo("CvrPageHandler::renderOrthoSlice",
                         "origo==[%f, %f, %f]",
                         origo[0], origo[1], origo[2]);
#endif // debug

  slice->render(state, origo, horizspan, verticalspan,
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
