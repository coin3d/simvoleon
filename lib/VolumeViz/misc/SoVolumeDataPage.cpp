#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif // HAVE_WINDOWS_H
#include <GL/gl.h>


#include <VolumeViz/misc/SoVolumeDataPage.h>
#include <Inventor/misc/SoState.h>
#include <Inventor/actions/SoGLRenderAction.h>

SoVolumeDataPage::SoVolumeDataPage()
{
  this->size = SbVec2s(0, 0);
  this->format = 0;
  this->storage = NOT_LOADED;
  this->lastuse = 0;
  this->transferFunctionId = 0;
}// constructor


SoVolumeDataPage::~SoVolumeDataPage()
{
  glDeleteTextures(1, &textureName);
  delete [] data;
}// destructor


// FIXME: Some magic has to be done to make this one work with OpenGL 1.0. 
// torbjorv 08052002
void SoVolumeDataPage::setActivePage(long tick)
{
  glBindTexture(GL_TEXTURE_2D, this->textureName);
  this->lastuse = tick;
}



void SoVolumeDataPage::setData( Storage storage,
                                unsigned char * bytes,
                                const SbVec2s & size,
                                const int numcomponents,
                                const int format,
                                const float quality,
                                const int border)
{
  this->size = size;
  this->format = format;
  this->storage = storage;
  this->lastuse = 0;

  if (storage & MEMORY) {
    this->data = new unsigned char[size[0]*size[1]*numcomponents];
    memcpy(this->data, bytes, size[0]*size[1]*numcomponents);
  }// if
  else
    this->data = NULL;

  if (storage & OPENGL) {
    //FIXME: these functions is only supported in opengl 1.1... torbjorv 08052002
    glGenTextures(1, &this->textureName);
    glBindTexture(GL_TEXTURE_2D, this->textureName);
    glTexImage2D( GL_TEXTURE_2D, 
                  0,
                  numcomponents,
                  size[0], 
                  size[1],
                  border,
                  format,
                  GL_UNSIGNED_BYTE,
                  bytes);


      // FIXME: Okay. I tried. This GLWrapper-thingy must be spawned right 
      // out of hell. I'm really not able to compile it. But these lines 
      // need to be fixed for OpenGL 1.0 support. torbjorv 03082002

//    GLenum clamping;
//      const GLWrapper_t * glw = GLWRAPPER_FROM_STATE(state);
//      if (glw->hasTextureEdgeClamp) 
//        clamping = GL_CLAMP_TO_EDGE;
//      else
//        (GLenum) GL_CLAMP;


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  }// if  
  else
    this->textureName = 0;

}//setData


