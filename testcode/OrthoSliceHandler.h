#ifndef CVRTEST_ORTHOSLICEHANDLER_H
#define CVRTEST_ORTHOSLICEHANDLER_H

#include <qwidget.h>

class SoOrthoSlice;
class SoVolumeData;
class SoOrthoSlice_ctrl;


class OrthoSliceHandler : QObject
{
  Q_OBJECT

public:
  OrthoSliceHandler(SoOrthoSlice * node, const SoVolumeData * volumedatanode, QWidget * parent = NULL);
  ~OrthoSliceHandler();

private slots:
  void sliceNumberSliderUpdate(int val);
  void sliceNumberEditUpdate(void);
  void axisUpdate(int idx);
  void interpolationUpdate(int idx);
  void alphaUseUpdate(int idx);
  void clippingCheckBoxUpdate(int idx);
  void clippingSideUpdate(int idx);

private:
  void initGUI(void);

  SoOrthoSlice * node;
  const SoVolumeData * vdnode;
  SoOrthoSlice_ctrl * ctrl;
};

#endif // !CVRTEST_ORTHOSLICEHANDLER_H
