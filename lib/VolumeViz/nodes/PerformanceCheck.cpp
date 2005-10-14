// TODO:
//
// - fix interface of gl_performance_timer() to handle multiple render callbacks
// - don't pollute namespace with function typedefs
// - convert to C? would make inclusion in Coin's gl-abstraction layer easier
// - factor our the Coin gl-abstraction layer into its own CVS module

#include <stdlib.h>
#include <limits.h>
#include <float.h>

#include <Inventor/C/glue/gl.h>
#include <Inventor/C/tidbits.h>
#include <Inventor/SbTime.h>
#include <Inventor/lists/SbList.h>
#include <Inventor/errors/SoDebugError.h>

// *************************************************************************

typedef void pre_render_cb(const cc_glglue * g, void * userdata);
typedef void render_cb(const cc_glglue * g, void * userdata);
typedef void post_render_cb(const cc_glglue * g, void * userdata);

// *************************************************************************

static SbBool
debug_gl_timing(void)
{
  static int do_debugging = -1;
  if (do_debugging == -1) {
    const char * envstr = coin_getenv("COIN_DEBUG_GL_PERFORMANCE");
    do_debugging = envstr && atoi(envstr) > 0;
  }
  return do_debugging == 1 ? TRUE : FALSE;
}

// *************************************************************************

static double
get_average_performance_time(const SbList<double> & l)
{
  assert(l.getLength() > 0);
  if (l.getLength() == 1) { return l[0]; }

  unsigned int i;

  // Find indices to the highest and lowest value in the array, in
  // case e.g. the initial uploading of textures took especially much
  // time:
  unsigned int idxhighest = UINT_MAX, idxlowest = UINT_MAX;
  if (l.getLength() >= 4) {
    double highest = -DBL_MAX, lowest = DBL_MAX;
    for (i = 0; i < (unsigned int)l.getLength(); ++i) {
      if (l[i] < lowest) {
        idxlowest = i;
        lowest = l[i];
      }
    }
    assert(idxlowest != UINT_MAX);

    for (i = 0; i < (unsigned int)l.getLength(); ++i) {
      if (l[i] > highest) {
        idxhighest = i;
        highest = l[i];
      }
    }
    assert(idxhighest != UINT_MAX);

    if (debug_gl_timing()) {
      SoDebugError::postInfo("get_average_performance_time",
                             "worst, best, ratio w/b: %f, %f, %f",
                             highest, lowest, highest / lowest);
    }
  }

  double sum = 0;
  for (i = 0; i < (unsigned int)l.getLength(); ++i) {
    if (i == idxlowest) { continue; }
    if (i == idxhighest) { continue; }
    sum += l[i];
  }
  return (sum / l.getLength());
}

// *************************************************************************

