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
 *  Systems in Motion, Postboks 1283, Pirsenteret, 7462 Trondheim, NORWAY.
 *  <URL:http://www.sim.no/>.
 *
\**************************************************************************/

#include <VolumeViz/elements/CvrVoxelBlockElement.h>

#include <assert.h>

#include <Inventor/SbBox2f.h>
#include <Inventor/SbVec2f.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/nodes/SoNode.h>

#include <VolumeViz/misc/CvrUtil.h>

// *************************************************************************

SO_ELEMENT_SOURCE(CvrVoxelBlockElement);

// *************************************************************************

void
CvrVoxelBlockElement::initClass(void)
{
  SO_ELEMENT_INIT_CLASS(CvrVoxelBlockElement, inherited);
}


CvrVoxelBlockElement::~CvrVoxelBlockElement(void)
{
}

void
CvrVoxelBlockElement::init(SoState * state)
{
  inherited::init(state);

  // Default values.
  this->bytesprvoxel = 1;
  this->voxelcubedims.setValue(0, 0, 0);
  this->voxels = NULL;
}

const CvrVoxelBlockElement *
CvrVoxelBlockElement::getInstance(SoState * const state)
{
  return (const CvrVoxelBlockElement *)
    CvrVoxelBlockElement::getConstElement(state,
                                          CvrVoxelBlockElement::classStackIndex);
}

// *************************************************************************

SbBool
CvrVoxelBlockElement::matches(const SoElement * element) const
{
  const CvrVoxelBlockElement * elem = (const CvrVoxelBlockElement *)element;

  return
    inherited::matches(element) &&
    elem->nodeId == this->nodeId &&
    elem->bytesprvoxel == this->bytesprvoxel &&
    elem->voxelcubedims == this->voxelcubedims &&
    elem->voxels == this->voxels &&
    elem->unitdimensionsbox == this->unitdimensionsbox;
}

SoElement *
CvrVoxelBlockElement::copyMatchInfo(void) const
{
  assert(this->getTypeId().canCreateInstance());

  CvrVoxelBlockElement * element = (CvrVoxelBlockElement *)
    this->getTypeId().createInstance();

  *element = *this;

  return element;
}

// *************************************************************************

void
CvrVoxelBlockElement::set(SoState * state, SoNode * node,
                          unsigned int bytesprvoxel,
                          const SbVec3s & voxelcubedims,
                          const uint8_t * voxels,
                          const SbBox3f & unitdimensionsbox)
{
  CvrVoxelBlockElement * elem = (CvrVoxelBlockElement *)
    SoElement::getElement(state, CvrVoxelBlockElement::classStackIndex);
  assert(elem);

  elem->nodeId = node->getNodeId();
  elem->bytesprvoxel = bytesprvoxel;
  elem->voxelcubedims = voxelcubedims;
  elem->voxels = voxels;
  elem->unitdimensionsbox = unitdimensionsbox;
}

// *************************************************************************

unsigned int
CvrVoxelBlockElement::getBytesPrVoxel(void) const
{
  return this->bytesprvoxel;
}

const SbVec3s &
CvrVoxelBlockElement::getVoxelCubeDimensions(void) const
{
  return this->voxelcubedims;
}

const uint8_t *
CvrVoxelBlockElement::getVoxels(void) const
{
  return this->voxels;
}

const SbBox3f &
CvrVoxelBlockElement::getUnitDimensionsBox(void) const
{
  return this->unitdimensionsbox;
}

// *************************************************************************

SbVec3s
CvrVoxelBlockElement::objectCoordsToIJK(const SbVec3f & objectpos) const
{
  const SbVec3s & voxeldims = this->getVoxelCubeDimensions();

  const SbBox3f & volsize = this->getUnitDimensionsBox();
  const SbVec3f & mincorner = volsize.getMin();
  const SbVec3f size = volsize.getMax() - mincorner;

  SbVec3s ijk;
  for (int i=0; i < 3; i++) {
    const float normcoord = (objectpos[i] - mincorner[i]) / size[i];
    ijk[i] = (short)(normcoord * (voxeldims[i]-1));
  }

  if (CvrUtil::debugRayPicks()) {
    SoDebugError::postInfo("CvrVoxelBlockElement::objectCoordsToIJK",
                           "objectpos==<%f, %f, %f>, volumesize==<%f, %f, %f>, "
                           "mincorner==<%f, %f, %f>, voxeldims==<%d, %d, %d> "
                           "==> ijk==<%d, %d, %d>",
                           objectpos[0], objectpos[1], objectpos[2],
                           size[0], size[1], size[2],
                           mincorner[0], mincorner[1], mincorner[2],
                           voxeldims[0], voxeldims[1], voxeldims[2],
                           ijk[0], ijk[1], ijk[2]);
  }

  return ijk;
}

