/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/

#ifndef COIN_SOVOLUMEDATASLICE_H
#define COIN_SOVOLUMEDATASLICE_H

#include <Inventor/SbVec2s.h>
#include <Inventor/misc/SoState.h>
#include <VolumeViz/misc/SoVolumeDataPage.h>
#include <VolumeViz/nodes/SoTransferFunction.h>
#include <VolumeViz/readers/SoVolumeReader.h>

class SoVolumeDataSlice{
public:
  SoVolumeDataSlice();
  ~SoVolumeDataSlice();

  SoVolumeDataPage * getPage(int col, 
                             int row, 
                             SoTransferFunction * transferFunction);

  SoVolumeDataPage * buildPage(int col, 
                               int row,
                               SoTransferFunction * transferFunction);

  void init(SoVolumeReader * reader,
            int sliceIdx,
            SoVolumeRendering::Axis axis,
            SbVec2s &pageSize);

  void render(SoState * state,
              SbVec3f &v0, 
              SbVec3f &v1, 
              SbVec3f &v2, 
              SbVec3f &v3, 
              SbBox2f &textureCoords, 
              SoTransferFunction * transferFunction,
              long tick);

  void releasePage(SoVolumeDataPage *page);
  void releaseLRUPage();
  void releaseAllPages();
  SoVolumeDataPage * getLRUPage();

  SoVolumeDataPage **pages;

  SoVolumeReader * reader;

  SoVolumeRendering::Axis axis;
  SoVolumeRendering::DataType dataType;
  int sliceIdx;
  SbVec2s pageSize;
  SbVec2s dimensions;
  

  int numTexels;
  int numPages;
  int numBytesHW;
  int numBytesSW;

  int numCols;
  int numRows;
};


#endif //COIN_SOVOLUMEDATASLICE_H