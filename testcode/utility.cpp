#include <Inventor/SbVec3s.h>
#include <Inventor/SbVec3f.h>
#include <string.h>
#include <assert.h>

#include "utility.h"

uint8_t *
generateRGBAVoxelSet(SbVec3s & dim)
{
  const size_t blocksize = dim[0] * dim[1] * dim[2] * 4;
  uint8_t * voxels = new uint8_t[blocksize];
  (void)memset(voxels, 0, blocksize);

  float t = 0;

  while (t < 50) {
    SbVec3f v(sin((t + 1.4234) * 1.9) * sin(t) * 0.45 + 0.5,
              cos((t * 2.5) - 10) * 0.45 + 0.5,
              cos((t - 0.23123) * 3) * sin(t + 0.5) * cos(t) * 0.45 + 0.5);

    uint8_t rgba[4] = {
      (uint8_t)(255.0 * cos(t)), (uint8_t)(255.0 * sin(t)), 0xff, // rgb
      0xff // alpha
    };

    assert(v[0] < 1.0f && v[1] < 1.0f && v[2] < 1.0f);
    const int nx = int(dim[0] * v[0]);
    const int ny = int(dim[1] * v[1]);
    const int nz = int(dim[2] * v[2]);

    const int memposition = nz*dim[0]*dim[1] + ny*dim[0] + nx;
    for (int i=0; i < 4; i++) { voxels[memposition * 4 + i] = rgba[i]; }

    t += 0.001;
  }

  return voxels;
}

uint8_t *
generate8bitVoxelSet(SbVec3s & dim)
{
  const size_t blocksize = dim[0] * dim[1] * dim[2];
  uint8_t * voxels = new uint8_t[blocksize];
  (void)memset(voxels, 0, blocksize);

  float t = 0;

  while (t < 50) {
    SbVec3f v(sin((t + 1.4234) * 1.9) * sin(t) * 0.45 + 0.5,
              cos((t * 2.5) - 10) * 0.45 + 0.5,
              cos((t - 0.23123) * 3) * sin(t + 0.5) * cos(t) * 0.45 + 0.5);

    assert(v[0] < 1.0f && v[1] < 1.0f && v[2] < 1.0f);
    const int nx = int(dim[0] * v[0]);
    const int ny = int(dim[1] * v[1]);
    const int nz = int(dim[2] * v[2]);

    const int memposition = nz*dim[0]*dim[1] + ny*dim[0] + nx;
    voxels[memposition] = (uint8_t)(255.0 * cos(t));

    t += 0.001;
  }

  return voxels;
}
