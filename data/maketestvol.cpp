// Make a raw 64x64x64 volume (written on stdout) with just a row of
// values monotonically increasing from 0 to 63 along the X axis where
// Y = 0 and Z = 0. Useful for testing.
//
// -mortene.

#include <stdio.h>
#include <assert.h>

int
main(void)
{
  for (unsigned int k = 0; k < 64; k++) {
    for (unsigned int j = 0; j < 64; j++) {
      for (unsigned int i = 0; i < 64; i++) {
        unsigned char c;
        if (k == 0 && j == 0) { c = (unsigned char)i; }
        else { c = 0; }
        
        size_t written = fwrite(&c, 1, 1, stdout);
        assert(written == 1);
      }
    }
  }

  return 0;
}
