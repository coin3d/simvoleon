#ifndef COIN_CVR3DTEXSUBCUBE_H
#define COIN_CVR3DTEXSUBCUBE_H

#include <Inventor/SbVec3s.h>
#include <Inventor/SbVec3f.h>
#include <Inventor/SbVec3f.h>
#include <Inventor/lists/SbList.h>
#include <Inventor/misc/SoState.h>
#include <Inventor/system/gl.h>
#include <Inventor/C/glue/gl.h>
#include <Inventor/SbClip.h>
#include <Inventor/SbViewVolume.h>

#include <VolumeViz/render/common/CvrTextureObject.h>

class SoGLRenderAction;
class CvrTextureObject;
class CvrCLUT;

struct subcube_slice {
  SbList <SbVec3f> texcoord; 
  SbList <SbVec3f> vertex;  
};

struct subcube_vertexinfo {
  SbVec3f vertex;
  SbVec3f texcoord;
  int neighbours[3];
};


class Cvr3DTexSubCube {
public:
  Cvr3DTexSubCube(SoGLRenderAction * action,
                  const CvrTextureObject * texobj,
                  const SbVec3f & cubesize, 
                  const SbVec3s & texsize,
                  const SbBool compresstextures);
  ~Cvr3DTexSubCube();

  enum Interpolation { NEAREST, LINEAR };

  void render(const SoGLRenderAction * action, Interpolation interpolation);
  void renderBBox(const SoGLRenderAction * action, int counter); // Debug method

  SbBool isPaletted(void) const;
  void setPalette(const CvrCLUT * newclut);

  static unsigned int totalNrOfTexels(void);
  static unsigned int totalTextureMemoryUsed(void);

  SbBool checkIntersectionSlice(SbVec3f const & cubeorigo, 
                                const SbViewVolume & viewvolume, 
                                float viewdistance);

  static void * subcube_clipperCB(const SbVec3f & v0, void * vdata0, 
                                  const SbVec3f & v1, void * vdata1,
                                  const SbVec3f & newvertex,
                                  void * userdata);

  float getDistanceFromCamera();
  void setDistanceFromCamera(float dist);


private:
  void transferTex3GL(SoGLRenderAction * action,
                      const CvrTextureObject * texobj);
  
  void activateTexture(Interpolation interpolation) const;
  void activateCLUT(const SoGLRenderAction * action); 
  void deactivateCLUT(const SoGLRenderAction * action); 
 
  GLuint texturename[1];
  static GLuint emptyimgname[1];
  SbVec3s texdims;
  SbVec3s originaltexsize;
  SbVec3f dimensions;
  SbVec3f origo;

  static unsigned int nroftexels;
  static unsigned int texmembytes;
  static SbBool detectedtextureswapping;
  unsigned int bitspertexel;
  const CvrCLUT * clut;
  SbBool ispaletted;
  SbBool compresstextures;
  float distancefromcamera;
  subcube_vertexinfo * subcubestruct;

  SbList <subcube_slice> volumeslices;
  SbList <SbVec3f *> texcoordlist;

};


#endif //COIN_CVR3DTEXSUBPAGE_H
