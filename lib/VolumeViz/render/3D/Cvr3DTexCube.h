#ifndef COIN_CVR3DTEXPAGE_H
#define COIN_CVR3DTEXPAGE_H

#include <Inventor/SbVec3s.h>
#include <Inventor/misc/SoState.h>
#include <VolumeViz/readers/SoVolumeReader.h>
#include <VolumeViz/render/3D/Cvr3DTexSubCube.h>

class Cvr3DTexCube {
  
public:
  Cvr3DTexCube(SoVolumeReader * reader);
  ~Cvr3DTexCube();
  
  void render(SoGLRenderAction * action, const SbVec3f & origo,
              const SbVec3f & cubespan,
              Cvr3DTexSubCube::Interpolation interpolation,
              unsigned int numslices);
  
  void setPalette(const CvrCLUT * c);
  const CvrCLUT * getPalette(void) const;
  
private:
  class Cvr3DTexSubCubeItem * getSubCube(SoState * state, int col, int row, int depth);

  class Cvr3DTexSubCubeItem * buildSubCube(SoGLRenderAction * action,
                                           int col, int row, int depth,
                                           SbVec3f cubescale);
   
  void releaseAllSubCubes(void);
  void releaseSubCube(const int row, const int col, const int depth);

  int calcSubCubeIdx(int row, int col, int depth) const;
  void calculateOptimalSubCubeSize();

  class Cvr3DTexSubCubeItem ** subcubes;
  SoVolumeReader * reader;

  SbVec3s subcubesize;
  SbVec3s dimensions;

  SbBool rendersubcubeoutline;
  int nrcolumns;
  int nrrows;
  int nrdepths;

  const CvrCLUT * clut;
};

#endif // !COIN_CVR3DTEXPAGE_H
