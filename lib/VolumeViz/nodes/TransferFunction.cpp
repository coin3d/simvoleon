/*!
  \class SoTransferFunction VolumeViz/nodes/SoTransferFunction.h
  \brief Contains the transfer function definition.
  \ingroup volviz
*/

#include <VolumeViz/nodes/SoTransferFunction.h>

#include <string.h>
#include <Inventor/C/tidbits.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoCallbackAction.h>
#include <Inventor/actions/SoPickAction.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/system/gl.h>
#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <VolumeViz/misc/CvrGIMPGradient.h>

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

class SoTransferFunctionP {
public:
  SoTransferFunctionP(SoTransferFunction * master) {
    this->master = master;

    // Init to lowest and highest uint16_t values.
    this->opaquethresholds[0] = 0;
    this->opaquethresholds[1] = (2 << 16) - 1;

    // Make sure this is set to an unused value, so it gets
    // initialized at first invocation.
    this->transfertablenodeid = SoNode::getNextNodeId();
  }

  static uint8_t PREDEFGRADIENTS[SoTransferFunction::SEISMIC + 1][256][4];
  static void initPredefGradients(void);

  int opaquethresholds[2];

  // The simple idea for speeding up transfer of volume data is to
  // dynamically fill in an index array, so each transfer value
  // calcualtion is done only once.
  void blankoutTransferTable(void);
  uint32_t transfertable[256];
  SbBool transferdone[256];
  uint32_t transfertablenodeid;

private:
  SoTransferFunction * master;
};

uint8_t SoTransferFunctionP::PREDEFGRADIENTS[SoTransferFunction::SEISMIC + 1][256][4];

void
SoTransferFunctionP::blankoutTransferTable(void)
{
  for (unsigned int i=0; i < 256; i++) { this->transferdone[i] = FALSE; }
}

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
  SO_ENABLE(SoCallbackAction, SoTransferFunctionElement);
  SO_ENABLE(SoPickAction, SoTransferFunctionElement);

  SoTransferFunctionP::initPredefGradients();
}

void
SoTransferFunction::doAction(SoAction * action)
{
  SoTransferFunctionElement::setTransferFunction(action->getState(), this, this);
}

void
SoTransferFunction::GLRender(SoGLRenderAction * action)
{
  this->doAction(action);
}

void
SoTransferFunction::callback(SoCallbackAction * action)
{
  this->doAction(action);
}

void
SoTransferFunction::pick(SoPickAction * action)
{
  this->doAction(action);
}

static inline uint32_t
compileRGBALittleEndian(const uint8_t red, const uint8_t green,
                        const uint8_t blue, const uint8_t opaqueness)
{
  return
    (uint32_t(red) << 0) | (uint32_t(green) << 8) |
    (uint32_t(blue) << 16) | (uint32_t(opaqueness) << 24);
}

static inline uint32_t
compileRGBABigEndian(const uint8_t red, const uint8_t green,
                     const uint8_t blue, const uint8_t opaqueness)
{
  return
    (uint32_t(red) << 24) | (uint32_t(green) << 16) |
    (uint32_t(blue) << 8) | (uint32_t(opaqueness) << 0);
}

