#ifndef COIN_CVRCUBEHANDLER_H
#define COIN_CVRCUBEHANDLER_H

#include <Inventor/SbBox2f.h>
#include <Inventor/SbVec3f.h>
#include <Inventor/SbVec3s.h>
#include <VolumeViz/render/3D/Cvr3DTexSubCube.h>
#include <VolumeViz/nodes/SoVolumeRender.h>

class Cvr3DTexCube;
class SoVolumeReader;
class SoVolumeData;
class SoState;
class SoGLRenderAction;


class CvrCubeHandler {
public:
  CvrCubeHandler(const SbVec3s & voldatadims, SoVolumeReader * reader);
  ~CvrCubeHandler();

  enum Composition { MAX_INTENSITY, SUM_INTENSITY, ALPHA_BLENDING };

  void render(SoGLRenderAction * action, unsigned int numslices,
              Cvr3DTexSubCube::Interpolation interpolation,
              CvrCubeHandler::Composition composition,
              SoVolumeRender::SoVolumeRenderAbortCB * abortfunc,
              void * abortcbdata);

  unsigned int getCurrentAxis(SoGLRenderAction * action) const;

  void releaseAllSlices(void);
  void releaseSlices(const unsigned int AXISIDX);

private:
  unsigned int getCurrentAxis(const SbVec3f & viewvec) const;
  void getViewVector(SoGLRenderAction * action, SbVec3f & direction) const;
  void setPalette(const CvrCLUT * c);

  Cvr3DTexCube * volumecube;
  unsigned int voldatadims[3];
  SbVec3s subcubesize;
  SoVolumeReader * reader;

  uint32_t transferfuncid;
  const CvrCLUT * clut;
};

#endif // !COIN_CVRCUBEHANDLER_H
