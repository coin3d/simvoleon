#include <VolumeViz/misc/SoVolumeDataSlice.h>
#include <Inventor/misc/SoState.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <string.h>

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif // HAVE_WINDOWS_H
#include <GL/gl.h>


SoVolumeDataSlice::SoVolumeDataSlice(void)
{
  this->pageSize = SbVec2s(32, 32);
  this->pages = NULL;
  this->axis = SoOrthoSlice::Z;
  this->numPages = 0;
  this->numTexels = 0;
  this->sliceIdx = 0;
  this->reader = NULL;
  this->dataType = SoVolumeData::RGBA;
}


SoVolumeDataSlice::~SoVolumeDataSlice()
{
  this->reader = NULL;
  this->releaseAllPages();
}


void SoVolumeDataSlice::init(SoVolumeReader * reader, int sliceIdx,
                             SoOrthoSlice::Axis axis,
                             const SbVec2s & pageSize)
{
  this->releaseAllPages();

  this->reader = reader;
  this->sliceIdx = sliceIdx;
  this->axis = axis;
  this->pageSize = pageSize;

  SbVec3s dim;
  SbBox3f size;
  reader->getDataChar(size, this->dataType, dim);

  switch (axis) {
    case SoOrthoSlice::X:
      this->dimensions[0] = dim[2];
      this->dimensions[1] = dim[1];
      break;

    case SoOrthoSlice::Y:
      this->dimensions[0] = dim[0];
      this->dimensions[1] = dim[2];
      break;

    case SoOrthoSlice::Z:
      this->dimensions[0] = dim[0];
      this->dimensions[1] = dim[1];
      break;
  }

  this->numCols = this->dimensions[0] / this->pageSize[0];
  this->numRows = this->dimensions[1] / this->pageSize[1];
}



/*!
  Searches for a specified page in the slice. The page is identified
  by it's memorylocation (the pointer). If the page isn't found,
  nothing happens.
*/
void
SoVolumeDataSlice::releasePage(SoVolumeDataPage * page)
{
  if (this->pages) {
    for (int i = 0; i < numCols*numRows; i++) {
      SoVolumeDataPage * p = this->pages[i];

      if (p != NULL) {
        // Delete the first page in the linked list
        if (p == page) {
          this->numPages--;
          this->numTexels -= this->pageSize[0] * this->pageSize[1];
          this->numBytesSW -= p->numBytesSW;
          this->numBytesHW -= p->numBytesHW;

          this->pages[i] = p->nextPage;
          p->nextPage = NULL;
          delete p;

          return;
        }
        else {
          while (p->nextPage != NULL) {
            if (p->nextPage == page) {
              this->numPages--;
              this->numTexels -= this->pageSize[0] * this->pageSize[1];
              this->numBytesSW -= p->numBytesSW;
              this->numBytesHW -= p->numBytesHW;

              p->nextPage = p->nextPage->nextPage;
              page->nextPage = NULL;
              delete page;
              
              return;
            }
            
            p = p->nextPage;
          }
        }
      }
    }
  }
}



void SoVolumeDataSlice::releaseLRUPage(void)
{
  SoVolumeDataPage * LRUPage = this->getLRUPage();
  this->releasePage(LRUPage);
}



SoVolumeDataPage *
SoVolumeDataSlice::getLRUPage(void)
{
  SoVolumeDataPage * LRUPage = NULL;
  if (this->pages) {
    for (int i = 0; i < this->numCols * this->numRows; i++) {
      SoVolumeDataPage * page = this->pages[i];
      while (page != NULL) {
        if (LRUPage == NULL) {
          LRUPage = page;
        }
        else {
          if (page->lastuse < LRUPage->lastuse) LRUPage = page;
        }

        page = page->nextPage;
      }
    }
  }
  return LRUPage;
}




void SoVolumeDataSlice::releaseAllPages(void)
{
  if (this->pages)
    for (int i = 0; i < this->numCols * this->numRows; i++) {
      delete this->pages[i];
      this->pages[i] = NULL;
    }

  delete[] this->pages;
  this->pages = NULL;
}




