#include <VolumeViz/render/2D/Cvr2DTexPage.h>

#include <Inventor/errors/SoDebugError.h>
#include <Inventor/system/gl.h>
#include <limits.h>

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
  uint32_t transferfuncid;
  long lasttick;
};

// *************************************************************************


Cvr2DTexPage::Cvr2DTexPage(void)
{
  this->subpagesize = SbVec2s(64, 64);
  this->subpages = NULL;
  this->axis = SoOrthoSlice::Z;
  this->sliceIdx = 0;
  this->reader = NULL;
  this->dataType = SoVolumeData::RGBA;
}


Cvr2DTexPage::~Cvr2DTexPage()
{
  this->reader = NULL;
  this->releaseAllSubPages();
}


void Cvr2DTexPage::init(SoVolumeReader * reader, int sliceIdx,
                        SoOrthoSlice::Axis axis,
                        const SbVec2s & subpagetexsize)
{
  assert(subpagetexsize[0] > 0 && subpagetexsize[1] > 0);

  this->releaseAllSubPages();

  this->reader = reader;
  this->sliceIdx = sliceIdx;
  this->axis = axis;
  this->subpagesize = subpagetexsize;

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

  this->nrcolumns = (this->dimensions[0] + this->subpagesize[0] - 1) / this->subpagesize[0];
  this->nrrows = (this->dimensions[1] + this->subpagesize[1] - 1) / this->subpagesize[1];

#if CVR_DEBUG && 0 // debug
  SoDebugError::postInfo("void Cvr2DTexPage::init",
                         "this->dimensions=[%d, %d], "
                         "this->nrcolumns==%d, this->nrrows==%d, "
                         "this->subpagesize=[%d, %d]",
                         this->dimensions[0], this->dimensions[1],
                         this->nrcolumns, this->nrrows,
                         this->subpagesize[0], this->subpagesize[1]);
#endif // debug

  assert(this->nrcolumns > 0);
  assert(this->nrrows > 0);
}



/*!
  Release resources used by a page in the slice.
*/
void
Cvr2DTexPage::releaseSubPage(Cvr2DTexSubPage * page)
{
  assert(page != NULL);
  assert(this->subpages != NULL);

  const int NRPAGES = this->nrcolumns * this->nrrows;
  for (int i = 0; i < NRPAGES; i++) {
    Cvr2DTexSubPageItem * p = this->subpages[i];
    if (p == NULL) continue; // skip this, continue with for-loop

    while ((p != NULL) && (p->page != page)) { p = p->next; }
    if (p == NULL) continue; // not the right list, continue with for-loop

    if (p->next) { p->next->prev = p->prev; }
    if (p->prev) { p->prev->next = p->next; }
    else { this->subpages[i] = p->next; }

    delete p->page;
    delete p;
    return;
  }
  assert(FALSE && "couldn't find page");
}



void Cvr2DTexPage::releaseLRUSubPage(void)
{
  assert(this->subpages != NULL);

  long tick;
  Cvr2DTexSubPage * LRUPage = this->getLRUSubPage(tick);
  assert(LRUPage != NULL);
  this->releaseSubPage(LRUPage);
}



Cvr2DTexSubPage *
Cvr2DTexPage::getLRUSubPage(long & tick)
{
  assert(this->subpages != NULL);

  Cvr2DTexSubPage * LRUPage = NULL;
  long lowesttick = LONG_MAX;

  const int NRPAGES = this->nrcolumns * this->nrrows;
  for (int i = 0; i < NRPAGES; i++) {
    Cvr2DTexSubPageItem * pitem = this->subpages[i];
    while (pitem != NULL) {
      if (pitem->lasttick < lowesttick) {
        LRUPage = pitem->page;
        lowesttick = pitem->lasttick;
      }
      pitem = pitem->next;
    }
  }
  tick = lowesttick;
  return LRUPage;
}


void
Cvr2DTexPage::releaseAllSubPages(void)
{
  if (this->subpages == NULL) return;

  for (int i = 0; i < this->nrcolumns * this->nrrows; i++) {
    Cvr2DTexSubPageItem * pitem = this->subpages[i];
    if (pitem == NULL) continue;

    do {
      Cvr2DTexSubPageItem * prev = pitem;
      pitem = pitem->next;
      delete prev->page;
      delete prev;
    } while (pitem != NULL);

    this->subpages[i] = NULL;
  }

  delete[] this->subpages;
  this->subpages = NULL;
}



void
Cvr2DTexPage::renderGLQuad(const SbVec3f & lowleft, const SbVec3f & lowright,
                           const SbVec3f & upleft, const SbVec3f & upright)
{
#if CVR_DEBUG && 0 // debug
  SoDebugError::postInfo("Cvr2DTexPage::renderGLQuad",
                         "minuv=[%f, %f], maxuv=[%f, %f]",
                         minuv[0], minuv[1], maxuv[0], maxuv[1]);
#endif // debug
  

  glBegin(GL_QUADS);
  glColor4f(1, 1, 1, 1);

  glTexCoord2f(0.0f, 0.0f);
  glVertex3f(lowleft[0], lowleft[1], lowleft[2]);

  glTexCoord2f(1.0f, 0.0f);
  glVertex3f(lowright[0], lowright[1], lowright[2]);

  glTexCoord2f(1.0f, 1.0f);
  glVertex3f(upright[0], upright[1], upright[2]);

  glTexCoord2f(0.0f, 1.0f);
  glVertex3f(upleft[0], upleft[1], upleft[2]);

  glEnd();
}



