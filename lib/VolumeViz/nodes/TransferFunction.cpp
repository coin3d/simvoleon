/*!
  \class SoTransferFunction VolumeViz/nodes/SoTransferFunction.h
  \brief Contains the transfer function definition.
  \ingroup volviz
*/

#include <VolumeViz/nodes/SoTransferFunction.h>

#include <string.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/system/gl.h>
#include <VolumeViz/elements/SoTransferFunctionElement.h>


// *************************************************************************

#include <gradients/GREY.h>
#include <gradients/TEMPERATURE.h>
#include <gradients/PHYSICS.h>
#include <gradients/STANDARD.h>
#include <gradients/GLOW.h>
#include <gradients/BLUE_RED.h>
#include <gradients/SEISMIC.h>

// *************************************************************************

SO_NODE_SOURCE(SoTransferFunction);

// *************************************************************************

struct GIMPGradientSegment {
  float left, middle, right;
  float left_RGBA[4];
  float right_RGBA[4];
  int type;
  int color;
};

struct GIMPGradient {
  int nrsegments;
  struct GIMPGradientSegment * segments;
};

class SoTransferFunctionP {
public:
  SoTransferFunctionP(SoTransferFunction * master) {
    this->master = master;
  }

  int unpack(const void * data, int numBits, int index);
  void pack(void * data, int numBits, int index, int val);

  static uint8_t PREDEFGRADIENTS[SoTransferFunction::SEISMIC + 1][256][4];

  // FIXME: these two would better be refactored to be part of the
  // GIMPGradient class. 20021113 mortene.
  static struct GIMPGradient * readGIMPGradient(const char * buffer);
  static void convertGIMPGradient2IntArray(const struct GIMPGradient * gg,
                                           uint8_t intgradient[256][4]);

  static void initPredefGradients(void);

private:
  SoTransferFunction * master;
};

uint8_t SoTransferFunctionP::PREDEFGRADIENTS[SoTransferFunction::SEISMIC + 1][256][4];

#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

// *************************************************************************

SoTransferFunction::SoTransferFunction(void)
{
  SO_NODE_CONSTRUCTOR(SoTransferFunction);

  PRIVATE(this) = new SoTransferFunctionP(this);

  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, NONE);
  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, GREY);
  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, GRAY);
  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, TEMPERATURE);
  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, PHYSICS);
  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, STANDARD);
  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, GLOW);
  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, BLUE_RED);
  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, SEISMIC);
  SO_NODE_SET_SF_ENUM_TYPE(predefColorMap, PredefColorMap);

  SO_NODE_DEFINE_ENUM_VALUE(ColorMapType, ALPHA);
  SO_NODE_DEFINE_ENUM_VALUE(ColorMapType, LUM_ALPHA);
  SO_NODE_DEFINE_ENUM_VALUE(ColorMapType, RGBA);
  SO_NODE_SET_SF_ENUM_TYPE(colorMapType, ColorMapType);

  SO_NODE_ADD_FIELD(shift, (0));
  SO_NODE_ADD_FIELD(offset, (0));
  SO_NODE_ADD_FIELD(predefColorMap, (GREY));
  SO_NODE_ADD_FIELD(colorMapType, (RGBA));
  SO_NODE_ADD_FIELD(colorMap, (0));
}


SoTransferFunction::~SoTransferFunction()
{
  delete PRIVATE(this);
}


void
SoTransferFunction::initClass(void)
{
  SO_NODE_INIT_CLASS(SoTransferFunction, SoVolumeRendering, "SoVolumeRendering");

  SO_ENABLE(SoGLRenderAction, SoTransferFunctionElement);

  SoTransferFunctionP::initPredefGradients();
}

void
SoTransferFunction::GLRender(SoGLRenderAction * action)
{
  SoTransferFunctionElement::setTransferFunction(action->getState(), 
                                                 this, this);
}


