all: examine simple8bit readvol raypick rgbload changingdataset orthoslicer

## 

READVOL_OBJECTS = \
        readvol.o \
        TransferFunctionHandler.o moc_TransferFunctionHandler.o \
        SoTransferFunction_ctrl.o moc_SoTransferFunction_ctrl.o

EXAMINE_OBJECTS = \
        examine.o utility.o

ORTHOSLICER_OBJECTS = \
        orthoslicer.o utility.o \
        OrthoSliceHandler.o moc_OrthoSliceHandler.o \
        SoOrthoSlice_ctrl.o moc_SoOrthoSlice_ctrl.o

SIMPLE8BIT_OBJECTS = \
        simple8bit.o utility.o

CHANGINGDATASET_OBJECTS = \
        changingdataset.o

RAYPICK_OBJECTS = \
        raypick.o utility.o

RGBLOAD_OBJECTS = \
        rgbload.o

clean:
	rm -f rgbload $(RGBLOAD_OBJECTS) rgbload.cpp \
              raypick $(RAYPICK_OBJECTS) raypick.cpp \
              examine $(EXAMINE_OBJECTS) examine.cpp \
              orthoslicer $(ORTHOSLICER_OBJECTS) orthoslicer.cpp \
              simple8bit $(SIMPLE8BIT_OBJECTS) simple8bit.cpp \
              changingdataset $(CHANGINGDATASET_OBJECTS) changingdataset.cpp \
              readvol $(READVOL_OBJECTS) readvol.cpp \
              moc_TransferFunctionHandler.cpp \
              SoOrthoSlice_ctrl.h SoOrthoSlice_ctrl.cpp moc_SoOrthoSlice_ctrl.cpp \
              moc_OrthoSliceHandler.cpp \

## common code

utility.o: utility.cpp
	soqt-config --compile $<

# (SoTransferFunction UI)

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

TransferFunctionHandler.o: TransferFunctionHandler.cpp SoTransferFunction_ctrl.h moc_SoTransferFunction_ctrl.cpp
	soqt-config --compile $<

# (SoOrthoSlice UI)

SoOrthoSlice_ctrl.h: SoOrthoSlice_ctrl.ui
	uic $< -o $@

SoOrthoSlice_ctrl.cpp: SoOrthoSlice_ctrl.ui SoOrthoSlice_ctrl.h
	uic -impl $*.h $< -o $@

SoOrthoSlice_ctrl.o: SoOrthoSlice_ctrl.cpp
	soqt-config --compile $<

moc_SoOrthoSlice_ctrl.cpp: SoOrthoSlice_ctrl.h
	moc -o $@ $<

moc_SoOrthoSlice_ctrl.o: moc_SoOrthoSlice_ctrl.cpp
	soqt-config --compile $<

moc_OrthoSliceHandler.cpp: OrthoSliceHandler.h
	moc -o $@ $<

moc_OrthoSliceHandler.o: moc_OrthoSliceHandler.cpp
	soqt-config --compile $<

OrthoSliceHandler.o: OrthoSliceHandler.cpp SoOrthoSlice_ctrl.h moc_SoOrthoSlice_ctrl.cpp
	soqt-config --compile $<


## ``readvol'' example

readvol.cpp: readvol.cpp.in
	@soqt-config --ac-subst $<

readvol.o: readvol.cpp TransferFunctionHandler.h
	soqt-config --compile $<

readvol: $(READVOL_OBJECTS)
	soqt-config --link $@ $(READVOL_OBJECTS) -lSimVoleon

## ``examine'' example

examine.cpp: examine.cpp.in
	@soqt-config --ac-subst $<

examine.o: examine.cpp
	soqt-config --compile $<

examine: $(EXAMINE_OBJECTS)
	soqt-config --link $@ $(EXAMINE_OBJECTS) -lSimVoleon

## ``orthoslicer'' example

orthoslicer.cpp: orthoslicer.cpp.in
	@soqt-config --ac-subst $<

orthoslicer.o: orthoslicer.cpp TransferFunctionHandler.h
	soqt-config --compile $<

orthoslicer: $(ORTHOSLICER_OBJECTS)
	soqt-config --link $@ $(ORTHOSLICER_OBJECTS) -lSimVoleon

## ``simple8bit'' example

simple8bit.cpp: simple8bit.cpp.in
	@soqt-config --ac-subst $<

simple8bit.o: simple8bit.cpp
	soqt-config --compile $<

simple8bit: $(SIMPLE8BIT_OBJECTS)
	soqt-config --link $@ $(SIMPLE8BIT_OBJECTS) -lSimVoleon

## ``changingdataset'' example

changingdataset.cpp: changingdataset.cpp.in
	@soqt-config --ac-subst $<

changingdataset.o: changingdataset.cpp
	soqt-config --compile $<

changingdataset: $(CHANGINGDATASET_OBJECTS)
	soqt-config --link $@ $(CHANGINGDATASET_OBJECTS) -lSimVoleon

## ``raypick'' example

raypick.cpp: raypick.cpp.in
	@soqt-config --ac-subst $<

raypick.o: raypick.cpp
	soqt-config --compile $<

raypick: $(RAYPICK_OBJECTS)
	soqt-config --link $@ $(RAYPICK_OBJECTS) -lSimVoleon

## ``rgbload'' example

rgbload.cpp: rgbload.cpp.in
	@soqt-config --ac-subst $<

rgbload.o: rgbload.cpp
	soqt-config --compile $<

rgbload: $(RGBLOAD_OBJECTS)
	soqt-config --link $@ $(RGBLOAD_OBJECTS) -lSimVoleon -lsimage

## end
