#ifndef COIN_SOVOLUMEDATAELEMENT_H
#define COIN_SOVOLUMEDATAELEMENT_H

#include <Inventor/elements/SoReplacedElement.h>

class SoVolumeData;


class SoVolumeDataElement : public SoReplacedElement {
  typedef SoReplacedElement inherited;
  SO_ELEMENT_HEADER(SoVolumeDataElement);

public:
  static void initClass(void);

  virtual void init(SoState * state);
  static void setVolumeData(SoState * const state, SoNode * const node,
                            SoVolumeData * volumeData);

  SoVolumeData * getVolumeData(void) const;
  static const SoVolumeDataElement * getInstance(SoState * const state);

  virtual void print(FILE * file) const;

protected:
  virtual ~SoVolumeDataElement();
  SoVolumeData * volumeData;

private:
  static void clean(void);
};

#endif // !COIN_SOVOLUMEDATAELEMENT_H