// FIXME: expose tidbits.h properly from Coin. 20021109 mortene.
extern "C" {
enum CoinEndiannessValues { COIN_HOST_IS_UNKNOWNENDIAN = -1, COIN_HOST_IS_LITTLEENDIAN = 0, COIN_HOST_IS_BIGENDIAN = 1 };
extern int coin_host_get_endianness(void);
}

static int endianness = COIN_HOST_IS_UNKNOWNENDIAN;


/*!
  Transfers voxel data from input buffer to output buffer, according
  to specified parameters.

  Output buffer is allocated within the function.
*/
void
SoTransferFunction::transfer(const void * input, 
                             SoVolumeData::DataType inputdatatype,
                             const SbVec2s & size,
                             void *& output,
                             int & outputFormat,
                             float *& palette,
                             int & paletteFormat,
                             int & palettesize)
{
  endianness = coin_host_get_endianness();
  assert(endianness != COIN_HOST_IS_UNKNOWNENDIAN && "weird hardware!");


  // FIXME: the RGBA datatype for inputdata should be killed,
  // methinks. 20021112 mortene.

  // Handling RGBA inputdata. Just forwarding to output
  if (inputdatatype == SoVolumeData::RGBA) {
    // FIXME: this is of course completely wrong -- the data should be
    // handled according to the SoTransferFunction settings. 20021112 mortene.

    // FIXME: we're using a GLEnum here, but SoVolumeData enum
    // below. 20021112 mortene.
    outputFormat = GL_RGBA;

    palette = NULL;
    paletteFormat = 0;
    palettesize = 0;

    output = new int[size[0]*size[1]];
    (void)memcpy(output, input, size[0]*size[1]*sizeof(int));
    return;
  }

#if 1
  if (inputdatatype == SoVolumeData::UNSIGNED_BYTE) {
    // FIXME: this is of course completely wrong -- the data should be
    // handled according to the SoTransferFunction settings. 20021112 mortene.

    // FIXME: we're using a GLEnum here, but SoVolumeData enum
    // below. 20021112 mortene.
    outputFormat = GL_RGBA;

    palette = NULL;
    paletteFormat = 0;
    palettesize = 0;

    uint32_t * outp = new uint32_t[size[0] * size[1]];
    const uint8_t * inp = (const uint8_t *)input;

    const int cmap = this->predefColorMap.getValue();
    assert(cmap >= GREY); // FIXME: "NONE" not handled yet. 20021113 mortene.
    assert(cmap <= SEISMIC);

    if (endianness == COIN_HOST_IS_LITTLEENDIAN) {
      for (int j=0; j < size[0]*size[1]; j++) {
        if (inp[j] == 0x00) {
          outp[j] = 0x00000000;
        }
        else {
          uint8_t * rgba = SoTransferFunctionP::PREDEFGRADIENTS[cmap][inp[j]];

          outp[j] =
            (uint32_t(rgba[0]) << 0) | // red
            (uint32_t(rgba[1]) << 8) | // green
            (uint32_t(rgba[2]) << 16) |  // blue
            (uint32_t(rgba[3]) << 24); // alpha
        }
      }
    }
    else {
      // FIXME: augh! Must be fixed before release. 20021113 mortene.
      assert(FALSE && "only little-endian platforms during development");
    }

    output = outp;
    return;
  }

  if (inputdatatype == SoVolumeData::UNSIGNED_SHORT) {
    // FIXME: this is of course completely wrong -- the data should be
    // handled according to the SoTransferFunction settings. 20021112 mortene.

    // FIXME: we're using a GLEnum here, but SoVolumeData enum
    // below. 20021112 mortene.
    outputFormat = GL_RGBA;

    palette = NULL;
    paletteFormat = 0;
    palettesize = 0;

    uint32_t * outp = new uint32_t[size[0] * size[1]];
    const uint16_t * inp = (const uint16_t *)input;

    if (endianness == COIN_HOST_IS_LITTLEENDIAN) {
      for (int j=0; j < size[0]*size[1]; j++) {
        outp[j] =
          (0 << 0) | // red
          (uint32_t(inp[j] & 0x00ff) << 8) | // green
          (uint32_t((inp[j] >> 8) & 0x00ff) << 16) | // blue
          ((inp[j] ? 0xff : 0) << 24); // alpha
      }
    }
    else {
      for (int j=0; j < size[0]*size[1]; j++) {
        outp[j] =
          (0 << 24) | // red
          (uint32_t(inp[j] & 0x00ff) << 16) | // green
          (uint32_t((inp[j] >> 8) & 0x00ff) << 8) | // blue
          ((inp[j] ? 0xff : 0) << 0); // alpha
      }
    }

    output = outp;
    return;
  }

  assert(FALSE && "unknown input format");

#else
  // FIXME: tmp disabled all use of paletted (and compressed?)
  // textures. (The code wasn't working at all, as far as I could
  // see.) 20021112 mortene.
  
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

    // FIXME: we're using a SoVolumeData enum here, but GLEnum
    // above for outputFormat. 20021112 mortene.

  // Deciding outputformat
  int newNumBits = 8;
  outputFormat = SoVolumeData::UNSIGNED_BYTE;
  if (palettesize > 256) {
    newNumBits = 16;
    outputFormat = SoVolumeData::UNSIGNED_SHORT;
  }

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
}

