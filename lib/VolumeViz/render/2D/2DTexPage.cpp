// From torbjorv's dictionary: a "slice" is "one complete cut through
// the volume, normal to an axis".

#include <VolumeViz/render/2D/Cvr2DTexPage.h>

#include <Inventor/errors/SoDebugError.h>
#include <Inventor/system/gl.h>


// *************************************************************************

// There's a linked list of pages on each [row, col] spot in a slice,
// because we need to make different pages for different
// SoTransferFunction instances.

class Cvr2DTexSubPageItem {
public:
  Cvr2DTexSubPageItem(Cvr2DTexSubPage * p)
  {
    this->page = p;
    this->next = NULL;
    this->prev = NULL;
  }

  Cvr2DTexSubPage * page;
  Cvr2DTexSubPageItem * next, * prev;
};

// *************************************************************************


Cvr2DTexPage::Cvr2DTexPage(void)
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


Cvr2DTexPage::~Cvr2DTexPage()
{
  this->reader = NULL;
  this->releaseAllPages();
}


void Cvr2DTexPage::init(SoVolumeReader * reader, int sliceIdx,
                             SoOrthoSlice::Axis axis,
                             const SbVec2s & pageSize)
{
  assert(pageSize[0] > 0 && pageSize[1] > 0);

  this->releaseAllPages();

  this->reader = reader;
  this->sliceIdx = sliceIdx;
  this->axis = axis;
  this->pageSize = pageSize;

  SbVec3s dim;
  SbBox3f size;
  reader->getDataChar(size, this->dataType, dim);

  assert(dim[0] > 0);
  assert(dim[1] > 0);
  assert(dim[2] > 0);

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

#if 0 // debug
  SoDebugError::postInfo("void Cvr2DTexPage::init",
                         "this->dimensions=[%d, %d], this->pageSize=[%d, %d]",
                         this->dimensions[0], this->dimensions[1],
                         this->pageSize[0], this->pageSize[1]);
#endif // debug

  this->numCols = (this->dimensions[0] + this->pageSize[0] - 1) / this->pageSize[0];
  this->numRows = (this->dimensions[1] + this->pageSize[1] - 1) / this->pageSize[1];

  assert(this->numCols > 0);
  assert(this->numRows > 0);
}



/*!
  Release resources used by a page in the slice.
*/
void
Cvr2DTexPage::releasePage(Cvr2DTexSubPage * page)
{
  assert(page != NULL);
  assert(this->pages != NULL);

  const int NRPAGES = this->numCols * this->numRows;
  for (int i = 0; i < NRPAGES; i++) {
    Cvr2DTexSubPageItem * p = this->pages[i];
    if (p == NULL) continue; // skip this, continue with for-loop

    while ((p != NULL) && (p->page != page)) { p = p->next; }
    if (p == NULL) continue; // not the right list, continue with for-loop

    this->numPages--;
    this->numTexels -= this->pageSize[0] * this->pageSize[1];
    this->numBytesHW -= p->page->numBytesHW;

    if (p->next) { p->next->prev = p->prev; }
    if (p->prev) { p->prev->next = p->next; }
    else { this->pages[i] = p->next; }

    delete p->page;
    delete p;
    return;
  }
  assert(FALSE && "couldn't find page");
}



void Cvr2DTexPage::releaseLRUPage(void)
{
  assert(this->pages != NULL);

  Cvr2DTexSubPage * LRUPage = this->getLRUPage();
  assert(LRUPage != NULL);
  this->releasePage(LRUPage);
}



Cvr2DTexSubPage *
Cvr2DTexPage::getLRUPage(void)
{
  assert(this->pages != NULL);

  Cvr2DTexSubPage * LRUPage = NULL;

  const int NRPAGES = this->numCols * this->numRows;
  for (int i = 0; i < NRPAGES; i++) {
    Cvr2DTexSubPageItem * pitem = this->pages[i];
    while (pitem != NULL) {
      if ((LRUPage == NULL) || (pitem->page->lastuse < LRUPage->lastuse)) {
        LRUPage = pitem->page;
      }
      pitem = pitem->next;
    }
  }
  return LRUPage;
}


