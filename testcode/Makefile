all: examine readvol raypick

## 

READVOL_OBJECTS = \
        readvol.o \
        TransferFunctionHandler.o moc_TransferFunctionHandler.o \
        SoTransferFunction_ctrl.o moc_SoTransferFunction_ctrl.o

EXAMINE_OBJECTS = \
        examine.o

RAYPICK_OBJECTS = \
        raypick.o

clean:
	rm -f raypick $(RAYPICK_OBJECTS) raypick.cpp \
              examine $(EXAMINE_OBJECTS) examine.cpp \
              readvol $(READVOL_OBJECTS) readvol.cpp \
              moc_TransferFunctionHandler.cpp \
              SoTransferFunction_ctrl.h SoTransferFunction_ctrl.cpp moc_SoTransferFunction_ctrl.cpp

## ``readvol'' example

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

## ``examine'' example

examine.cpp: examine.cpp.in
	@soqt-config --ac-subst $<

examine.o: examine.cpp
	soqt-config --compile $<

examine: $(EXAMINE_OBJECTS)
	soqt-config --link $@ $(EXAMINE_OBJECTS) -lVolumeViz

## ``raypick'' example

raypick.cpp: raypick.cpp.in
	@soqt-config --ac-subst $<

raypick.o: raypick.cpp
	soqt-config --compile $<

raypick: $(RAYPICK_OBJECTS)
	soqt-config --link $@ $(RAYPICK_OBJECTS) -lVolumeViz

## end
