all: readvol

READVOL_OBJECTS = \
        readvol.o \
        TransferFunctionHandler.o moc_TransferFunctionHandler.o \
        SoTransferFunction_ctrl.o moc_SoTransferFunction_ctrl.o

clean:
	rm -f readvol $(READVOL_OBJECTS) readvol.cpp \
              moc_TransferFunctionHandler.cpp \
              SoTransferFunction_ctrl.h SoTransferFunction_ctrl.cpp moc_SoTransferFunction_ctrl.cpp

readvol.cpp: readvol.cpp.in
	@soqt-config --ac-subst $<

SoTransferFunction_ctrl.h: SoTransferFunction_ctrl.ui
	uic $< -o $@

SoTransferFunction_ctrl.cpp: SoTransferFunction_ctrl.ui SoTransferFunction_ctrl.h
	uic -impl $*.h $< -o $@

SoTransferFunction_ctrl.o: SoTransferFunction_ctrl.cpp
	soqt-config --compile $<

moc_SoTransferFunction_ctrl.cpp: SoTransferFunction_ctrl.h
	moc -o $@ $<

moc_SoTransferFunction_ctrl.o: moc_SoTransferFunction_ctrl.cpp
	soqt-config --compile $<

moc_TransferFunctionHandler.cpp: TransferFunctionHandler.h
	moc -o $@ $<

moc_TransferFunctionHandler.o: moc_TransferFunctionHandler.cpp
	soqt-config --compile $<

readvol.o: readvol.cpp TransferFunctionHandler.h
	soqt-config --compile $<

TransferFunctionHandler.o: TransferFunctionHandler.cpp SoTransferFunction_ctrl.h moc_SoTransferFunction_ctrl.cpp
	soqt-config --compile $<

readvol: $(READVOL_OBJECTS)
	soqt-config --link $@ $(READVOL_OBJECTS) -lVolumeViz
