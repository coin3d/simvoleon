#include "OrthoSliceHandler.h"
#include "SoOrthoSlice_ctrl.h"

#include <VolumeViz/nodes/SoOrthoSlice.h>
#include <qvalidator.h>
#include <qlineedit.h>
#include <qcombobox.h>


OrthoSliceHandler::OrthoSliceHandler(SoOrthoSlice * node, QWidget * parent)
{
  this->node = node;

  // FIXME: should better place a sensor on it and detect when the
  // node is dying. 20021211 mortene.
  this->node->ref();

  this->ctrl = new SoOrthoSlice_ctrl(parent);
  this->ctrl->show();

  this->initGUI();
}

OrthoSliceHandler::~OrthoSliceHandler()
{
  this->node->unref();
}

void
OrthoSliceHandler::initGUI(void)
{
//   // predefColorMap combobox

//   this->ctrl->predefCombo->setCurrentItem(this->node->predefColorMap.getValue());

//   QObject::connect(this->ctrl->predefCombo, SIGNAL(activated(int)),
//                    this, SLOT(predefColorMapUpdate(int)));


//   // "shift" & "offset" lineedits

//   v = new QIntValidator(-32767, 32767, new QObject);

//   this->ctrl->shiftEdit->setValidator(v);
//   this->ctrl->offsetEdit->setValidator(v);

//   s;
//   s.sprintf("%d", this->node->shift.getValue());
//   this->ctrl->shiftEdit->setText(s);
//   s.sprintf("%d", this->node->offset.getValue());
//   this->ctrl->offsetEdit->setText(s);

//   QObject::connect(this->ctrl->shiftEdit, SIGNAL(returnPressed()),
//                    this, SLOT(shiftEditUpdate()));

//   QObject::connect(this->ctrl->offsetEdit, SIGNAL(returnPressed()),
//                    this, SLOT(offsetEditUpdate()));

}

// void
// OrthoSliceHandler::predefColorMapUpdate(int idx)
// {
//   this->node->predefColorMap = idx;
// }

// void
// OrthoSliceHandler::shiftEditUpdate(void)
// {
//   this->node->shift = this->ctrl->shiftEdit->text().toInt();
// }

// void
// OrthoSliceHandler::offsetEditUpdate(void)
// {
//   this->node->offset = this->ctrl->offsetEdit->text().toInt();
// }