/*!
  Renders a arbitrary shaped quad. Automatically loads all pages
  needed for the given texturecoords. Texturecoords is in normalized
  coordinates [0, 1].

  vertices specified in counterclockwise order: v0 maps to lower left
  of slice, v1 maps to lower right of slice, v2 maps to upper right of
  slice, and v3 maps to upper left of slice.
*/
void SoVolumeDataSlice::render(SoState * state,
                               const SbVec3f & v0, const SbVec3f & v1,
                               const SbVec3f & v2, const SbVec3f & v3,
                               const SbBox2f & textureCoords,
                               SoTransferFunction * transferFunction,
                               long tick)
{
  assert(this->reader);
  assert(transferFunction);

  SbVec2f minUV, maxUV;
  textureCoords.getBounds(minUV, maxUV);

  SbVec2f pageSizef =
    SbVec2f(float(this->pageSize[0])/float(dimensions[0]),
            float(this->pageSize[1])/float(dimensions[1]));

  // Local page-UV-coordinates for the current quad to be rendered.
  SbVec2f localMinUV, localMaxUV;

  // Global slice-UV-coordinates for the current quad to be rendered.
  SbVec2f globalMinUV, globalMaxUV;

  // Vertices for left and right edge of current row
  SbVec3f endLowerLeft, endLowerRight;
  SbVec3f endUpperLeft, endUpperRight;

  // Vertices for current quad to be rendered
  SbVec3f upperLeft, upperRight, lowerLeft, lowerRight;

  globalMinUV = minUV;
  endLowerLeft = v0;
  endLowerRight = v1;
  int row = (int) (minUV[1]*this->numRows);
  while (globalMinUV[1] != maxUV[1]) {

    if ((row + 1)*pageSizef[1] < maxUV[1]) {
      // This is not the last row to be rendered
      globalMaxUV[1] = (row + 1)*pageSizef[1];
      localMaxUV[1] = 1.0;

      // Interpolating the row's endvertices
      float k =
        float(globalMaxUV[1] - minUV[1])/float(maxUV[1] - minUV[1]);
      endUpperLeft[0] = (1 - k)*v0[0] + k*v3[0];
      endUpperLeft[1] = (1 - k)*v0[1] + k*v3[1];
      endUpperLeft[2] = (1 - k)*v0[2] + k*v3[2];

      endUpperRight[0] = (1 - k)*v1[0] + k*v2[0];
      endUpperRight[1] = (1 - k)*v1[1] + k*v2[1];
      endUpperRight[2] = (1 - k)*v1[2] + k*v2[2];
    }
    else {

      // This is the last row to be rendered
      globalMaxUV[1] = maxUV[1];
      localMaxUV[1] = (globalMaxUV[1] - row*pageSizef[1])/pageSizef[1];

      endUpperLeft = v3;
      endUpperRight = v2;
    }


    int col = (int) (minUV[0]*this->dimensions[0]/this->pageSize[0]);
    globalMinUV[0] = minUV[0];
    localMinUV[0] = (globalMinUV[0] - col*pageSizef[0])/pageSizef[0];
    localMinUV[1] = (globalMinUV[1] - row*pageSizef[1])/pageSizef[1];
    lowerLeft = endLowerLeft;
    upperLeft = endUpperLeft;
    while (globalMinUV[0] != maxUV[0]) {
      if ((col + 1)*pageSizef[0] < maxUV[0]) {

        // Not the last quad on the row
        globalMaxUV[0] = (col + 1)*pageSizef[0];
        localMaxUV[0] = 1.0;

        // Interpolating the quad's rightmost vertices
        float k =
          float(globalMaxUV[0] - minUV[0])/float(maxUV[0] - minUV[0]);
        lowerRight = (1 - k)*endLowerLeft + k*endLowerRight;
        upperRight = (1 - k)*endUpperLeft + k*endUpperRight;

      }
      else {

        // The last quad on the row
        globalMaxUV[0] = maxUV[0];
        localMaxUV[0] = (maxUV[0] - col*pageSizef[0])/pageSizef[0];

        lowerRight = endLowerRight;
        upperRight = endUpperRight;
      }

      // rendering
      SoVolumeDataPage * page = getPage(col, row, transferFunction);
      if (!page)
        page = buildPage(col, row, transferFunction);

      page->setActivePage(tick);

      glBegin(GL_QUADS);
      glColor4f(1, 1, 1, 1);
      glTexCoord2f(localMinUV[0], localMinUV[1]);
      glVertex3f(lowerLeft[0], lowerLeft[1], lowerLeft[2]);
      glTexCoord2f(localMaxUV[0], localMinUV[1]);
      glVertex3f(lowerRight[0], lowerRight[1], lowerRight[2]);
      glTexCoord2f(localMaxUV[0], localMaxUV[1]);
      glVertex3f(upperRight[0], upperRight[1], upperRight[2]);
      glTexCoord2f(localMinUV[0], localMaxUV[1]);
      glVertex3f(upperLeft[0], upperLeft[1], upperLeft[2]);
      glEnd();

      globalMinUV[0] = globalMaxUV[0];
      lowerLeft = lowerRight;
      upperLeft = upperRight;
      localMinUV[0] = 0.0;
      col++;
    }

    globalMinUV[1] = globalMaxUV[1];
    localMinUV[0] = 0.0;
    endLowerLeft = endUpperLeft;
    endLowerRight = endUpperRight;
    row++;
  }

}


