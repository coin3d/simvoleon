#include "VolumeRenderHandler.h"
#include "SoVolumeRender_ctrl.h"

#include <VolumeViz/nodes/SoVolumeRender.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <qvalidator.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qslider.h>
#include <qlabel.h>


VolumeRenderHandler::VolumeRenderHandler(SoVolumeRender * rendernode,
                                         const SoVolumeData * volumedatanode,
                                         QWidget * parent)
{
  this->node = rendernode;
  // FIXME: should better place a sensor on it and detect when the
  // node is dying. 20021211 mortene.
  this->node->ref();

  this->vdnode = volumedatanode;
  this->vdnode->ref();

  this->ctrl = new SoVolumeRender_ctrl(parent);
  this->ctrl->show();

  this->initGUI();
}

VolumeRenderHandler::~VolumeRenderHandler()
{
  this->node->unref();
  this->vdnode->unref();
}

void
VolumeRenderHandler::initGUI(void)
{
  SbVec3s dimension;
  void * data;
  SoVolumeData::DataType type;
  SbBool ok = this->vdnode->getVolumeData(dimension, data, type);

  // "numSlices" slider & edit

  this->ctrl->numSlicesSlider->setMinValue(0);
  const unsigned short maxdim = SbMax(dimension[0], SbMax(dimension[1], dimension[2]));
  this->ctrl->numSlicesSlider->setMaxValue(maxdim);
  this->ctrl->numSlicesSlider->setValue(this->node->numSlices.getValue());

  QObject::connect(this->ctrl->numSlicesSlider, SIGNAL(valueChanged(int)),
                   this, SLOT(numSlicesSliderUpdate(int)));


  QString s;
  s.sprintf("%d", this->node->numSlices.getValue());
  this->ctrl->numSlicesEdit->setText(s);

  QObject::connect(this->ctrl->numSlicesEdit, SIGNAL(returnPressed()),
                   this, SLOT(numSlicesEditUpdate()));

  // numSlicesControl combobox

  this->ctrl->numSlicesControlCombo->setCurrentItem(this->node->numSlicesControl.getValue());

  QObject::connect(this->ctrl->numSlicesControlCombo, SIGNAL(activated(int)),
                   this, SLOT(numSlicesControlUpdate(int)));

  if (this->node->numSlicesControl.getValue() == SoVolumeRender::ALL) {
    this->ctrl->numSlicesEdit->setEnabled(FALSE);
    this->ctrl->numSlicesSlider->setEnabled(FALSE);
  }

  // interpolation combobox

  this->ctrl->interpolationCombo->setCurrentItem(this->node->interpolation.getValue());

  QObject::connect(this->ctrl->interpolationCombo, SIGNAL(activated(int)),
                   this, SLOT(interpolationUpdate(int)));

  // composition combobox

  this->ctrl->compositionCombo->setCurrentItem(this->node->composition.getValue());

  QObject::connect(this->ctrl->compositionCombo, SIGNAL(activated(int)),
                   this, SLOT(compositionUpdate(int)));

  // viewAlignedSlices checkbox

  this->ctrl->viewAlignedSlicesCheckBox->setChecked(this->node->viewAlignedSlices.getValue());

  QObject::connect(this->ctrl->viewAlignedSlicesCheckBox, SIGNAL(stateChanged(int)),
                   this, SLOT(viewAlignedSlicesCheckBoxUpdate(int)));

  // Not in use yet -- 3D textures are not supported.
  this->ctrl->viewAlignedSlicesCheckBox->setEnabled(FALSE);
}


void
VolumeRenderHandler::numSlicesSliderUpdate(int val)
{
  this->node->numSlices = val;

  QString s;
  s.sprintf("%d", val);
  this->ctrl->numSlicesEdit->setText(s);
}

void
VolumeRenderHandler::numSlicesEditUpdate(void)
{
  this->node->numSlices = this->ctrl->numSlicesEdit->text().toInt();

  this->ctrl->numSlicesSlider->setValue(this->node->numSlices.getValue());
}

void
VolumeRenderHandler::numSlicesControlUpdate(int idx)
{
  this->node->numSlicesControl = idx;

  const SbBool is_all = this->node->numSlicesControl.getValue() == SoVolumeRender::ALL;
  this->ctrl->numSlicesEdit->setEnabled(!is_all);
  this->ctrl->numSlicesSlider->setEnabled(!is_all);
  this->ctrl->numSlicesLabel->setEnabled(!is_all);
}

void
VolumeRenderHandler::interpolationUpdate(int idx)
{
  this->node->interpolation = idx;
}

void
VolumeRenderHandler::compositionUpdate(int idx)
{
  this->node->composition = idx;
}

void
VolumeRenderHandler::viewAlignedSlicesCheckBoxUpdate(int idx)
{
  if (idx == QButton::NoChange) return;
  this->node->viewAlignedSlices = (idx == QButton::On) ? TRUE : FALSE;
}
