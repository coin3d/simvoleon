#ifndef COIN_SOTRANSFERFUNCTIONELEMENT_H
#define COIN_SOTRANSFERFUNCTIONELEMENT_H

#include <Inventor/elements/SoReplacedElement.h>

class SoTransferFunction;


class SoTransferFunctionElement : public SoReplacedElement {
  typedef SoReplacedElement inherited;
  SO_ELEMENT_HEADER(SoTransferFunctionElement);

public:
  static void initClass(void);
  virtual void init(SoState * state);

  static void setTransferFunction(SoState * const state, SoNode * const node,
                                  SoTransferFunction * transferFunction);

  SoTransferFunction * getTransferFunction(void) const;
  static const SoTransferFunctionElement * getInstance(SoState * const state);

  virtual void print(FILE * file) const;

protected:
  virtual ~SoTransferFunctionElement();
  SoTransferFunction * transferFunction;

private:
  static void clean(void);
};

#endif // !COIN_SOTRANSFERFUNCTIONELEMENT_H
