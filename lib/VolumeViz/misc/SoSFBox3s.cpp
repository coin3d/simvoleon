/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/

/*!
  \class SoSFBox3s SoSFBox3s.h Inventor/fields/SoSFBox3s.h
  \brief The SoSFBox3s class is a container for an SbBox3s vector.
  \ingroup fields

  This field is used where nodes, engines or other field containers
  needs to store a single vector with two elements.

  \since 2002-01-25

*/

#include <VolumeViz/misc/SoSFBox3s.h>
#include <Inventor/fields/SoSubField.h>
#include <Inventor/SoInput.h>
#include <Inventor/SoOutput.h>
#include <Inventor/errors/SoReadError.h>
#if COIN_DEBUG
#include <Inventor/errors/SoDebugError.h>
#endif // COIN_DEBUG





SO_SFIELD_SOURCE(SoSFBox3s, SbBox3s, SbBox3s);


// Override from parent class.
void
SoSFBox3s::initClass(void)
{
  SO_SFIELD_INIT_CLASS(SoSFBox3s, SoSField);
}

// No need to document readValue() and writeValue() here, as the
// necessary information is provided by the documentation of the
// parent classes.
#ifndef DOXYGEN_SKIP_THIS


SbBool
SoSFBox3s::readValue(SoInput * in)
{
  float min[3];
  float max[3];
  if (!in->read(min[0]) || 
      !in->read(min[1]) || 
      !in->read(min[2]) || 
      !in->read(max[0]) || 
      !in->read(max[1]) || 
      !in->read(max[2])) {
    SoReadError::post(in, "Couldn't read vector");
    return FALSE;
  }
  this->setValue(min[0], min[1], min[2], max[0], max[1], max[2]);
  return TRUE;
}

void
SoSFBox3s::writeValue(SoOutput * out) const
{
  short min[3];
  short max[3];
  SbBox3s b = this->getValue();
  b.getBounds(min[0], min[1], min[2], max[0], max[1], max[2]);
  out->write(min[0]);
  out->write(min[1]);
  out->write(min[2]);
  out->write(max[0]);
  out->write(max[1]);
  out->write(max[2]);
}

#endif // DOXYGEN_SKIP_THIS


/*!
  Set value of vector.
*/
void
SoSFBox3s::setValue(short xmin, short ymin, short zmin, short xmax, short ymax, short zmax)
{
  this->setValue(SbBox3s(xmin, ymin, zmin, xmax, ymax, zmax));
}


/*!
  Set value of vector.
*/
void
SoSFBox3s::setValue(const SbVec3s & minvec, const SbVec3s & maxvec)
{
  this->setValue(SbBox3s(minvec, maxvec));
}


/*!
  Set value of vector.
*/
void
SoSFBox3s::getValue(SbBox3s &box)
{
  box = value;
}

