#ifndef CVRTEST_VOLUMERENDERHANDLER_H
#define CVRTEST_VOLUMERENDERHANDLER_H

#include <qwidget.h>

class SoVolumeRender;
class SoVolumeData;
class SoVolumeRender_ctrl;


class VolumeRenderHandler : QObject
{
  Q_OBJECT

public:
  VolumeRenderHandler(SoVolumeRender * node, const SoVolumeData * volumedatanode, QWidget * parent = NULL);
  ~VolumeRenderHandler();

private slots:
  void numSlicesSliderUpdate(int val);
  void numSlicesEditUpdate(void);
  void numSlicesControlUpdate(int idx);
  void interpolationUpdate(int idx);
  void compositionUpdate(int idx);
  void viewAlignedSlicesCheckBoxUpdate(int idx);

private:
  void initGUI(void);

  SoVolumeRender * node;
  const SoVolumeData * vdnode;
  SoVolumeRender_ctrl * ctrl;
};

#endif // !CVRTEST_VOLUMERENDERHANDLER_H
