/*!
  \class SoVolumeDataElement VolumeViz/elements/SoVolumeDataElement.h
  \brief This class stores a reference to the current set of volume data.
  \ingroup volviz
*/

#include <assert.h>

#include <Inventor/SbBox3f.h>
#include <Inventor/errors/SoDebugError.h>

#include <VolumeViz/elements/SoVolumeDataElement.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/misc/CvrUtil.h>


SO_ELEMENT_SOURCE(SoVolumeDataElement);


void
SoVolumeDataElement::initClass(void)
{
  SO_ELEMENT_INIT_CLASS(SoVolumeDataElement, inherited);
}


SoVolumeDataElement::~SoVolumeDataElement(void)
{
}

void
SoVolumeDataElement::init(SoState * state)
{
  inherited::init(state);
  this->nodeptr = NULL;
}

void
SoVolumeDataElement::setVolumeData(SoState * const state, SoNode * const node,
                                   SoVolumeData * newvoldata)
{
  SoVolumeDataElement * elem = (SoVolumeDataElement *)
    SoElement::getElement(state, SoVolumeDataElement::classStackIndex);

  if (elem) { elem->nodeptr = newvoldata; }
}


SoVolumeData *
SoVolumeDataElement::getVolumeData(void) const
{
  return this->nodeptr;
}

const SoVolumeDataElement *
SoVolumeDataElement::getInstance(SoState * const state)
{
  return (const SoVolumeDataElement *)
    SoVolumeDataElement::getConstElement(state,
                                         SoVolumeDataElement::classStackIndex);
}

const SbVec3s
SoVolumeDataElement::getVoxelCubeDimensions(void) const
{
  SbVec3s voxeldims;
  void * discardptr;
  SoVolumeData::DataType discard;
  SbBool ok = this->nodeptr->getVolumeData(voxeldims, discardptr, discard);
  assert(ok);
  return voxeldims;
}

SoVolumeData::DataType
SoVolumeDataElement::getVoxelDataType(void) const
{
  SbVec3s discard;
  void * discardptr;
  SoVolumeData::DataType type;
  SbBool ok = this->nodeptr->getVolumeData(discard, discardptr, type);
  assert(ok);
  return type;
}

SbVec3s
SoVolumeDataElement::objectCoordsToIJK(const SbVec3f & objectpos) const
{
  SbVec3s voxeldims;
  void * voxelptr;
  SoVolumeData::DataType voxeltype;
  SbBool ok = this->nodeptr->getVolumeData(voxeldims, voxelptr, voxeltype);
  assert(ok);

  const SbBox3f volsize = this->nodeptr->getVolumeSize();
  const SbVec3f & mincorner = volsize.getMin();
  const SbVec3f size = volsize.getMax() - mincorner;

  SbVec3s ijk;
  for (int i=0; i < 3; i++) {
    const float normcoord = (objectpos[i] - mincorner[i]) / size[i];
    ijk[i] = (short)(normcoord * voxeldims[i]);
  }

  if (CvrUtil::debugRayPicks()) {
    SoDebugError::postInfo("SoVolumeDataElement::objectCoordsToIJK",
                           "objectpos==<%f, %f, %f>, volumesize== <%f, %f, %f>, "
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

void
SoVolumeDataElement::getPageGeometry(const int axis, const int slicenr,
                                     SbVec3f & origo,
                                     SbVec3f & horizspan,
                                     SbVec3f & verticalspan) const
{
  SbBox3f spacesize = this->nodeptr->getVolumeSize();
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

  SbVec3s dimensions = this->getVoxelCubeDimensions();
 
  const float depthprslice = (spacemax[axis] - spacemin[axis]) / dimensions[axis];
  const float depth = spacemin[axis] + slicenr * depthprslice;

  // "origo" should point at the upper left corner of the page to
  // render. horizspan should point rightwards, and verticalspan
  // downwards.

  switch (axis) {
  case 0: origo = SbVec3f(depth, qmax[1], qmin[0]); break;
  case 1: origo = SbVec3f(qmin[0], depth, qmin[1]); break;
  case 2: origo = SbVec3f(qmin[0], qmax[1], depth); break;
  default: assert(FALSE); break;
  }

  const float width = qmax[0] - qmin[0];
  const float height = qmax[1] - qmin[1];

  switch (axis) {
  case 0:
    horizspan = SbVec3f(0, 0, width);
    verticalspan = SbVec3f(0, -height, 0);
    break;
  case 1:
    // The last component is "flipped" to make the y-direction slices
    // not come out upside-down. FIXME: should really investigate if
    // this is the correct fix. 20021124 mortene.
    horizspan = SbVec3f(width, 0, 0);
    verticalspan = SbVec3f(0, 0, height);
    break;
  case 2:
    horizspan = SbVec3f(width, 0, 0);
    verticalspan = SbVec3f(0, -height, 0);
    break;
  default: assert(FALSE); break;
  }

  const SbVec3f SCALE((spacemax[0] - spacemin[0]) / dimensions[0],
                      (spacemax[1] - spacemin[1]) / dimensions[1],
                      (spacemax[2] - spacemin[2]) / dimensions[2]);

  const SbVec2f QUADSCALE = (axis == 2) ?
    SbVec2f(SCALE[0], SCALE[1]) :
    ((axis == 0) ?
     SbVec2f(SCALE[2], SCALE[1]) :
     SbVec2f(SCALE[0], SCALE[2]));

  horizspan.normalize();
  horizspan *= QUADSCALE[0];
  verticalspan.normalize();
  verticalspan *= QUADSCALE[1];

}
