/*
 *  Set projection
 */
#include "core.h"

void Project(double fov, double win_aspect, double cap_aspect, double dim)
{
   //  Tell OpenGL we want to manipulate the projection matrix
   glMatrixMode(GL_PROJECTION);
   //  Undo previous transformations
   glLoadIdentity();
   //  Perspective transformation
   if (fov)
      gluPerspective(fov, win_aspect, dim/16, 16*dim);
   //  Orthogonal transformation
   else if (win_aspect > 1)
        glOrtho(-win_aspect / cap_aspect, win_aspect / cap_aspect, -1, +1, 0.0, 1.0);
          else
        glOrtho(-1, +1, -cap_aspect / win_aspect, cap_aspect / win_aspect, 0.0, 1.0);

   //  Switch to manipulating the model matrix
   glMatrixMode(GL_MODELVIEW);
   //  Undo previous transformations
   glLoadIdentity();
}
