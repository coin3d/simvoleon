#include <Inventor/SbVec3s.h>
#include <string.h>

#include "utility.h"

static void
set_dot(uint32_t * pData, SbVec3s & dim,
        float X, float Y, float Z,
        unsigned char R,
        unsigned char G,
        unsigned char B,
        unsigned char A)
{
  int nX = int(dim[0]*X);
  int nY = int(dim[1]*Y);
  int nZ = int(dim[2]*Z);

  int pixel = (A<<24) + (B<<16) + (G<<8) + R;
  pData[nZ*dim[0]*dim[1] + nY*dim[0] + nX] = pixel;
}

void *
generateRGBAVoxelSet(SbVec3s & dim)
{
  uint32_t * voxels = new uint32_t[dim[0] * dim[1] * dim[2]];
  (void)memset(voxels, 0, dim[0] * dim[1] * dim[2]*4);

  float t = 0;

  while (t < 50) {
    float x = sin((t + 1.4234) * 1.9) * sin(t) * 0.45 + 0.5;
    float y = cos((t * 2.5) - 10) * 0.45 + 0.5;
    float z = cos((t - 0.23123) * 3) * sin(t + 0.5) * cos(t) * 0.45 + 0.5;

    uint8_t r = (unsigned char)(255.0 * cos(t));
    uint8_t g = (unsigned char)(255.0 * sin(t));
    uint8_t b = 0xff;
    uint8_t a = 0xff;

    set_dot(voxels, dim, x, y, z, r, g, b, a);

    t += 0.001;
  }

  return voxels;
}