// Transfers voxel data from input buffer to output buffer, according
// to specified parameters.
//
// Output buffer is allocated within the function.
//
// The "invisible" flag will be set according to whether or not
// there's at least one texel that's not fully transparent.
uint32_t *
SoTransferFunction::transfer(const uint8_t * input, 
                             SoVolumeData::DataType inputdatatype,
                             const SbVec2s & size,
                             SbBool & invisible) const
{
  // FIXME: about the "invisible" flag: this should really be an
  // SbBox2s that indicates which part of the output buffer is
  // non-fully-transparent. This could be used to optimize texture
  // rendering. 20021201 mortene.

  // FIXME: in addition to the "invisible" area, we could keep an
  // "opaqueness" area, to make it possible to optimize rendering by
  // occlusion culling. 20021201 mortene.


  // This table needs to be invalidated when any parameter of the
  // SoTransferFunction node changes.
  if (PRIVATE(this)->transfertablenodeid != this->getNodeId()) {
    PRIVATE(this)->blankoutTransferTable();
    PRIVATE(this)->transfertablenodeid = this->getNodeId();
  }

  assert(inputdatatype == SoVolumeData::RGBA ||
         inputdatatype == SoVolumeData::UNSIGNED_BYTE ||
         inputdatatype == SoVolumeData::UNSIGNED_SHORT);

  static int endianness = COIN_HOST_IS_UNKNOWNENDIAN;
  if (endianness == COIN_HOST_IS_UNKNOWNENDIAN) {
    endianness = coin_host_get_endianness();
    assert(endianness != COIN_HOST_IS_UNKNOWNENDIAN && "weird hardware!");
  }

  const unsigned int nrelements = size[0]*size[1];

  uint32_t * output = new uint32_t[nrelements];

  invisible = TRUE;

  // Handling RGBA inputdata. Just forwarding to output
  if (inputdatatype == SoVolumeData::RGBA) {
    (void)memcpy(output, input, nrelements * sizeof(uint32_t));
    // FIXME: set the "invisible" flag correctly according to actual
    // input. 20021129 mortene.
    invisible = FALSE;
  }

  else if (inputdatatype == SoVolumeData::UNSIGNED_BYTE) {
    int colormaptype = this->colorMapType.getValue();
    int nrcomponents = 1;
    switch (colormaptype) {
    case ALPHA: nrcomponents = 1; break;
    case LUM_ALPHA: nrcomponents = 2; break;
    case RGBA: nrcomponents = 4; break;
    default: assert(FALSE && "invalid SoTransferFunction::colorMapType value"); break;
    }

    const float * colormap = this->colorMap.getValues(0);
    int nrcols = this->colorMap.getNum() / nrcomponents;
    // FIXME: a bit strict this, should warn instead. 20021119 mortene.
    assert((this->colorMap.getNum() % nrcomponents) == 0);

    uint8_t * predefmap = NULL;
    const int predefmapidx = this->predefColorMap.getValue();
    assert(predefmapidx >= NONE);
    assert(predefmapidx <= SEISMIC);

    if (predefmapidx != NONE) {
      predefmap = &(SoTransferFunctionP::PREDEFGRADIENTS[predefmapidx][0][0]);
    }

    int32_t shiftval = this->shift.getValue();
    int32_t offsetval = this->offset.getValue();

    for (unsigned int j=0; j < nrelements; j++) {
      const uint8_t voldataidx = input[j];

      if (PRIVATE(this)->transferdone[voldataidx]) {
        output[j] = PRIVATE(this)->transfertable[voldataidx];

        uint8_t alpha = (endianness == COIN_HOST_IS_LITTLEENDIAN) ?
          ((output[j] & 0xff000000) > 24) : (output[j] & 0x000000ff);
        invisible = invisible && (alpha == 0x00);
        continue;
      }

      uint8_t inval = voldataidx << shiftval;
      inval += offsetval;

      if ((inval == 0x00) || // FIXME: not sure about this.. 20021119 mortene.
          (inval < PRIVATE(this)->opaquethresholds[0]) ||
          (inval > PRIVATE(this)->opaquethresholds[1])) {
        output[j] = 0x00000000;
      }
      else {
        uint8_t rgba[4];

        if (predefmapidx == NONE) {
          assert(inval < nrcols);
          const float * colvals = &(colormap[inval * nrcomponents]);
#if CVR_DEBUG
          // Done for robustness. Costly, though, so should probably remove.
          for (int cvchk = 0; cvchk < nrcomponents; cvchk++) {
            assert(colvals[cvchk] >= 0.0f && colvals[cvchk] <= 1.0f);
          }
#endif // CVR_DEBUG
          switch (colormaptype) {
          case ALPHA:
            rgba[0] = rgba[1] = rgba[2] = rgba[3] = uint8_t(colvals[0] * 255.0f);
            break;

          case LUM_ALPHA:
            rgba[0] = rgba[1] = rgba[2] = uint8_t(colvals[0] * 255.0f);
            rgba[3] = uint8_t(colvals[1] * 255.0f);
            break;

          case RGBA:
            rgba[0] = uint8_t(colvals[0] * 255.0f);
            rgba[1] = uint8_t(colvals[1] * 255.0f);
            rgba[2] = uint8_t(colvals[2] * 255.0f);
            rgba[3] = uint8_t(colvals[3] * 255.0f);
            break;

          default:
            assert(FALSE && "impossible");
            break;
          }
        }
        else { // using one of the predefined colormaps
          rgba[0] = predefmap[inval * 4 + 0];
          rgba[1] = predefmap[inval * 4 + 1];
          rgba[2] = predefmap[inval * 4 + 2];
          rgba[3] = predefmap[inval * 4 + 3];
        }

        output[j] = (endianness == COIN_HOST_IS_LITTLEENDIAN) ?
          compileRGBALittleEndian(rgba[0], rgba[1], rgba[2], rgba[3]) :
          compileRGBABigEndian(rgba[0], rgba[1], rgba[2], rgba[3]);

        invisible = invisible && (rgba[3] == 0x00);
      }
      
      PRIVATE(this)->transferdone[voldataidx] = TRUE;
      PRIVATE(this)->transfertable[voldataidx] = output[j];
    }
  }

  else if (inputdatatype == SoVolumeData::UNSIGNED_SHORT) {
    assert(FALSE && "not yet implemented");

    // FIXME: this is of course completely wrong -- the data should be
    // handled according to the SoTransferFunction settings. 20021112 mortene.

    // FIXME: set the "invisible" flag correctly according to actual
    // input. 20021129 mortene.
    invisible = FALSE;

    uint32_t * outp = (uint32_t *)output;
    const uint16_t * inp = (const uint16_t *)input;

    if (endianness == COIN_HOST_IS_LITTLEENDIAN) {
      for (unsigned int j=0; j < nrelements; j++) {
        outp[j] =
          (0 << 0) | // red
          (uint32_t(inp[j] & 0x00ff) << 8) | // green
          (uint32_t((inp[j] >> 8) & 0x00ff) << 16) | // blue
          ((inp[j] ? 0xff : 0) << 24); // alpha
      }
    }
    else {
      for (unsigned int j=0; j < nrelements; j++) {
        outp[j] =
          (0 << 24) | // red
          (uint32_t(inp[j] & 0x00ff) << 16) | // green
          (uint32_t((inp[j] >> 8) & 0x00ff) << 8) | // blue
          ((inp[j] ? 0xff : 0) << 0); // alpha
      }
    }
  }

  return output;
}

void
SoTransferFunction::reMap(int low, int high)
{
  assert(low <= high);

  PRIVATE(this)->opaquethresholds[0] = low;
  PRIVATE(this)->opaquethresholds[1] = high;
  
  // This is done to update our node-id, which should automatically
  // invalidate any 2D texture slices or 3D textures generated with
  // the previous colormap transfer. Also, our internal index tables
  // for speeding up transfers needs to be invalidated when these
  // values changes.
  //
  // These settings should really have been made public as fields, and
  // this would have been unnecessary. We can't do that without
  // breaking compatibility with TGS's VolumeViz, though.
  this->touch();
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
    CvrGIMPGradient * gg = CvrGIMPGradient::read(gradientbufs[j]);
    gg->convertToIntArray(SoTransferFunctionP::PREDEFGRADIENTS[j+1]);

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
