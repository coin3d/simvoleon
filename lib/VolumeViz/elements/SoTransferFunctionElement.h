#ifndef COIN_SOTRANSFERFUNCTIONELEMENT_H
#define COIN_SOTRANSFERFUNCTIONELEMENT_H

#include <Inventor/elements/SoReplacedElement.h>
#include <VolumeViz/C/basic.h>

class SoTransferFunction;


class SIMVOLEON_DLL_API SoTransferFunctionElement : public SoReplacedElement {
  typedef SoReplacedElement inherited;
  SO_ELEMENT_HEADER(SoTransferFunctionElement);

public:
  static void initClass(void);
  virtual void init(SoState * state);


  static void setTransferFunction(SoState * const state,
                                  SoTransferFunction * node);
  SoTransferFunction * getTransferFunction(void) const;


  static void setTransparencyThresholds(SoState * const state,
                                        uint32_t low, uint32_t high);
  void getTransparencyThresholds(uint32_t & low, uint32_t & high) const;


  static const SoTransferFunctionElement * getInstance(SoState * const state);


protected:
  virtual ~SoTransferFunctionElement();

private:
  SoTransferFunction * transferfunction;
  uint32_t transpthreshold[2];
};

#endif // !COIN_SOTRANSFERFUNCTIONELEMENT_H