/*!
  Builds a page if it doesn't exist. Rebuilds it if it does exist.
*/
SoVolumeDataPage *
SoVolumeDataSlice::buildPage(int col, int row,
                             SoTransferFunction * transferFunction)
{
  assert(this->reader);
  assert(transferFunction);

  // Does the page exist already?
  SoVolumeDataPage * page = this->getPage(col, row, transferFunction);
  if (!page) {

    // First SoVolumeDataPage ever in this slice?
    if (!this->pages) {
      this->pages = new SoVolumeDataPage*[this->numCols * this->numRows];
      memset(this->pages, 0,
             sizeof(SoVolumeDataPage*) * this->numCols * this->numRows);
    }

    page = new SoVolumeDataPage;
    SoVolumeDataPage ** pNewPage = &pages[row * this->numCols + col];
    while (*pNewPage != NULL) {
      pNewPage = &((*pNewPage)->nextPage);
    }

    *pNewPage = page;
  }

  SbBox2s subSlice = SbBox2s(col*pageSize[0],
                             row*pageSize[1],
                             (col + 1)*pageSize[0],
                             (row + 1)*pageSize[1]);

  void * transferredTexture = NULL;
  float * palette = NULL;
  int paletteDataType;
  int outputDataType;
  int paletteSize;
  unsigned char * texture = new unsigned char[pageSize[0] * pageSize[1] * 4];

  SoVolumeReader::Axis ax =
    this->axis == SoOrthoSlice::X ?
    SoVolumeReader::X : (this->axis == SoOrthoSlice::Y ?
                         SoVolumeReader::Y : SoVolumeReader::Z);
  reader->getSubSlice(subSlice, sliceIdx, texture, ax);

  transferFunction->transfer(texture,
                             this->dataType,
                             pageSize,
                             transferredTexture,
                             outputDataType,
                             palette,
                             paletteDataType,
                             paletteSize);

  delete[] texture;

  page->setData(SoVolumeDataPage::OPENGL,
                (unsigned char*)transferredTexture,
                pageSize,
                palette,
                paletteDataType,
                paletteSize);

  page->transferFunctionId = transferFunction->getNodeId();

  delete[] ((char*) transferredTexture);
  delete[] palette;

  pages[row*this->numCols + col] = page;

  numTexels += pageSize[0]*pageSize[1];
  numPages++;
  numBytesSW += page->numBytesSW;
  numBytesHW += page->numBytesHW;

  return page;
}






SoVolumeDataPage *
SoVolumeDataSlice::getPage(int col, int row,
                           SoTransferFunction * transferFunction)
{
  if (!this->pages) return NULL;

  // Valid SoVolumeDataPage?
  if ((col >= this->numCols) || (row >= this->numRows))
    return NULL;

  SoVolumeDataPage * p = pages[row * this->numCols + col];

  while (p != NULL) {
    if (p->transferFunctionId == transferFunction->getNodeId()) break;
    p = p->nextPage;
  }

  return p;
}