/*
  FIXME: some general usage docs here. 20051014 mortene.

  Be aware that this is not meant to be used for measuring cases where
  there are small differences in performance between competing
  techiques due to e.g. better pipeline parallelization or other such,
  rather marginal, effects.

  This is supposed to be used to smoke out systems/drivers where there
  are *major* performance hits suffered for OpenGL techniques that are
  available, but not optimal for use. This can for instance happen if
  a technique is available in the driver, but not hardware
  accelerated.  Good case in point: 3D texturing is available in all
  OpenGL 1.2+ drivers, but often not hardware accelerated on many
  systems.  Or perhaps the GL feature we want to measure / check is
  just known to be really badly implemented on systems, or to have one
  or more bugs which causes major slowdowns.
*/
static const SbTime
gl_performance_timer(const cc_glglue * glue,
                     const unsigned int nrrendercbs,
                     const render_cb * rendercbs[],
                     double averagerendertime[],
                     pre_render_cb * precb = NULL,
                     post_render_cb * postcb = NULL,
                     const unsigned int maxruns = 10,
                     const SbTime maxtime = SbTime(0.5),
                     void * userdata = NULL)
{
  const SbTime starttime = SbTime::getTimeOfDay();

  glPushAttrib(GL_ALL_ATTRIB_BITS);

  if (precb) { (*precb)(glue, userdata); }

  glPixelTransferf(GL_RED_SCALE, 1.0f);  // Setting to initial values
  glPixelTransferf(GL_GREEN_SCALE, 1.0f);
  glPixelTransferf(GL_BLUE_SCALE, 1.0f);
  glPixelTransferf(GL_ALPHA_SCALE, 1.0f);
  glPixelTransferf(GL_RED_BIAS, 0.0f);
  glPixelTransferf(GL_GREEN_BIAS, 0.0f);
  glPixelTransferf(GL_BLUE_BIAS, 0.0f);
  glPixelTransferf(GL_ALPHA_BIAS, 0.0f);  
  glPixelZoom(1.0f, 1.0f);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
  
  // Save the framebuffer for later
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  const unsigned int size = viewport[2] * viewport[3] * 4;
  GLubyte * framebuf = new GLubyte[size];
  glReadPixels(0, 0, viewport[2], viewport[3], GL_RGBA, GL_UNSIGNED_BYTE, framebuf);

  glDepthMask(GL_FALSE); // Dont write to the depthbuffer. It wont be restored.
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glDisable(GL_CULL_FACE);
  glDisable(GL_BLEND);

  glViewport(0, 0, viewport[2], viewport[3]);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();


  // The following is a substitute for gluPerspective(...)
  const float fovy = 45.0f;
  const float zNear = 0.1f;
  const float zFar = 10.0f;
  const float aspect = (float) viewport[2] / (float) viewport[3];
  const double radians = fovy / 2 * M_PI / 180;
  const double deltaZ = zFar - zNear;
  const double sine = sin(radians);
  const double cotangent = cos(radians) / sine;
  GLdouble m[4][4];
  m[0][1] = 0; m[0][2] = 0; m[0][3] = 0;
  m[1][0] = 0; m[1][2] = 0; m[1][3] = 0;
  m[2][0] = 0; m[2][1] = 0;
  m[3][0] = 0; m[3][1] = 0;
  m[0][0] = cotangent / aspect;
  m[1][1] = cotangent;
  m[2][2] = -(zFar + zNear) / deltaZ;
  m[2][3] = -1;
  m[3][2] = -2 * zNear * zFar / deltaZ;
  m[3][3] = 0;
  glMultMatrixd(&m[0][0]); // Finished

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glTranslatef(0, 0, -0.5f); // Move camera a bit, so polygons can be
                             // rendered at z=0.
  
  static int dbgtimingrendering = -1;
  if (dbgtimingrendering == -1) {
    const char * envstr = coin_getenv("COIN_DEBUG_GL_TIMING_RENDERING");
    dbgtimingrendering = envstr && atoi(envstr) > 0;
  }

  if (dbgtimingrendering == 0) {
    glClearColor(0, 0, 1, 0);
    glClear(GL_COLOR_BUFFER_BIT);
  }
  
  // Make sure we don't drag along the full pipeline into the first
  // test run, by forcing completion of all GL commands currently
  // being processed.
  glFinish();

  SbList<double> timings[nrrendercbs];
 
  for (unsigned int i = 0; i < maxruns; ++i) {
    for (unsigned int j = 0; j < nrrendercbs; j++) {
      const SbTime start = SbTime::getTimeOfDay();
      (*(rendercbs[j]))(glue, userdata);
      glFinish();
      const SbTime now = SbTime::getTimeOfDay();
      const double t = (now - start).getValue();
      timings[j].append(t);

      if (debug_gl_timing()) {
        SoDebugError::postInfo("gl_performance_timer",
                               "run %u with cb %u done at %f (%f since last)",
                               i, j, now.getValue(), t);
      }
    }
            
    // Don't run the test for more than the maximum alloted.
    const SbTime t = SbTime::getTimeOfDay() - starttime;
    if (t > maxtime) { break; }
  }
  
  if (debug_gl_timing() && (nrrendercbs > 0)) {
    SoDebugError::postInfo("gl_performance_timer",
                           "managed %d runs with all render callbacks",
                           timings[0].getLength());
  }

  for (unsigned int j = 0; j < nrrendercbs; j++) {
    averagerendertime[j] = get_average_performance_time(timings[j]);
  }
 
  if (postcb) { (*postcb)(glue, userdata); }

  // Write back framebuffer
  if (TRUE /* && FALSE *//*<- for debugging*/) {
    glViewport(0, 0, viewport[2], viewport[3]);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, viewport[2], 0, viewport[3], -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
         
    glRasterPos2i(0, 0);
    glDisable(GL_DEPTH_TEST);
    glDrawPixels(viewport[2], viewport[3], GL_RGBA, GL_UNSIGNED_BYTE, framebuf);

  }
  delete[] framebuf;

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glPopAttrib(); 

  return SbTime::getTimeOfDay() - starttime;
}

// *************************************************************************
