#ifndef __biped_opengl_h__
#define __biped_opengl_h__

#if defined(__APPLE__)
  #include <OpenGL/gl.h>
  #include <OpenGL/glu.h>
  #include <GLUT/glut.h>
  //#include <GLUT/glui.h>
#else
  #include <GL/gl.h>
  #include <GL/glu.h>
  #include <GL/glut.h>
  #include <GL/glui.h>
#endif

#endif // __biped_opengl_h__
