//  CSCIx229 library
//  Willem A. (Vlakkies) Schreuder
#include "CSCIx229.h"

//
//  Set projection
//
void Project(int perspective, double fov, double asp, double dim)
{
   //  Tell OpenGL we want to manipulate the projection matrix
   glMatrixMode(GL_PROJECTION);
   //  Undo previous transformations
   glLoadIdentity();
   //  Perspective transformation
   if (perspective > 0)
      gluPerspective(fov, asp, 0.01, 100);
   //  Orthogonal projection
   else
      glOrtho(-asp * dim, +asp * dim, -dim, +dim, -dim, 500);
   //  Switch to manipulating the model matrix
   glMatrixMode(GL_MODELVIEW);
   //  Undo previous transformations
   glLoadIdentity();
}
