
/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/

#ifndef COIN_SOVOLUMEDATAELEMENT_H
#define COIN_SOVOLUMEDATAELEMENT_H

#include <Inventor/elements/SoReplacedElement.h>
#include <Inventor/SbVec3f.h>
#include <Inventor/SbVec4f.h>

class SoVolumeData;

class SoVolumeDataElement : public SoReplacedElement {
  typedef SoReplacedElement inherited;

  SO_ELEMENT_HEADER(SoVolumeDataElement);
public:
  static void initClass(void);
protected:
  virtual ~SoVolumeDataElement();

public:
  virtual void init(SoState * state);
  static void setVolumeData(SoState * const state, 
                            SoNode * const node,
                            SoVolumeData * volumeData);

  SoVolumeData * getVolumeData() const;
  static const SoVolumeDataElement * getInstance(SoState * const state);

  virtual void print(FILE * file) const;

protected:
  SoVolumeData * volumeData;

private:
  static void clean(void);
};

#endif // !COIN_SOVOLUMEDATAELEMENT_H
