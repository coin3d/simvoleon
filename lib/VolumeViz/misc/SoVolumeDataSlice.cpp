#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif // HAVE_WINDOWS_H
#include <GL/gl.h>

/*
Notes for documentation:
* The slice's reader is set in the init-function. Only the pointer
  is saved. If any of the reader's data is changed, the init-function
  must be rerun to setup all the correct values. 

* Describe the datastructure for pages. 

*/


#include <VolumeViz/misc/SoVolumeDataSlice.h>
#include <Inventor/misc/SoState.h>
#include <Inventor/actions/SoGLRenderAction.h>

SoVolumeDataSlice::SoVolumeDataSlice()
{
  this->pageSize = SbVec2s(32, 32);
  this->pages = NULL;
  this->axis = SoVolumeRendering::Z;
  this->numPages = 0;
  this->numTexels = 0;
  this->sliceIdx = 0;
  this->reader;
}// constructor


SoVolumeDataSlice::~SoVolumeDataSlice()
{
  this->reader = NULL;
  releaseAllPages();
}// destructor


void SoVolumeDataSlice::init(SoVolumeReader * reader,
                             int sliceIdx,
                             SoVolumeRendering::Axis axis,
                             SbVec2s &pageSize)
{
  this->reader = reader;
  this->sliceIdx = sliceIdx;
  this->axis = axis;
  this->pageSize = pageSize;

  SbVec3s dim;
  SbBox3f size;
  SoVolumeRendering::DataType type;
  reader->getDataChar(size, type, dim);

  switch (axis) {
    case SoVolumeRendering::X: 
      {
        this->dimensions[0] = dim[2];
        this->dimensions[1] = dim[1];
      }// case
      break;

    case SoVolumeRendering::Y: 
      {
        this->dimensions[0] = dim[0];
        this->dimensions[1] = dim[2];
      }// case
      break;

    case SoVolumeRendering::Z: 
      {
        this->dimensions[0] = dim[0];
        this->dimensions[1] = dim[1];
      }// case
      break;
  }// switch

  this->numCols = this->dimensions[0]/this->pageSize[0];
  this->numRows = this->dimensions[1]/this->pageSize[1];                  
}// init



/*
Searches for a specified page in the slice. The page
is identified by it's memorylocation (the pointer). If the
page isn't found, nothing happens. 
*/
void 
SoVolumeDataSlice::releasePage(SoVolumeDataPage * page)
{
  if (pages) {
    for (int i = 0; i < numCols*numRows; i++) {
      SoVolumeDataPage *p = pages[i];

      if (p != NULL) {
        // Delete the first page in the linked list
        if (p == page) {
          pages[i] = p->nextPage;
          p->nextPage = NULL;
          delete p;

          numPages--;
          numTexels -= pageSize[0]*pageSize[1];
          return;
        }// if
        else
        while (p->nextPage != NULL) {
          if (p->nextPage == page) {
            p->nextPage = p->nextPage->nextPage;
            page->nextPage = NULL;
            delete page;

            numPages--;
            numTexels -= pageSize[0]*pageSize[1];
            return;
          }// if
      
          p = p->nextPage;
        }// while
      }// if
    }// for
  }// if
}// releasePage



void SoVolumeDataSlice::releaseLRUPage()
{
  SoVolumeDataPage * LRUPage = getLRUPage();
  releasePage(LRUPage);
}// releaseLRUPage



SoVolumeDataPage * 
SoVolumeDataSlice::getLRUPage()
{
  SoVolumeDataPage * LRUPage = NULL;
  if (pages) {
    for (int i = 0; i < numCols*numRows; i++) {
      SoVolumeDataPage * page = pages[i];
      while (page != NULL) {
        if (LRUPage == NULL) 
          LRUPage = page;
        else
          if (page->lastuse < LRUPage->lastuse)
            LRUPage = page;

        page = page->nextPage;
      }// while
    }// for
  }// if
  return LRUPage;
}// releaseLRUPage




void SoVolumeDataSlice::releaseAllPages()
{
  if (pages)
    for (int i = 0; i < numCols*numRows; i++) {
      delete pages[i];
      pages[i] = NULL;
    }//for

  delete [] pages;
}// releaseAllPages