// The returned vectors will be:
//
// * origo: starting position for the next quadface, in coordinates
//   normalized to be within a unit cube
//
// * horizspan, verticalspan: normalized direction vectors which
//   defines the directions to the corners of the slice quad
void
CvrVoxelBlockElement::getPageGeometry(const int axis, const int slicenr,
                                      SbVec3f & origo,
                                      SbVec3f & horizspan,
                                      SbVec3f & verticalspan) const
{
  SbBox3f spacesize(SbVec3f(-0.5, -0.5, -0.5), SbVec3f(0.5, 0.5, 0.5));
  SbVec3f spacemin, spacemax;
  spacesize.getBounds(spacemin, spacemax);

  const SbBox2f QUAD = (axis == 2) ? // Z axis?
    SbBox2f(spacemin[0], spacemin[1], spacemax[0], spacemax[1]) :
    ((axis == 0) ? // X axis?
     SbBox2f(spacemin[2], spacemin[1], spacemax[2], spacemax[1]) :
     // then it's along Y
     SbBox2f(spacemin[0], spacemin[2], spacemax[0], spacemax[2]));

  SbVec2f qmax, qmin;
  QUAD.getBounds(qmin, qmax);

  const SbVec3s & dimensions = this->getVoxelCubeDimensions();
 
  const float depthprslice = (spacemax[axis] - spacemin[axis]) / dimensions[axis];
  const float depth = spacemin[axis] + slicenr * depthprslice + depthprslice/2.0f;

  // "origo" should point at the upper left corner of the page to
  // render. horizspan should point rightwards, and verticalspan
  // downwards.

  switch (axis) {
  case 0: origo = SbVec3f(depth, -qmax[1], qmin[0]); break;
  case 1: origo = SbVec3f(qmin[0], depth, qmin[1]); break;
  case 2: origo = SbVec3f(qmin[0], -qmax[1], depth); break;
  default: assert(FALSE); break;
  }

  const float width = qmax[0] - qmin[0];
  const float height = qmax[1] - qmin[1];

  switch (axis) {
  case 0:
    horizspan = SbVec3f(0, 0, width);
    verticalspan = SbVec3f(0, height, 0);
    break;
  case 1:
    horizspan = SbVec3f(width, 0, 0);
    verticalspan = SbVec3f(0, 0, height);
    break;
  case 2:
    horizspan = SbVec3f(width, 0, 0);
    verticalspan = SbVec3f(0, height, 0);
    break;
  default: assert(FALSE); break;
  }

  horizspan.normalize();
  verticalspan.normalize();

  // This is here to support client code which depends on an old bug:
  // data was flipped on the Y axis.
  if (CvrUtil::useFlippedYAxis()) {
    if (axis != 1) {
      verticalspan = -verticalspan;
      origo[1] = -origo[1];
    }
  }
}

// *************************************************************************

// FIXME: this function is also present in SoVolumeData.  Refactor
// to common util function. 20040719 mortene.

uint32_t
CvrVoxelBlockElement::getVoxelValue(const SbVec3s & voxelpos) const
{
  assert(voxelpos[0] < this->voxelcubedims[0]);
  assert(voxelpos[1] < this->voxelcubedims[1]);
  assert(voxelpos[2] < this->voxelcubedims[2]);

  const uint8_t * voxptr = this->voxels;

  int advance = 0;
  const unsigned int dim[3] = { // so we don't overflow a short
    this->voxelcubedims[0], this->voxelcubedims[1],  this->voxelcubedims[2]
  };
  advance += voxelpos[2] * dim[0] * dim[1];
  advance += voxelpos[1] * dim[0];
  advance += voxelpos[0];

  advance *= this->bytesprvoxel;

  voxptr += advance;

  uint32_t val = 0;
  switch (this->bytesprvoxel) {
  case 1: val = *voxptr; break;
  case 2: val = *((uint16_t *)voxptr); break;
  default: assert(FALSE); break;
  }
  return val;
}

// *************************************************************************
