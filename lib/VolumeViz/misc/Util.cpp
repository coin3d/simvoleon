#include <VolumeViz/misc/CvrUtil.h>
#include <VolumeViz/misc/CvrVoxelChunk.h>
#include <string.h>
#include <Inventor/C/tidbits.h>

// Returns a value indicating whether or not to spit out debugging
// information during execution. Debugging is turned on by the user by
// setting the CVR_DEBUG environment variable to "1".
SbBool
CvrUtil::doDebugging(void)
{
  static int do_debugging = -1;
  if (do_debugging == -1) {
    const char * envstr = coin_getenv("CVR_DEBUG");
    do_debugging = envstr && atoi(envstr) > 0;
  }
  return do_debugging == 1 ? TRUE : FALSE;
}
