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
  \class SoTransferFunction VolumeViz/nodes/SoTransferFunction.h
  \brief Contains the transfer function definition.

  This node sets up the mapping from voxel data values to actual
  on-screen color values and transparency / opaqueness.

  It has a set of pre-defined color maps, commonly used in
  e.g. seismic visualization, and the option to set up one's own
  free-form color map lookup table (by setting
  SoTransferFunction::predefColorMap to NONE).
*/

// *************************************************************************

#include <VolumeViz/nodes/SoTransferFunction.h>

#include <string.h>
#include <Inventor/C/tidbits.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoCallbackAction.h>
#include <Inventor/actions/SoPickAction.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/system/gl.h>
#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <VolumeViz/misc/CvrVoxelChunk.h>
#include <VolumeViz/render/common/CvrTextureObject.h>

// *************************************************************************

SO_NODE_SOURCE(SoTransferFunction);

// *************************************************************************

class SoTransferFunctionP {
public:
  SoTransferFunctionP(SoTransferFunction * master) {
    this->master = master;
  }

private:
  SoTransferFunction * master;
};

#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

// *************************************************************************

/*!
  \enum SoTransferFunction::PredefColorMap
  Predefined color transfer functions, each containing exactly 256 colors.
*/
/*!
  \var SoTransferFunction::PredefColorMap SoTransferFunction::NONE
  If this is set, the SoTransferFunction::colorMap field must be used.
*/

/*!
  \var SoTransferFunction::PredefColorMap SoTransferFunction::GREY

  Default transfer function color map. The checker board indicates
  that this gradient is partly transparent.

  <center><img src="http://doc.coin3d.org/images/SIMVoleon/gradients/grey.png"></center>
*/
/*!
  \var SoTransferFunction::PredefColorMap SoTransferFunction::GRAY
  Same as GREY.
*/
/*!
  \var SoTransferFunction::PredefColorMap SoTransferFunction::TEMPERATURE

  This gradient is fully opaque.

  <center><img src="http://doc.coin3d.org/images/SIMVoleon/gradients/temperature.png"></center>
*/
/*!
  \var SoTransferFunction::PredefColorMap SoTransferFunction::PHYSICS

  This gradient is fully opaque.

  <center><img src="http://doc.coin3d.org/images/SIMVoleon/gradients/physics.png"></center>
*/
/*!
  \var SoTransferFunction::PredefColorMap SoTransferFunction::STANDARD

  This gradient is fully opaque.

  <center><img src="http://doc.coin3d.org/images/SIMVoleon/gradients/standard.png"></center>
*/
/*!
  \var SoTransferFunction::PredefColorMap SoTransferFunction::GLOW

  This gradient is fully opaque.

  <center><img src="http://doc.coin3d.org/images/SIMVoleon/gradients/glow.png"></center>
*/
/*!
  \var SoTransferFunction::PredefColorMap SoTransferFunction::BLUE_RED

  This gradient is fully opaque.

  <center><img src="http://doc.coin3d.org/images/SIMVoleon/gradients/blue_red.png"></center>
*/
/*!
  \var SoTransferFunction::PredefColorMap SoTransferFunction::SEISMIC

  This gradient is partly transparent.

  <center><img src="http://doc.coin3d.org/images/SIMVoleon/gradients/seismic.png"></center>
*/

/*!
  \enum SoTransferFunction::ColorMapType

  Type of colormap array. Defines the possible values for the
  SoTransferFunction::colorMapType field.
*/
/*!
  \var SoTransferFunction::ColormapType SoTransferFunction::ALPHA

  Each color entry in the transfer function has only one component, so
  the SoTransferFunction::colorMap field should consist of exactly 256
  float values, each value representing a transparency value. The RGB
  colors will all be set to [1,1,1], i.e. all white.
*/
/*!
  \var SoTransferFunction::ColormapType SoTransferFunction::LUM_ALPHA
 
 Each transfer function color map entry in
 SoTransferFunction::colorMap has two components, ie. two floats, so
 the field should contain exactly 256*2 = 512 values. The first float
 value of each pair is taken to be an intensity / grey color value,
 and the second float of each pair is a transparency value (as for the
 ALPHA enum).
*/
/*!
  \var SoTransferFunction::ColormapType SoTransferFunction::RGBA

  Four floats are used to specify each color of the transfer
  function. SoTransferFunction::colorMap should consist of exactly
  256*4 = 1024 float values. Each quadruple consist of intensity
  values for red, green, and blue, and then the fourth value is an
  alpha value, as for LUM_ALPHA and ALPHA.
*/

