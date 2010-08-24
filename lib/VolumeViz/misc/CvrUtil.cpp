/**************************************************************************\
 *
 *  This file is part of the SIM Voleon visualization library.
 *  Copyright (C) 2003-2004 by Systems in Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  ("GPL") version 2 as published by the Free Software Foundation.
 *  See the file LICENSE.GPL at the root directory of this source
 *  distribution for additional information about the GNU GPL.
 *
 *  For using SIM Voleon with software that can not be combined with
 *  the GNU GPL, and for taking advantage of the additional benefits
 *  of our support services, please contact Systems in Motion about
 *  acquiring a SIM Voleon Professional Edition License.
 *
 *  See <URL:http://www.coin3d.org/> for more information.
 *
 *  Systems in Motion, Postboks 1283, Pirsenteret, 7462 Trondheim, NORWAY.
 *  <URL:http://www.sim.no/>.
 *
\**************************************************************************/

#include <VolumeViz/misc/CvrUtil.h>

#include <cstring>
#include <climits>

#include <Inventor/SbRotation.h>
#include <Inventor/SbLinear.h>
#include <Inventor/C/tidbits.h>

#include <VolumeViz/elements/CvrVoxelBlockElement.h>
#include <VolumeViz/misc/CvrVoxelChunk.h>

// *************************************************************************

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


// If this environment flag is set, output debugging information
// specifically about raypick-related operations.
SbBool
CvrUtil::debugRayPicks(void)
{
  static int CVR_DEBUG_RAYPICKS = -1;
  if (CVR_DEBUG_RAYPICKS == -1) {
    const char * env = coin_getenv("CVR_DEBUG_RAYPICKS");
    CVR_DEBUG_RAYPICKS = env && (atoi(env) > 0);
  }
  return (CVR_DEBUG_RAYPICKS == 0) ? FALSE : TRUE;
}


// For debugging. Force different way of rendering, to aid in
// visualizing how the slices are set up. The significance of the
// possible values are as follows:
//
// 0 - default, render as usual
//
// 1 - render both the usual textured sub-slices aswell as wireframe
//     borders of the sub-slices
//
// 2 - render only the (non-textured) sub-slice frames
unsigned int
CvrUtil::debugRenderStyle(void)
{
  static int renderstyle = -1;
  if (renderstyle == -1) {
    const char * env = coin_getenv("CVR_DEBUG_SLICE_RENDERSTYLE");
    renderstyle = env ? atoi(env) : 0;
    assert(renderstyle >= 0 && renderstyle <= 2);
  }
  return (unsigned int)renderstyle;
}


// If the environment flag is set, data along the Y axis will be
// flipped upside down. This to keep compatibility with an old bug,
// since there's client code depending on this behavior.
SbBool
CvrUtil::useFlippedYAxis(void)
{
  static int val = -1;
  if (val == -1) {
    const char * env = coin_getenv("CVR_USE_FLIPPED_Y_AXIS");
    val = env && (atoi(env) > 0);
  }
  return (val == 0) ? FALSE : TRUE;
}


// A performance gain can be achieved if texture modulation is to be
// ignored. The largest gain is achieved when rendering using
// fragment programs.
SbBool
CvrUtil::dontModulateTextures(void)
{
  static int flag = -1;
  if (flag == -1) {
    const char * envstr = coin_getenv("CVR_DISABLE_TEXTURE_MODULATION");
    flag = envstr && (atoi(envstr) > 0);
  }
  return (flag == 0) ? FALSE : TRUE;
}


// Shall we force 2D texture rendering?
SbBool
CvrUtil::force2DTextureRendering(void)
{
 static int flag = -1;
  if (flag == -1) {
    const char * envstr = coin_getenv("CVR_FORCE_2D_TEXTURES");
    flag = envstr && (atoi(envstr) > 0);
  }
  return (flag == 0) ? FALSE : TRUE;
}


static uint32_t crc32_precalc_table[] = {
  0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
  0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
  0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
  0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
  0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
  0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
  0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
  0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
  0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
  0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
  0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
  0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
  0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
  0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
  0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
  0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
  0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
  0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
  0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
  0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
  0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
  0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
  0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
  0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
  0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
  0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
  0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
  0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
  0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
  0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
  0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
  0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
  0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
  0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
  0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
  0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
  0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
  0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
  0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
  0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
  0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
  0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
  0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};


static inline uint32_t
updc32(uint8_t octet, uint32_t crc)
{
  return crc32_precalc_table[(crc ^ octet) & 0xff] ^ (crc >> 8);
}


