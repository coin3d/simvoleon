#include "TransferFunctionHandler.h"
#include "SoTransferFunction_ctrl.h"

#include <VolumeViz/nodes/SoTransferFunction.h>
#include <qvalidator.h>
#include <qlineedit.h>
#include <qcombobox.h>


TransferFunctionHandler::TransferFunctionHandler(SoTransferFunction * node,
                                                 QWidget * parent)
{
  this->node = node;

  // FIXME: should better place a sensor on it and detect when the
  // node is dying. 20021211 mortene.
  this->node->ref();

  this->ctrl = new SoTransferFunction_ctrl(parent);
  this->ctrl->show();

  this->remap[0] = 0;
  this->remap[1] = 255;

  this->initGUI();
}

TransferFunctionHandler::~TransferFunctionHandler()
{
  this->node->unref();
}

void
TransferFunctionHandler::initGUI(void)
{
  // "reMap" lineedits

  QIntValidator * v = new QIntValidator(0, 255, new QObject);

  this->ctrl->lowEdit->setValidator(v);
  this->ctrl->highEdit->setValidator(v);

  QString s;
  s.sprintf("%d", this->remap[0]);
  this->ctrl->lowEdit->setText(s);
  s.sprintf("%d", this->remap[1]);
  this->ctrl->highEdit->setText(s);

  QObject::connect(this->ctrl->lowEdit, SIGNAL(returnPressed()),
                   this, SLOT(lowEditUpdate()));

  QObject::connect(this->ctrl->highEdit, SIGNAL(returnPressed()),
                   this, SLOT(highEditUpdate()));


  // predefColorMap combobox

  this->ctrl->predefCombo->setCurrentItem(this->node->predefColorMap.getValue());

  QObject::connect(this->ctrl->predefCombo, SIGNAL(activated(int)),
                   this, SLOT(predefColorMapUpdate(int)));


  // "shift" & "offset" lineedits

  v = new QIntValidator(-32767, 32767, new QObject);

  this->ctrl->shiftEdit->setValidator(v);
  this->ctrl->offsetEdit->setValidator(v);

  s;
  s.sprintf("%d", this->node->shift.getValue());
  this->ctrl->shiftEdit->setText(s);
  s.sprintf("%d", this->node->offset.getValue());
  this->ctrl->offsetEdit->setText(s);

  QObject::connect(this->ctrl->shiftEdit, SIGNAL(returnPressed()),
                   this, SLOT(shiftEditUpdate()));

  QObject::connect(this->ctrl->offsetEdit, SIGNAL(returnPressed()),
                   this, SLOT(offsetEditUpdate()));

}

void
TransferFunctionHandler::lowEditUpdate(void)
{
  this->remap[0] = this->ctrl->lowEdit->text().toInt();
  assert(this->remap[0] <= this->remap[1] && "must be <= the high value");

  this->node->reMap(this->remap[0], this->remap[1]);
}

void
TransferFunctionHandler::highEditUpdate(void)
{
  this->remap[1] = this->ctrl->highEdit->text().toInt();
  assert(this->remap[1] >= this->remap[0] && "must be >= the low value");

  this->node->reMap(this->remap[0], this->remap[1]);
}

void
TransferFunctionHandler::predefColorMapUpdate(int idx)
{
  this->node->predefColorMap = idx;
}

void
TransferFunctionHandler::shiftEditUpdate(void)
{
  this->node->shift = this->ctrl->shiftEdit->text().toInt();
}

void
TransferFunctionHandler::offsetEditUpdate(void)
{
  this->node->offset = this->ctrl->offsetEdit->text().toInt();
}
