/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http:// www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/
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
#include <VolumeViz/nodes/SoVolumeRendering.h>

SoVolumeDataPage::SoVolumeDataPage()
{
  this->size = SbVec2s(0, 0);
  this->format = 0;
  this->storage = NOT_LOADED;
  this->lastuse = 0;
  this->transferFunctionId = 0;
  this->data = NULL;
  this->palette = NULL;
  this->paletteFormat = 0;
  this->paletteSize = 0;
  this->nextPage = NULL;
  this->numBytesHW = 0;
  this->numBytesSW = 0;
}// constructor


SoVolumeDataPage::~SoVolumeDataPage()
{
  release();
  delete nextPage;
}// destructor


// FIXME: Some magic has to be done to make this one work with OpenGL 1.0. 
// torbjorv 08052002
void SoVolumeDataPage::setActivePage(long tick)
{
  glBindTexture(GL_TEXTURE_2D, this->textureName);
  this->lastuse = tick;

}// setActivePage



/*
  If no palette specified, this function assumes RGBA data. If a palette
  is specified, the input data should be indexes into the palette. 
  The function uses the palette's size to decide whether the indices are
  byte or short. 
*/
void SoVolumeDataPage::setData( Storage storage,
                                unsigned char * bytes,
                                const SbVec2s & size,
                                const float * palette,
                                int paletteFormat,
                                int paletteSize)
{
  release();
  this->size = size;
  this->storage = storage;
  this->paletteFormat = paletteFormat;
  this->paletteSize = paletteSize;




  // Possible creating an in-memory copy of all data
  if (storage & MEMORY) {
    this->data = new unsigned char[size[0]*size[1]*4];
    memcpy(this->data, bytes, size[0]*size[1]*4);

    if (!palette) 
      numBytesSW += size[0]*size[1]*4;      // RGBA-data
    else {
      if (paletteSize > 256)
        numBytesSW += size[0]*size[1]*2;    // indexed, short
      else
        numBytesSW += size[0]*size[1];      // indexed, byte
    }// else

    if (palette != NULL) {
      this->palette = new unsigned char[sizeof(unsigned char)*paletteSize];
      memcpy(this->palette, palette, sizeof(float)*paletteSize);

      switch (paletteFormat) {
        case GL_RGBA: numBytesSW += paletteSize*4;
                      break;
      }// switch
    }// if
  }// if
  else {
    this->data = NULL;
    this->palette = NULL;
  }// else





  // Creating OpenGL-texture
  if (storage & OPENGL) {
    // FIXME: these functions is only supported in opengl 1.1... 
    // torbjorv 08052002
    glGenTextures(1, &this->textureName);
    glBindTexture(GL_TEXTURE_2D, this->textureName);


    // Uploading standard RGBA-texture
    if (palette == NULL) {
      glTexImage2D( GL_TEXTURE_2D, 
                    0,
                    4,
                    size[0], 
                    size[1],
                    0,
                    GL_RGBA,
                    GL_UNSIGNED_BYTE,
                    bytes);

      numBytesHW += size[0]*size[1]*4;
    }// if

    // Uploading paletted texture
    else {

      // Check size of indices
      int format = GL_UNSIGNED_BYTE;
      if (paletteSize > 256)
        format = GL_UNSIGNED_SHORT;


      // Setting palette
      glColorTableEXT(GL_TEXTURE_2D, 
                      GL_RGBA, 
                      paletteSize,
                      GL_RGBA,
                      GL_FLOAT,
                      palette);

      // Checking what palettesize we actually got
      int actualPaletteSize;
      glGetColorTableParameterivEXT(GL_TEXTURE_2D, 
                                    GL_COLOR_TABLE_WIDTH_EXT, 
                                    &actualPaletteSize);

      numBytesHW += actualPaletteSize*4*4;


      int internalFormat;
      switch (actualPaletteSize) {
        case     2: internalFormat = GL_COLOR_INDEX1_EXT;
                    numBytesHW += size[0]*size[1]/8;
                    break;
        case     4: internalFormat = GL_COLOR_INDEX2_EXT;
                    numBytesHW += size[0]*size[1]/4;
                    break;
        case    16: internalFormat = GL_COLOR_INDEX4_EXT;
                    numBytesHW += size[0]*size[1]/2;
                    break;
        case   256: internalFormat = GL_COLOR_INDEX8_EXT;
                    numBytesHW += size[0]*size[1];
                    break;
        case 65536: internalFormat = GL_COLOR_INDEX16_EXT;
                    numBytesHW += size[0]*size[1]*2;
                    break;
      }// switch


      // Upload texture
      glTexImage2D(GL_TEXTURE_2D, 
                   0,
                   internalFormat,
                   size[0],
                   size[1],
                   0,
                   GL_COLOR_INDEX,
                   format,
                   bytes);

    }//else


    // FIXME: Okay. I tried. This GLWrapper-thingy must be spawned right 
    // out of hell. I'm really not able to compile it. But these lines 
    // need to be fixed for OpenGL 1.0 support. torbjorv 08032002

    // GLenum clamping;
    // const GLWrapper_t * glw = GLWRAPPER_FROM_STATE(state);
    // if (glw->hasTextureEdgeClamp) 
    //   clamping = GL_CLAMP_TO_EDGE;
    // else
    //   (GLenum) GL_CLAMP;


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  }// if  
  else
    this->textureName = 0;

}//setData



void SoVolumeDataPage::release()
{
  if (textureName != 0)
    glDeleteTextures(1, &textureName);

  delete [] data;
  delete [] palette;

  this->size = SbVec2s(0, 0);
  this->format = 0;
  this->storage = NOT_LOADED;
  this->lastuse = 0;
  this->transferFunctionId = 0;
  this->data = NULL;
  this->palette = NULL;
  this->paletteFormat = 0;
  this->paletteSize = 0;
  this->numBytesHW = 0;
  this->numBytesSW = 0;
}// release