/*!
  Handles packed data for 1, 2, 4, 8 and 16 bits. Assumes 16-bit data
  are stored in little-endian format (that is the x86-way,
  right?). And MSB is the leftmost of the bitstream...? Think so.
*/
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



/*!
  Saves the index specified in val with the specified number of bits,
  at the specified index (not elementindex, not byteindex) in data.
*/
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


void
SoTransferFunction::reMap(int min, int max)
{
  // FIXME: Implement this function torbjorv 08282002
}

struct GIMPGradient *
SoTransferFunctionP::readGIMPGradient(const char * buf)
{
  // The format of a gradient in GIMP v1.2 is:
  //
  // <header = "GIMP Gradient\n">
  // <nrsegments>\n
  // <segment 0: left middle right leftR leftG leftB leftA rightR rightG rightB rightA type color>
  // <segment 1: ...>
  // ...
  //
  // Note that the format has changed after GIMP 1.2.

  const int BUFLEN = strlen(buf);
  const char * HEADER = "GIMP Gradient\n";
  const int HEADERLEN = strlen(HEADER);

  const char * ptr = buf + HEADERLEN;
  assert(ptr < (buf + BUFLEN));

  struct GIMPGradient * gg = new struct GIMPGradient;

  int r = sscanf(ptr, "%d\n", &gg->nrsegments);
  assert(r == 1); // Note: this will fail if input is from Gimp v > 1.2.

  gg->segments = new struct GIMPGradientSegment[gg->nrsegments];

  while (*ptr++ != '\n');
  for (int i=0; i < gg->nrsegments; i++) {
    struct GIMPGradientSegment * s = &gg->segments[i];

    r = sscanf(ptr,
               "%f %f %f "
               "%f %f %f %f "
               "%f %f %f %f "
               "%d %d",
               &s->left, &s->middle, &s->right,
               &s->left_RGBA[0], &s->left_RGBA[1], &s->left_RGBA[2], &s->left_RGBA[3],
               &s->right_RGBA[0], &s->right_RGBA[1], &s->right_RGBA[2], &s->right_RGBA[3],
               &s->type, &s->color);
    assert(r == 13);

    // Consistency check to help us catch bugs early.
    assert(s->type == 0 && "unhandled gradient data");
    assert(s->color == 0 && "unhandled gradient data");
    assert(s->left < s->middle);
    assert(s->middle < s->right);
    assert(s->left >= 0.0f && s->left <= 1.0f);
    assert(s->middle >= 0.0f && s->middle <= 1.0f);
    assert(s->right >= 0.0f && s->right <= 1.0f);
    assert(s->left_RGBA[0] >= 0.0f && s->left_RGBA[0] <= 1.0f);
    assert(s->left_RGBA[1] >= 0.0f && s->left_RGBA[1] <= 1.0f);
    assert(s->left_RGBA[2] >= 0.0f && s->left_RGBA[2] <= 1.0f);
    assert(s->left_RGBA[3] >= 0.0f && s->left_RGBA[3] <= 1.0f);
    assert(s->right_RGBA[0] >= 0.0f && s->right_RGBA[0] <= 1.0f);
    assert(s->right_RGBA[1] >= 0.0f && s->right_RGBA[1] <= 1.0f);
    assert(s->right_RGBA[2] >= 0.0f && s->right_RGBA[2] <= 1.0f);
    assert(s->right_RGBA[3] >= 0.0f && s->right_RGBA[3] <= 1.0f);

    while (*ptr++ != '\n');
  }

  return gg;
}