/*
vertices specified in counterclockwise order. 
v0 maps to lower left of slice. 
v1 maps to lower right of slice.
v2 maps to upper right of slice.
v3 maps to upper left of slice.
*/
void SoVolumeDataSlice::render(SoState * state,
                               SbVec3f &v0, 
                               SbVec3f &v1, 
                               SbVec3f &v2, 
                               SbVec3f &v3, 
                               SbBox2f &textureCoords, 
                               SoTransferFunction * transferFunction,
                               long tick)
{
  assert(reader);

  // FIXME: Enable this when the transferfunctions are implemented.
  // 08092002 torbjorv
  // assert(transferFunction);

  SbVec2f minUV, maxUV;
  textureCoords.getBounds(minUV, maxUV);

  SbVec2f pageSizef = SbVec2f(float(this->pageSize[0])/float(dimensions[0]), 
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
  int row = minUV[1]*this->numRows;
  while (globalMinUV[1] != maxUV[1]) {

    if ((row + 1)*pageSizef[1] < maxUV[1]) {
      // This is not the last row to be rendered
      globalMaxUV[1] = (row + 1)*pageSizef[1];
      localMaxUV[1] = 1.0;

      // Interpolating the row's endvertices
      float k = float(globalMaxUV[1] - minUV[1])/float(maxUV[1] - minUV[1]);
      endUpperLeft[0] = (1 - k)*v0[0] + k*v3[0];
      endUpperLeft[1] = (1 - k)*v0[1] + k*v3[1];
      endUpperLeft[2] = (1 - k)*v0[2] + k*v3[2];

      endUpperRight[0] = (1 - k)*v1[0] + k*v2[0];
      endUpperRight[1] = (1 - k)*v1[1] + k*v2[1];
      endUpperRight[2] = (1 - k)*v1[2] + k*v2[2];
    }// if
    else {

      // This is the last row to be rendered
      globalMaxUV[1] = maxUV[1];
      localMaxUV[1] = (globalMaxUV[1] - row*pageSizef[1])/pageSizef[1];

      endUpperLeft = v3;
      endUpperRight = v2;
    }// else
  

    int col = minUV[0]*this->dimensions[0]/this->pageSize[0];
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
        float k = float(globalMaxUV[0] - minUV[0])/float(maxUV[0] - minUV[0]);
        lowerRight = (1 - k)*endLowerLeft + k*endLowerRight;
        upperRight = (1 - k)*endUpperLeft + k*endUpperRight;

      }// if
      else {

        // The last quad on the row
        globalMaxUV[0] = maxUV[0];
        localMaxUV[0] = (maxUV[0] - col*pageSizef[0])/pageSizef[0];
        
        lowerRight = endLowerRight;
        upperRight = endUpperRight;
      }// else

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
    }// while

    globalMinUV[1] = globalMaxUV[1];
    localMinUV[0] = 0.0;
    endLowerLeft = endUpperLeft;
    endLowerRight = endUpperRight;
    row++;
  }// while

}//renderSlice






/*
Builds a page if it doesn't exist. Rebuilds it if it does exist. 
*/

// FIXME: Create support for different dataformats. torbjorv 08092002
SoVolumeDataPage * SoVolumeDataSlice::buildPage(int col, 
                                                int row,
                                                SoTransferFunction * transferFunction)
{
  assert(reader);
  // FIXME: Enable this when the transferfunctions are implemented.
  // 08092002 torbjorv
  // assert(transferFunction);

  // Does the page exist already?
  SoVolumeDataPage * page = getPage(col, row, transferFunction);
  if (!page) {

    // First SoVolumeDataPage ever in this slice?
    if (!this->pages) {
      pages = new SoVolumeDataPage*[this->numCols*this->numRows];
      memset( this->pages, 
              0, 
              sizeof(SoVolumeDataPage*)*this->numCols*this->numRows);
    }// if

    page = new SoVolumeDataPage;
    SoVolumeDataPage **pNewPage = &pages[row*this->numCols + col];
    while (*pNewPage != NULL)
      pNewPage = &((*pNewPage)->nextPage);

    *pNewPage = page;
  }// if

  SbBox2s subSlice = SbBox2s(col*pageSize[0], 
                             row*pageSize[1], 
                             (col + 1)*pageSize[0],
                             (row + 1)*pageSize[1]);



  unsigned char * RGBATexture = new unsigned char[pageSize[0]*pageSize[1]*4];

  reader->getSubSlice(subSlice, 
                      sliceIdx, 
                      RGBATexture, 
                      axis);

    
  page->setData(SoVolumeDataPage::OPENGL,
                RGBATexture, 
                SbVec2s(pageSize[0], pageSize[1]), 
                4,
                GL_RGBA);

  delete [] RGBATexture;

  pages[row*this->numCols + col] = page;

  numTexels += pageSize[0]*pageSize[1];
  numPages++;

  return page;
}// buildPage






SoVolumeDataPage * 
SoVolumeDataSlice::getPage(int col, 
                           int row, 
                           SoTransferFunction * transferFunction)
{
  if (!pages) return NULL;

  // Valid SoVolumeDataPage?
  if ((col >= this->numCols) || 
      (row >= this->numRows))
    return NULL;

  SoVolumeDataPage * p = pages[row*this->numCols + col];


  // FIXME: Enable these lines as soon as the transferfunctions are implemented.
/*  while (p != NULL) {
    if (p->transferFunctionId == transferFunction->getNodeId()) break;
    p = p->nextPage;
  }//while*/

  return p;
}//getPage

