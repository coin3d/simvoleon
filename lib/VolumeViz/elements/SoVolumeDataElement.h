#ifndef COIN_SOVOLUMEDATAELEMENT_H
#define COIN_SOVOLUMEDATAELEMENT_H

#include <Inventor/elements/SoReplacedElement.h>

class SoVolumeData;
class SbVec3f;


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

  void getPageGeometry(const int axis, const int slicenr,
                       SbVec3f & origo,
                       SbVec3f & horizspan,
                       SbVec3f & verticalspan) const;

protected:
  virtual ~SoVolumeDataElement();
  SoVolumeData * nodeptr;

private:
  static void clean(void);
};

#endif // !COIN_SOVOLUMEDATAELEMENT_H
