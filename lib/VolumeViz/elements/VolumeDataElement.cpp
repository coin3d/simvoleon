/*!
  \class SoVolumeDataElement VolumeViz/elements/SoVolumeDataElement.h
  \brief This class stores a reference to the current set of volume data.
  \ingroup volviz
*/

#include <VolumeViz/elements/SoVolumeDataElement.h>
#include <Inventor/nodes/SoNode.h>
#include <assert.h>

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
  this->volumeData = NULL;
}

void
SoVolumeDataElement::setVolumeData(SoState * const state, SoNode * const node,
                                   SoVolumeData * newvoldata)
{
  SoVolumeDataElement * elem = (SoVolumeDataElement *)
    SoElement::getElement(state, SoVolumeDataElement::classStackIndex);

  if (elem) {
    elem->volumeData = newvoldata;
  }
}


SoVolumeData *
SoVolumeDataElement::getVolumeData(void) const
{
  return this->volumeData;
}


const SoVolumeDataElement *
SoVolumeDataElement::getInstance(SoState * const state)
{
  return (const SoVolumeDataElement *)
    SoVolumeDataElement::getConstElement(state,
                                         SoVolumeDataElement::classStackIndex);
}


void
SoVolumeDataElement::print(FILE * file) const
{
  // FIXME: stub. 20021106 mortene.
}
