/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/
#ifndef COIN_SOSFBOX3S_H
#define COIN_SOSFBOX3S_H

#include <Inventor/fields/SoSubField.h>
#include <Inventor/SbBox3s.h>

class SoSFBox3s : public SoSField {
  typedef SoSField inherited;

  SO_SFIELD_HEADER(SoSFBox3s, SbBox3s, SbBox3s);

public:
  static void initClass(void);

  void setValue(short xmin, short ymin, short zmin, short xmax, short ymax, short zmax);
  void setValue(const SbVec3s & minvec, const SbVec3s & maxvec);
  void getValue(SbBox3s & box);
};

#endif // !COIN_SOSFBOX3S_H
