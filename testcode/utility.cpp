#include <Inventor/SbVec3s.h>
#include <string.h>

#include "utility.h"

static void
setDot(uint32_t * pData, SbVec3s & dim,
       float X, float Y, float Z,
       unsigned char R,
       unsigned char G,
       unsigned char B,
       unsigned char A)
{
  int nX = int(dim[0]*X);
  int nY = int(dim[1]*Y);
  int nZ = int(dim[2]*Z);

  //  if (A < 128) A = 0;
  // if (A >= 128) A = 255;
  A >>= 2;

  if (R < 128) R = 0;
  if (R >= 128) R = 255;

  int pixel = (A<<24) + (B<<16) + (G<<8) + R;
  pData[nZ*dim[0]*dim[1] + nY*dim[0] + nX] = pixel;
  pData[(nZ+1)*dim[0]*dim[1] + nY*dim[0] + nX] = pixel;
  pData[(nZ-1)*dim[0]*dim[1] + nY*dim[0] + nX] = pixel;
  pData[nZ*dim[0]*dim[1] + (nY+1)*dim[0] + nX] = pixel;
  pData[nZ*dim[0]*dim[1] + (nY-1)*dim[0] + nX] = pixel;
  pData[nZ*dim[0]*dim[1] + nY*dim[0] + (nX+1)] = pixel;
  pData[nZ*dim[0]*dim[1] + nY*dim[0] + (nX-1)] = pixel;
}

void *
generateRGBAVoxelSet(SbVec3s & dim)
{
  uint32_t * voxels = new uint32_t[dim[0] * dim[1] * dim[2]];
  (void)memset(voxels, 0, dim[0] * dim[1] * dim[2]*4);

  float t = 0;

  while (t < 50) {
    float x = sin((t + 1.4234)*1.9);
    float y = cos((t*2.5) - 10);
    float z = cos((t - 0.23123)*3)*sin(t + 0.5);

    setDot(voxels, dim,
           x*sin(t)*0.45 + 0.5,
           y*0.45 + 0.5,
           z*cos(t)*0.45 + 0.5,
           (unsigned char)(255.0*cos(t)),
           (unsigned char)(255.0*sin(t)),
           255,
           255);

    t += 0.001;
  }

  return voxels;
}