void
Cvr2DTexPage::releaseAllPages(void)
{
  if (this->pages == NULL) return;

  for (int i = 0; i < this->numCols * this->numRows; i++) {
    Cvr2DTexSubPageItem * pitem = this->pages[i];
    if (pitem == NULL) continue;

    do {
      Cvr2DTexSubPageItem * prev = pitem;
      pitem = pitem->next;
      delete prev->page;
      delete prev;
    } while (pitem != NULL);

    this->pages[i] = NULL;
  }

  delete[] this->pages;
  this->pages = NULL;
}




/*!
  Renders arbitrary shaped quad. Automatically loads all pages needed
  for the given texturecoords. Texturecoords are in normalized
  coordinates [0, 1].

  Vertices are specified in counterclockwise order: v0 maps to lower
  left of slice, v1 maps to lower right of slice, v2 maps to upper
  right of slice, and v3 maps to upper left of slice.
*/
void Cvr2DTexPage::render(SoState * state,
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
      Cvr2DTexSubPage * page = this->getPage(col, row, transferFunction);
      if (page == NULL) { page = this->buildPage(col, row, transferFunction); }
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


int
Cvr2DTexPage::calcPageIdx(int row, int col) const
{
#if 1 // FIXME: base this on a compiler variable ("COINVOL_DEBUG" or something). 20021117 mortene.
  if (! ((row >= 0) && (row < this->numRows))) {
    SoDebugError::post("Cvr2DTexPage::calcPageIdx",
                       "row %d out of bounds, this->numRows==%d",
                       row, this->numRows);
    assert(FALSE);
  }
  if (! ((col >= 0) && (col < this->numCols))) {
    SoDebugError::post("Cvr2DTexPage::calcPageIdx",
                       "col %d out of bounds, this->numCols==%d",
                       col, this->numCols);
    assert(FALSE);
  }
#endif // debug

  return (row * this->numCols) + col;
}

/*!
  Builds a page if it doesn't exist. Rebuilds it if it does exist.
*/
Cvr2DTexSubPage *
Cvr2DTexPage::buildPage(int col, int row,
                             SoTransferFunction * transferFunction)
{
  assert(this->reader);
  assert(transferFunction);

  // Does the page exist already?
  Cvr2DTexSubPage * page = this->getPage(col, row, transferFunction);
  if (!page) {
    // First Cvr2DTexSubPage ever in this slice?
    if (this->pages == NULL) {
      int nrpages = this->numCols * this->numRows;
      this->pages = new Cvr2DTexSubPageItem*[nrpages];
      for (int i=0; i < nrpages; i++) { this->pages[i] = NULL; }
    }

    page = new Cvr2DTexSubPage;
    Cvr2DTexSubPageItem * pitem = new Cvr2DTexSubPageItem(page);

    Cvr2DTexSubPageItem * p = this->pages[this->calcPageIdx(row, col)];
    if (p == NULL) {
      this->pages[this->calcPageIdx(row, col)] = pitem;
    }
    else {
      while (p->next != NULL) { p = p->next; }
      p->next = pitem;
      pitem->prev = p;
    }
  }

  SbBox2s subSlice = SbBox2s(col * this->pageSize[0],
                             row * this->pageSize[1],
                             (col + 1) * this->pageSize[0],
                             (row + 1) * this->pageSize[1]);

  int texturebuffersize = this->pageSize[0] * this->pageSize[1] * 4;
  unsigned char * texture = new unsigned char[texturebuffersize];

  SoVolumeReader::Axis ax =
    this->axis == SoOrthoSlice::X ?
    SoVolumeReader::X : (this->axis == SoOrthoSlice::Y ?
                         SoVolumeReader::Y : SoVolumeReader::Z);
  reader->getSubSlice(subSlice, sliceIdx, texture, ax);

  uint32_t * transferredTexture = transferFunction->transfer(texture,
                                                             this->dataType,
                                                             this->pageSize);
  delete[] texture;

  float * palette = NULL;
  int paletteDataType = 0;
  int paletteSize = 0;

  // FIXME: paletted textures not supported yet. 20021119 mortene.

  page->setData(Cvr2DTexSubPage::OPENGL,
                (unsigned char *)transferredTexture,
                this->pageSize,
                palette, paletteDataType, paletteSize);

  // FIXME: this is used to invalidate a page (as far as I can see),
  // but AFAICT there is no code for actually cleaning it out (or is
  // that handled automatically by the LRU
  // algos?). Investigate. 20021111 mortene.
  page->transferFunctionId = transferFunction->getNodeId();

  delete[] transferredTexture;
  delete[] palette;

  this->numTexels += this->pageSize[0] * this->pageSize[1];
  this->numPages++;
  this->numBytesHW += page->numBytesHW;

  return page;
}


// *******************************************************************

// FIXME: tmp disabled all use of paletted (and compressed?)
// textures. (The code wasn't working at all, as far as I could
// see.) 20021112 mortene.

#if 0 // This was in SoTransferFunction::transfer():

  int numbits;
  switch (inputdatatype) {
  case SoVolumeData::UNSIGNED_BYTE: numbits = 8; break;
        
  case SoVolumeData::UNSIGNED_SHORT: numbits = 16; break;

  default: assert(FALSE && "unknown input data type"); break;
  }

  int maxpalentries = 1 << numbits;

  // Counting number of references to each palette entry     
  SbBool * palcountarray = new SbBool[maxpalentries];
  (void)memset(palcountarray, 0, sizeof(SbBool) * maxpalentries);

  int nrpalentries = 0;

  int32_t shift = this->shift.getValue(); // for optimized access in loop below
  int32_t offset = this->offset.getValue();  // for optimized access in loop

  int i;
  for (i = 0; i < size[0]*size[1]; i++) {
    int unpacked = PRIVATE(this)->unpack(input, numbits, i);
    int idx = (unpacked << shift) + offset;

    if (idx >= maxpalentries) {
#if 1 // debug
      SoDebugError::postInfo("SoTransferFunction::transfer",
                             "idx %d out-of-bounds [0, %d] "
                             "(unpacked==%d, shift==%d, offset==%d)",
                             idx, maxpalentries - 1,
                             unpacked, shift, offset);
#endif // debug
    }

    assert(idx < maxpalentries);

    if (palcountarray[idx] == FALSE) {
      palcountarray[idx] = TRUE;
      nrpalentries++;
    }
  }

  // Creating remap-table and counting number of entries in new palette
  int * remap = new int[maxpalentries];
  int palidx = 0;
  for (i = 0; i < maxpalentries; i++) {
    if (palcountarray[i]) { remap[i] = palidx++; }
    else { remap[i] = -1; }
  }

  // Calculating the new palette's size to a power of two.
  palettesize = 2;
  while (palettesize < nrpalentries) { palettesize <<= 1; }

#if 0 // debug
  SoDebugError::postInfo("SoTransferFunction::transfer",
                         "nrpalentries==%d, palettesize==%d",
                         nrpalentries, palettesize);
#endif // debug

  // Building new palette
  // FIXME: convert this to a bytearray. 20021112 mortene.
  palette = new float[palettesize*4]; // "*4" is to cover 1 byte each for RGBA
  (void)memset(palette, 0, sizeof(float)*palettesize*4);
  const float * oldPal = this->colorMap.getValues(0);
#if 0 // debug
  SoDebugError::postInfo("SoTransferFunction::transfer",
                         "this->colorMap.getNum()==%d, maxpalentries==%d",
                         this->colorMap.getNum(), maxpalentries);
#endif // debug
  // FIXME: the way we use this->colorMap here leaves much to be
  // desired wrt both robustness and correctness. 20021112 mortene.
  int tmp = 0;
  for (i = 0; i < maxpalentries; i++) {
    if (palcountarray[i]) {
      // FIXME: out-of-bounds read on oldPal! 20021112 mortene.
      (void)memcpy(&palette[tmp*4], &oldPal[i*4], sizeof(float)*4);
      tmp++;
    }
  }

  int newNumBits = 8;
  if (palettesize > 256) { newNumBits = 16; }

  // Rebuilding texturedata
  unsigned char * newTexture = 
    new unsigned char[size[0] * size[1] * newNumBits / 8];
  memset(newTexture, 0, size[0] * size[1] * newNumBits / 8);

  for (i = 0; i < size[0]*size[1]; i++) {

    int unpacked = PRIVATE(this)->unpack(input, numbits, i);
    int idx = (unpacked << shift) + offset;

    if (idx >= maxpalentries) {
#if 1 // debug
      SoDebugError::postInfo("SoTransferFunction::transfer",
                             "idx %d out-of-bounds [0, %d] "
                             "(unpacked==%d, shift==%d, offset==%d)",
                             idx, maxpalentries - 1,
                             unpacked, shift, offset);
#endif // debug
    }

    assert(idx < maxpalentries);

    idx = remap[idx];
    PRIVATE(this)->pack(newTexture, newNumBits, i, idx);
  }
  output = newTexture;
  paletteFormat = GL_RGBA;

  delete[] palcountarray;
  delete[] remap;
#endif // TMP DISABLED

#if 0  // TMP DISABLED, helper functions for the disabled code above

// Handles packed data for 1, 2, 4, 8 and 16 bits. Assumes 16-bit data
// are stored in little-endian format (that is the x86-way,
// right?). And MSB is the leftmost of the bitstream...? Think so.
int 
SoTransferFunctionP::unpack(const void * data, int numbits, int index)
{
  if (numbits == 8)
    return (int)((char*)data)[index];

  if (numbits == 16)
    return (int)((short*)data)[index];


  // Handling 1, 2 and 4 bit formats
  int bitIndex = numbits*index;
  int byteIndex = bitIndex/8;
  int localBitIndex = 8 - bitIndex%8 - numbits;

  char val = ((char*)data)[byteIndex];
  val >>= localBitIndex;
  val &= (1 << numbits) - 1;

  return val;
}



// Saves the index specified in val with the specified number of bits,
// at the specified index (not elementindex, not byteindex) in data.
void 
SoTransferFunctionP::pack(void * data, int numbits, int index, int val)
{
  if (val >= (1 << numbits))
    val = (1 << numbits) - 1;

  if (numbits == 8) {
    ((char*)data)[index] = (char)val;
  }
  else 
  if (numbits == 16) {
    ((short*)data)[index] = (short)val;
  }
  else {
    int bitIndex = numbits*index;
    int byteIndex = bitIndex/8;
    int localBitIndex = 8 - bitIndex%8 - numbits;

    char byte = ((char*)data)[byteIndex];
    char mask = (((1 << numbits) - 1) << localBitIndex) ^ -1;
    byte &= mask;
    val <<= localBitIndex;
    byte |= val;
    ((char*)data)[byteIndex] = byte;
  }
}

// *******************************************************************

#endif // TMP DISABLED code

Cvr2DTexSubPage *
Cvr2DTexPage::getPage(int col, int row,
                      SoTransferFunction * transferFunction)
{
  if (this->pages == NULL) return NULL;

  assert((col < this->numCols) && (row < this->numRows));

  Cvr2DTexSubPageItem * p = this->pages[this->calcPageIdx(row, col)];

  while (p != NULL) {
    if (p->page->transferFunctionId == transferFunction->getNodeId()) break;
    p = p->next;
  }

  return p ? p->page : NULL;
}