// Calculates CRC32 checksum from a block of bytes.
uint32_t
CvrUtil::crc32(uint8_t * buf, unsigned int len)
{
  uint32_t crc = 0xffffffff;
  for (unsigned int i = 0; i < len; i++) { crc = updc32(buf[i], crc); }
  return crc;
}


void
CvrUtil::getTransformFromVolumeBoxDimensions(const CvrVoxelBlockElement * vd,
                                             SbMatrix & m)
{
  const SbVec3s & voxcubedims = vd->getVoxelCubeDimensions();
  const SbBox3f & localbox = vd->getUnitDimensionsBox();

  const SbVec3f
    localspan((localbox.getMax()[0] - localbox.getMin()[0]) / voxcubedims[0],
              (localbox.getMax()[1] - localbox.getMin()[1]) / voxcubedims[1],
              (localbox.getMax()[2] - localbox.getMin()[2]) / voxcubedims[2]);

  const SbVec3f localtrans =
    (localbox.getMax() - localbox.getMin()) / 2.0f + localbox.getMin();

#if 0 // debug, remove when 3D textures have been confirmed to work
  printf("voxcubedims: <%d, %d, %d>\n", voxcubedims[0], voxcubedims[1], voxcubedims[2]);
  printf("localbox: <%f, %f, %f> -> <%f, %f, %f>\n",
         localbox.getMin()[0], localbox.getMin()[1], localbox.getMin()[2],
         localbox.getMax()[0], localbox.getMax()[1], localbox.getMax()[2]);
  printf("localspan: "); localspan.print(stdout); printf("\n");
  printf("localtrans: "); localtrans.print(stdout); printf("\n");
#endif // debug

  m.setTransform(localtrans, SbRotation::identity(), localspan);
}


SbVec3s
CvrUtil::clampSubCubeSize(const SbVec3s & size)
{
  // FIXME: this doesn't guarantee that we can actually use a texture
  // of this size, should instead use Coin's
  // cc_glglue_is_texture_size_legal() (at least in combination with
  // the subcubesize found here). 20040709 mortene.
  //
  // UPDATE: the above Coin cc_glglue function was introduced with
  // Coin 2.3, so we can't use this without first separating out the
  // gl-wrapper, as planned. 20040714 mortene.

  // FIXME: this design is a bit bogus. Consider this: the size can be
  // set in one GL context, but the tex-cube can later be attempted
  // used in another GL context, with a smaller max size. Not sure how
  // to fix this yet. 20041221 mortene.

  GLint maxsize = -1;
  glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &maxsize);

  // This has been reported by an external developer to hit on an ATI
  // OpenGL driver on a Linux system. As 3D texture based rendering
  // otherwise seemed to work just fine, we simply warn, correct the
  // problem, and go on.
  if (maxsize == -1) {
    static const char CVR_IGNORE_ATI_QUERY_BUG[] = "CVR_IGNORE_ATI_QUERY_BUG";
    const char * env = coin_getenv(CVR_IGNORE_ATI_QUERY_BUG);

    static SbBool first = TRUE;
    if (first && (env == NULL)) {
      SoDebugError::postWarning("Cvr3DTexCube::clampSubCubeSize",
                                "Obscure bug found with your OpenGL driver. "
                                "If you are employed by Systems in Motion, "
                                "report this occurrence to <mortene@sim.no> "
                                "for further debugging. Otherwise, you can "
                                "safely ignore this warning. (Set the "
                                "environment variable '%s' on the system to "
                                "not get this notification again.)",
                                CVR_IGNORE_ATI_QUERY_BUG);
    }
    first = FALSE;
    maxsize = 128; // this should be safe
  }

  if (CvrUtil::doDebugging()) {
    SoDebugError::postInfo("Cvr3DTexCube::clampSubCubeSize",
                           "GL_MAX_3D_TEXTURE_SIZE==%d", maxsize);
  }

  const char * envstr = coin_getenv("CVR_FORCE_SUBCUBE_SIZE");
  if (envstr) {
    short forcedsubcubesize = atoi(envstr);
    assert(forcedsubcubesize > 0);
    assert(forcedsubcubesize <= maxsize && "subcube size must be <= than max 3D texture size");
    assert(coin_is_power_of_two(forcedsubcubesize) && "subcube size must be power of two");
    return SbVec3s(forcedsubcubesize, forcedsubcubesize, forcedsubcubesize);
  }

  // FIXME: My GeforceFX 5600 card sometime fails when asking for 512 as
  // cube size even if it is supposed to handle it. (20040302 handegar)
  //maxsize = SbMin(256, maxsize);

  assert((maxsize < SHRT_MAX) && "unsafe cast");
  const short smax = (short)maxsize;
  return SbVec3s(SbMin(size[0], smax), SbMin(size[1], smax), SbMin(size[2], smax));
}

