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
  on-screen color values and transparency / opaqueness. It has a set
  of pre-defined color maps, commonly used in e.g. seismic
  visualization, and the option to set up one's own free-form color
  map lookup table.
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

    // Init to lowest and highest uint16_t values.
    this->opaquethresholds[0] = 0;
    this->opaquethresholds[1] = (2 << 16) - 1;
  }

  int opaquethresholds[2];

private:
  SoTransferFunction * master;
};

#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

// *************************************************************************

/*!
  \enum SoTransferFunction::PredefColorMap
  Predefined color transfer funcions of size 256.
*/
/*!
  \var SoTransferFunction::PredefColorMap SoTransferFunction::NONE
*/
/*!
  \var SoTransferFunction::PredefColorMap SoTransferFunction::GREY
  Default
*/
/*!
  \var SoTransferFunction::PredefColorMap SoTransferFunction::GRAY
  Same as GREY
*/
/*!
  \var SoTransferFunction::PredefColorMap SoTransferFunction::TEMPERATURE
*/
/*!
  \var SoTransferFunction::PredefColorMap SoTransferFunction::PYSICS
*/
/*!
  \var SoTransferFunction::PredefColorMap SoTransferFunction::STANDARD
*/
/*!
  \var SoTransferFunction::PredefColorMap SoTransferFunction::GLOW
*/
/*!
  \var SoTransferFunction::PredefColorMap SoTransferFunction::BLUE_RED
*/
/*!
  \var SoTransferFunction::PredefColorMap SoTransferFunction::SEISMIC
*/

/*!
  \enum SoTransferFunction::ColorMapType
  Type of colormap array.
*/
/*!
  \var SoTransferFunction::ColormapType SoTransferFunction::ALPHA
  Each color entry in the transfer function has only one component.
*/
/*!
  \var SoTransferFunction::ColormapType SoTransferFunction::LUM_ALPHA
  Each entry has to components, ie. two floats.
*/
/*!
  \var SoTransferFunction::ColormapType SoTransferFunction::RGBA
  Four floats are used to specify each color of the transfer function.
*/

/*!
  \var SoSFInt32 SoTransferFunction::offset
  See the SoTransferFunction::shift field.
*/

/*!
  \var SoSFInt32 SoTransferFunction::shift
  Used to shift the transfer function before assigning voxel value.
  \code
  voxelvalue = (datavalue << shift) + offset
  \endcode
*/

/*!
  \var SoMFFloat SoTransferFunction::colorMap
  An array of floats describing the transfer function. Each value must
  be normalized to [0..1]. The array must contain 256 colors. The
  number of floats for each color depends on the
  SoTransferFunction::ColorMapType setting.
*/

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
}

void
SoTransferFunction::doAction(SoAction * action)
{
  SoState * s = action->getState();

  SoTransferFunctionElement::setTransferFunction(s, this);
  SoTransferFunctionElement::setTransparencyThresholds(s,
                                                       PRIVATE(this)->opaquethresholds[0],
                                                       PRIVATE(this)->opaquethresholds[1]);
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

  if ((low == PRIVATE(this)->opaquethresholds[0]) &&
      (high == PRIVATE(this)->opaquethresholds[1])) {
    // No change.
    return;
  }

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
