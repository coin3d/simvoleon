#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif // HAVE_WINDOWS_H
#include <GL/gl.h>


#include <VolumeViz/misc/SoVolumeDataSlice.h>
#include <Inventor/misc/SoState.h>
#include <Inventor/actions/SoGLRenderAction.h>

SoVolumeDataSlice::SoVolumeDataSlice()
{
  this->size = SbVec2s(0, 0);
  this->pages = NULL;
}// constructor


SoVolumeDataSlice::~SoVolumeDataSlice()
{
}// destructor


void SoVolumeDataSlice::setActivePage(int col, 
                                      int row, 
                                      SoTransferFunction *transferFunction, 
                                      int tick)
{
}

void SoVolumeDataSlice::releaseOldestPage()
{
}

void SoVolumeDataSlice::releaseAllPages()
{
}
