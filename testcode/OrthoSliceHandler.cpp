#include "OrthoSliceHandler.h"
#include "SoOrthoSlice_ctrl.h"

#include <VolumeViz/nodes/SoOrthoSlice.h>
#include <qvalidator.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>


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

  // axis combobox

  this->ctrl->axisCombo->setCurrentItem(this->node->axis.getValue());

  QObject::connect(this->ctrl->axisCombo, SIGNAL(activated(int)),
                   this, SLOT(axisUpdate(int)));

  // interpolation combobox

  this->ctrl->interpolationCombo->setCurrentItem(this->node->interpolation.getValue());

  QObject::connect(this->ctrl->interpolationCombo, SIGNAL(activated(int)),
                   this, SLOT(interpolationUpdate(int)));

  // alphaUse combobox

  this->ctrl->alphaUseCombo->setCurrentItem(this->node->alphaUse.getValue());

  QObject::connect(this->ctrl->alphaUseCombo, SIGNAL(activated(int)),
                   this, SLOT(alphaUseUpdate(int)));

  // clipping checkbox

  this->ctrl->clippingCheckBox->setChecked(this->node->clipping.getValue());

  QObject::connect(this->ctrl->clippingCheckBox, SIGNAL(stateChanged(int)),
                   this, SLOT(clippingCheckBoxUpdate(int)));

  // clippingSide combobox

  this->ctrl->clippingSideCombo->setCurrentItem(this->node->clippingSide.getValue());

  QObject::connect(this->ctrl->clippingSideCombo, SIGNAL(activated(int)),
                   this, SLOT(clippingSideUpdate(int)));
}


void
OrthoSliceHandler::sliceNumberEditUpdate(void)
{
  this->node->sliceNumber = this->ctrl->sliceNumberEdit->text().toInt();
}

void
OrthoSliceHandler::axisUpdate(int idx)
{
  this->node->axis = idx;
}

void
OrthoSliceHandler::interpolationUpdate(int idx)
{
  this->node->interpolation = idx;
}

void
OrthoSliceHandler::alphaUseUpdate(int idx)
{
  this->node->alphaUse = idx;
}

void
OrthoSliceHandler::clippingCheckBoxUpdate(int idx)
{
  if (idx == QButton::NoChange) return;
  this->node->clipping = (idx == QButton::On) ? TRUE : FALSE;
}

void
OrthoSliceHandler::clippingSideUpdate(int idx)
{
  this->node->clippingSide = idx;
}
