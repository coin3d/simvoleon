#ifndef CVRTEST_TRANSFERFUNCTIONHANDLER_H
#define CVRTEST_TRANSFERFUNCTIONHANDLER_H

#include <qwidget.h>

class SoTransferFunction;
class SoTransferFunction_ctrl;
class Gradient;


class TransferFunctionHandler : QObject
{
  Q_OBJECT

public:
  TransferFunctionHandler(SoTransferFunction * node,
                          int remaplow, int remaphigh,
                          QWidget * parent = NULL);
  ~TransferFunctionHandler();

private slots:
  void lowEditUpdate(void);
  void highEditUpdate(void);
  void predefColorMapUpdate(int idx);
  void shiftEditUpdate(void);
  void offsetEditUpdate(void);

private:
  void initGUI(void);

  SoTransferFunction * node;
  SoTransferFunction_ctrl * ctrl;

  int remap[2];

  class SoQtGradientDialog * gradientdialog;
  void gradientCallback(const Gradient & g);
  static void gradientCallbackP(const Gradient & g, void * userdata);
};

#endif // !CVRTEST_TRANSFERFUNCTIONHANDLER_H