/*!
  Renders arbitrary shaped quad. Automatically loads all pages needed
  for the given texturecoords. Texturecoords are in normalized
  coordinates [0, 1].

  Vertices are specified in counterclockwise order: v0 maps to lower
  left of slice, v1 maps to lower right of slice, v2 maps to upper
  right of slice, and v3 maps to upper left of slice.
*/
void Cvr2DTexPage::render(SoState * state, const SbVec3f quadcoords[4],
                          SoTransferFunction * transferfunc,
                          long tick)
{
#if CVR_DEBUG && 0 // debug
  SoDebugError::postInfo("void Cvr2DTexPage::render",
                         "v0=[%f, %f, %f], "
                         "v1=[%f, %f, %f], "
                         "v2=[%f, %f, %f], "
                         "v3=[%f, %f, %f]",
                         quadcoords[0][0], quadcoords[0][1], quadcoords[0][2],
                         quadcoords[1][0], quadcoords[1][1], quadcoords[1][2],
                         quadcoords[2][0], quadcoords[2][1], quadcoords[2][2],
                         quadcoords[3][0], quadcoords[3][1], quadcoords[3][2]);
#endif // debug

  assert(this->reader);
  assert(transferfunc);

  SbVec3f subpagewidth = (quadcoords[1] - quadcoords[0]) / this->nrcolumns;
  SbVec3f subpageheight = (quadcoords[3] - quadcoords[0]) / this->nrrows;

  for (int rowidx = 0; rowidx < this->nrrows; rowidx++) {

    for (int colidx = 0; colidx < this->nrcolumns; colidx++) {

      Cvr2DTexSubPage * page = NULL;
      Cvr2DTexSubPageItem * pageitem = this->getSubPage(colidx, rowidx, transferfunc);
      if (pageitem == NULL) { pageitem = this->buildSubPage(colidx, rowidx, transferfunc); }
      assert(pageitem != NULL);
      assert(pageitem->page != NULL);

      pageitem->page->activate();
      pageitem->lasttick = tick;

      SbVec3f lowleft = quadcoords[0] +
        subpagewidth * colidx + subpageheight * rowidx;

      SbVec3f lowright = lowleft + subpagewidth;
      SbVec3f upleft = lowleft + subpageheight;
      SbVec3f upright = upleft + subpagewidth;

      this->renderGLQuad(lowleft, lowright, upleft, upright);
    }
  }
}


int
Cvr2DTexPage::calcSubPageIdx(int row, int col) const
{
  assert((row >= 0) && (row < this->nrrows));
  assert((col >= 0) && (col < this->nrcolumns));

  return (row * this->nrcolumns) + col;
}

/*!
  Builds a page if it doesn't exist. Rebuilds it if it does exist.
*/
Cvr2DTexSubPageItem *
Cvr2DTexPage::buildSubPage(int col, int row, SoTransferFunction * transferfunc)
{
  assert(this->reader);
  assert(transferfunc);

  assert(this->getSubPage(col, row, transferfunc) == NULL);

  // First Cvr2DTexSubPage ever in this slice?
  if (this->subpages == NULL) {
    int nrpages = this->nrcolumns * this->nrrows;
    this->subpages = new Cvr2DTexSubPageItem*[nrpages];
    for (int i=0; i < nrpages; i++) { this->subpages[i] = NULL; }
  }

  SbBox2s subSlice = SbBox2s(col * this->subpagesize[0],
                             row * this->subpagesize[1],
                             (col + 1) * this->subpagesize[0],
                             (row + 1) * this->subpagesize[1]);

  int texturebuffersize = this->subpagesize[0] * this->subpagesize[1] * 4;
  unsigned char * texture = new unsigned char[texturebuffersize];

  SoVolumeReader::Axis ax =
    this->axis == SoOrthoSlice::X ?
    SoVolumeReader::X : (this->axis == SoOrthoSlice::Y ?
                         SoVolumeReader::Y : SoVolumeReader::Z);
  reader->getSubSlice(subSlice, sliceIdx, texture, ax);

  uint32_t * transferredTexture = transferfunc->transfer(texture,
                                                         this->dataType,
                                                         this->subpagesize);
  delete[] texture;

  // FIXME: paletted textures not supported yet. 20021119 mortene.
  float * palette = NULL;
  int paletteSize = 0;

  Cvr2DTexSubPage * page =
    new Cvr2DTexSubPage((const uint8_t *)transferredTexture, this->subpagesize,
                        palette, paletteSize);

  delete[] transferredTexture;
  delete[] palette;

  Cvr2DTexSubPageItem * pitem = new Cvr2DTexSubPageItem(page);
  pitem->transferfuncid = transferfunc->getNodeId();
  pitem->lasttick = LONG_MAX; // avoid getting it swapped right out again

  Cvr2DTexSubPageItem * p = this->subpages[this->calcSubPageIdx(row, col)];
  if (p == NULL) {
    this->subpages[this->calcSubPageIdx(row, col)] = pitem;
  }
  else {
    while (p->next != NULL) { p = p->next; }
    p->next = pitem;
    pitem->prev = p;
  }

  return pitem;
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

Cvr2DTexSubPageItem *
Cvr2DTexPage::getSubPage(int col, int row,
                         SoTransferFunction * transferfunc)
{
  if (this->subpages == NULL) return NULL;

  assert((col >= 0) && (col < this->nrcolumns));
  assert((row >= 0) && (row < this->nrrows));

  Cvr2DTexSubPageItem * p = this->subpages[this->calcSubPageIdx(row, col)];

  while (p != NULL) {
    if (p->transferfuncid == transferfunc->getNodeId()) break;
    p = p->next;
  }

  return p;
}
