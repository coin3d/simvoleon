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
  // "sliceNumber" lineedit

  QIntValidator * v = new QIntValidator(0, 32767, new QObject);

  this->ctrl->sliceNumberEdit->setValidator(v);

  QString s;
  s.sprintf("%d", this->node->sliceNumber.getValue());
  this->ctrl->sliceNumberEdit->setText(s);

  QObject::connect(this->ctrl->sliceNumberEdit, SIGNAL(returnPressed()),
                   this, SLOT(sliceNumberEditUpdate()));

//   // predefColorMap combobox

//   this->ctrl->predefCombo->setCurrentItem(this->node->predefColorMap.getValue());

//   QObject::connect(this->ctrl->predefCombo, SIGNAL(activated(int)),
//                    this, SLOT(predefColorMapUpdate(int)));
}

// void
// OrthoSliceHandler::predefColorMapUpdate(int idx)
// {
//   this->node->predefColorMap = idx;
// }

void
OrthoSliceHandler::sliceNumberEditUpdate(void)
{
  this->node->sliceNumber = this->ctrl->sliceNumberEdit->text().toInt();
}