// Fills out a 256-value array of integer values, by iterating over
// the colors of the GIMP gradient.
void
SoTransferFunctionP::convertGIMPGradient2IntArray(const struct GIMPGradient * gg,
                                                  uint8_t intgradient[256][4])
{
  int segmentidx = 0;
  struct GIMPGradientSegment * segment = NULL;
  float middle_RGBA[4];

  for (int i=0; i < 256; i++) {
    float gradpos = (1.0f / 256.0f) * float(i);

    // Advance to correct segment, if necessary.
    while ((segment == NULL) || (gradpos > segment->right)) {
      assert(segmentidx < gg->nrsegments);
      segment = &gg->segments[segmentidx];
      segmentidx++;

      // While we're at it, calculate the RGBA value of the middle
      // gradient point of the new segment.
      for (int j=0; j < 4; j++) {
        middle_RGBA[j] =
          (segment->right_RGBA[j] - segment->left_RGBA[j]) / 2.0f +
          segment->left_RGBA[j];
      }
    }

    float left, right;
    float left_RGBA[4], right_RGBA[4];
    if (gradpos < segment->middle) {
      left = segment->left;
      right = segment->middle;
      left_RGBA = segment->left_RGBA;
      right_RGBA = middle_RGBA;
    }
    else {
      left = segment->middle;
      right = segment->right;
      left_RGBA = middle_RGBA;
      right_RGBA = segment->right_RGBA;
    }

    for (int k=0; k < 4; k++) {
      float changeperunit = float(right_RGBA[k] - left_RGBA[k]) / (right - left);
      float add = changeperunit * (gradpos - left);
      intgradient[i][k] = uint8_t((left_RGBA[k] + add) * 255.0f);
    }
  }
}

// Initialize all 256-value predefined colormaps.
void
SoTransferFunctionP::initPredefGradients(void)
{
  const char * gradientbufs[] = {
    GREY_gradient, TEMPERATURE_gradient, PHYSICS_gradient,
    STANDARD_gradient, GLOW_gradient, BLUE_RED_gradient, SEISMIC_gradient
  };

  for (unsigned int j=0; j < sizeof(gradientbufs)/sizeof(gradientbufs[0]); j++) {
    struct GIMPGradient * gg =
      SoTransferFunctionP::readGIMPGradient(gradientbufs[j]);
    SoTransferFunctionP::convertGIMPGradient2IntArray(gg, SoTransferFunctionP::PREDEFGRADIENTS[j+1]);

#if 0 // DEBUG: spits out a 256x1 image of the colormap.
    SbString s;
    s.sprintf("/tmp/gradient-%d.ppm", j + 1);
    FILE * f = fopen(s.getString(), "w");
    assert(f);
    (void)fprintf(f, "P3\n256 1 255\n"); // width height maxcolval

    for (int i=0; i < 256; i++) {
      fprintf(f, "%d %d %d\n",
              SoTransferFunctionP::PREDEFGRADIENTS[TEMPERATURE][i][0],
              SoTransferFunctionP::PREDEFGRADIENTS[TEMPERATURE][i][1],
              SoTransferFunctionP::PREDEFGRADIENTS[TEMPERATURE][i][2]);
    }
    fclose(f);
#endif // DEBUG
  }
}
