#ifndef CVRTEST_TRANSFERFUNCTIONHANDLER_H
#define CVRTEST_TRANSFERFUNCTIONHANDLER_H

#include <qwidget.h>

class SoTransferFunction;
class SoTransferFunction_ctrl;


class TransferFunctionHandler : QObject
{
  Q_OBJECT

public:
  TransferFunctionHandler(SoTransferFunction * node, QWidget * parent = NULL);
  ~TransferFunctionHandler();

private slots:
  void lowEditUpdate(void);
  void highEditUpdate(void);
  void predefColorMapUpdate(int idx);

private:
  void initGUI(void);

  SoTransferFunction * node;
  SoTransferFunction_ctrl * ctrl;

  int remap[2];
};

#endif // !CVRTEST_TRANSFERFUNCTIONHANDLER_H
