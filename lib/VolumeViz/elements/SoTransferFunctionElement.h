/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/

#ifndef COIN_SOTRANSFERFUNCTIONELEMENT_H
#define COIN_SOTRANSFERFUNCTIONELEMENT_H

#include <Inventor/elements/SoReplacedElement.h>
#include <Inventor/SbVec3f.h>
#include <Inventor/SbVec4f.h>

class SoTransferFunction;

class SoTransferFunctionElement : public SoReplacedElement {
  typedef SoReplacedElement inherited;

  SO_ELEMENT_HEADER(SoTransferFunctionElement);
public:
  static void initClass(void);
protected:
  virtual ~SoTransferFunctionElement();

public:
  virtual void init(SoState * state);
  static void setTransferFunction(SoState * const state, 
                                  SoNode * const node,
                                  SoTransferFunction * transferFunction);

  SoTransferFunction * getTransferFunction() const;
  static const SoTransferFunctionElement * 
    getInstance(SoState * const state);

  virtual void print(FILE * file) const;

protected:
  SoTransferFunction * transferFunction;

private:
  static void clean(void);
};

#endif // !COIN_SOTRANSFERFUNCTIONELEMENT_H