/*!
  \var SoSFInt32 SoTransferFunction::offset

  See the SoTransferFunction::shift field.
*/

/*!
  \var SoSFInt32 SoTransferFunction::shift

  Used to shift the voxel attribute value before assigning a color
  value from the transfer function color map. In pseudo-code:

  \code
  colorvalue = transferfunction[(voxelvalue << shift) + offset]
  \endcode

  (\c offset is the value of the SoTransferFunction::offset field.)
*/

/*!
  \var SoSFEnum SoTransferFunction::colorMapType

  This field specifies how the SoTransferFunction::colorMap field
  should be interpreted when SoTransferFunction::predefColorMap is set
  to SoTransferFunction::NONE.

  (Note that it will \e not have any effect when
  SoTransferFunction::predefColorMap is set to any of the actual
  pre-defined transfer function color maps, i.e. not equal to \c
  NONE.)

  See SoTransferFunction::ColorMapType for what possible values this
  field can have, and their semantics.
*/

/*!
  \var SoSFEnum SoTransferFunction::predefColorMap

  Sets up the transfer function to use. See
  SoTransferFunction::PredefColorMap for a list of the pre-defined
  color maps.

  Note that most of the pre-defined color maps are all opaque, even
  for data value 0. This will often cause the unwanted behavior that
  the full data set will be completely opaque for visualization, so
  only the outer "walls" of the volume are shown. To avoid this, use
  the SoTransferFunction::reMap() method to "narrow" the data values
  that are to be rendered according to the transfer function (all
  values outside of the given range will then be fully transparent).

  The value SoTransferFunction::NONE has special meaning: it signifies
  that none of the pre-defined values should be used, but that the
  transfer function color map should be fetched from the
  SoTransferFunction::colorMap field instead.
*/

/*!
  \var SoMFFloat SoTransferFunction::colorMap

  An array of floats describing the transfer function. Each value must
  be normalized to be within [0.0, 1.0] for the intensity value of a
  color, or the alpha value for transparency.

  The array must contain 256 colors. The number of floats needed in
  the array for each color depends on the
  SoTransferFunction::ColorMapType setting.
*/

// FIXME: the colorMap field shouldn't have to contain exactly 256
// colors. This is mentioned several times in the API docs above, so
// when this restriction is lifted, make sure all of them are
// updated. 20040923 mortene.

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

  // These were added to make it possible to control the narrowing of
  // the transfer function from the iv-file. They provide the same
  // functionality as the reMap() function.
  //
  // These are "unofficial", private fields, as they are not available
  // in TGS VolumeViz, which we want to stay compatible with.
  //
  // Init to lowest and highest uint16_t values.
  SO_NODE_ADD_FIELD(remapLow, (0));
  SO_NODE_ADD_FIELD(remapHigh, ((2 << 16) - 1));
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
}

void
SoTransferFunction::doAction(SoAction * action)
{
  SoState * s = action->getState();

  SoTransferFunctionElement::setTransferFunction(s, this);

  const uint32_t low = this->remapLow.getValue();
  const uint32_t high = this->remapHigh.getValue();
  SoTransferFunctionElement::setTransparencyThresholds(s, low, high);
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

/*!
  Set two thresholds, where all transfer function mappings that ends
  up below the \a low value or above the \a high value are set to be
  completely transparent.

  Initial default values are [0, 65535].
 */
void
SoTransferFunction::reMap(int low, int high)
{
  assert(low <= high);
  assert(low >= 0);
  assert(high >= 0);

  const uint32_t l = (uint32_t)low;
  const uint32_t h = (uint32_t)high;

  if ((l == this->remapLow.getValue()) && (h == this->remapHigh.getValue())) {
    // No change.
    return;
  }

  // This will cause an update to the SoTransferFunction instance's
  // node-id, which should automatically invalidate any 2D texture
  // slices or 3D textures generated with the previous colormap
  // transfer.
  this->remapLow = l;
  this->remapHigh = h;
}
