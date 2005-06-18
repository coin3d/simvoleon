/*
  Misc info:

  - check out EXT_paletted_texture and EXT_shared_texture_palette
    (both found in 3Dlabs driver on ASK)

  - use glColorTable(GL_PROXY_COLOR_TABLE, ...) to check that
    colortable fits
 */

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

int glutwin;
GLuint texid = 0;

void
expose_cb(void)
{
}

void
reshape_cb(int w, int h)
{
  glViewport(0, 0, w, h);
}

int
max_colortable_size(void)
{
  unsigned int trywidth = 1;
  while (1) {
    int actualsize;

    glColorTable(GL_PROXY_TEXTURE_2D, /* target */
		 GL_RGBA, /* GL internalformat */
		 trywidth, /* nr of paletteentries */
		 GL_RGBA, /* palette entry format */
		 GL_UNSIGNED_BYTE, /* palette entry unit type */
		 NULL); /* data ptr */

    if (glGetError() == GL_TABLE_TOO_LARGE) {
      printf("too large, %d\n", trywidth);
      break;
    }

    glGetColorTableParameteriv(GL_PROXY_TEXTURE_2D,
			       GL_COLOR_TABLE_WIDTH,
			       (GLint *)&actualsize);
    if (actualsize != trywidth) {
      printf("got %d when asking for %d\n", actualsize, trywidth);
      break;
    }

    printf("%d ok\n", trywidth);

    trywidth *= 2;
  }

  trywidth /= 2;
  printf("returning %d\n", trywidth);
  return trywidth;
}

void
set_colortable(void)
{
  GLubyte palette[][4] = {
    { 0x80, 0x00, 0x00, 0xff },
    { 0x00, 0x00, 0x00, 0xff },
    { 0x00, 0x00, 0x00, 0xff },
  };

  glEnable(GL_COLOR_TABLE);

  glColorTable(GL_TEXTURE_2D, /* target */
	       GL_RGBA, /* GL internalformat */
               2, /* nr of paletteentries */
               GL_RGBA, /* palette entry format */
               GL_UNSIGNED_BYTE, /* palette entry unit type */
               palette); /* data ptr */

  assert(glGetError() == GL_NO_ERROR);
}


void
change_colortable(void)
{
  GLubyte palette[][4] = {
    { 0x00, 0x00, 0x00, 0xff },
  };

  static GLubyte col = 0x00;
  palette[0][1] = col;
  col = (col + 0x01) % 0xff;

  glColorSubTable(GL_TEXTURE_2D, /* target */
		  1, /* start */
		  1, /* count */
		  GL_RGBA, /* palette entry format */
		  GL_UNSIGNED_BYTE, /* palette entry unit type */
		  palette); /* data ptr */
  assert(glGetError() == GL_NO_ERROR);
}

void
init_texture(void)
{
  GLubyte image[] = {
    0x01, 0x00, 0x01, 0x00,
    0x00, 0x01, 0x00, 0x01,
    0x01, 0x00, 0x01, 0x00,
    0x01, 0x01, 0x00, 0x01,
  };

  glGenTextures(1, &texid);
  glBindTexture(GL_TEXTURE_2D, texid);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_COLOR_INDEX8_EXT, 4, 4, 0, 
               GL_COLOR_INDEX, GL_UNSIGNED_BYTE, image);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glEnable(GL_TEXTURE_2D);

  set_colortable();
}

void
idle_cb(void)
{
  if (texid == 0) init_texture();

  glClearColor(0.2, 0.4, 0.6, 0.0);
  glClear(GL_COLOR_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  change_colortable();

  glBindTexture(GL_TEXTURE_2D, texid);

  glBegin(GL_POLYGON);

  glTexCoord2f(0, 0);
  glVertex3f(-1.0, -0.5, 0.0);
                                
  glTexCoord2f(1, 0);
  glVertex3f(1.0, -0.5, 0.0);

  glTexCoord2f(1, 1);
  glVertex3f(1.0, 0.5, 0.0);          

  glTexCoord2f(0, 1);
  glVertex3f(-1.0, 0.5, 0.0);          

  glEnd();

  glutSwapBuffers();
}

int
main(int argc, char ** argv)
{
  glutInit(&argc, argv);

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

  glutInitWindowSize(512, 400);
  glutwin = glutCreateWindow("clut test");
  glutDisplayFunc(expose_cb);
  glutReshapeFunc(reshape_cb);

  glutIdleFunc(idle_cb);

  max_colortable_size();

  glutMainLoop();

  return 0;
}
