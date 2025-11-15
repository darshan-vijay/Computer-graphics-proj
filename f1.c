/*
 *  HW 5 Textures
 *
 *  F1 Cars
 *
 *  Key bindings:
 *  m/M        Cycle through different modes
 *  p/P        Cycle through different Perspectives
 *  w/a/s/d    Move POV (in POV mode)
 *  q          Toggle axes
 *  arrows     Change view angle
 *  0          Reset view angle
 *  ESC        Exit
 *  p/P        Cycle through different perspectives
 *  [          Lower light
 *  ]          Higher light
 *  l,L        Toggle light
 *  F3         Toggle light distance
 */

#include "CSCIx229.h"

#define NUM_CURVE_POINTS 50
#define NUM_ROTATIONS 50

int th = -50;        //  Azimuth of view angle
int ph = 25;         //  Elevation of view angle
int axes = 1;        //  Display axes
int mode = 0;        //  What to display
int perspective = 0; // Perspective
int fov = 60;        //  Field of view (for perspective)
double asp = 1;      //  Aspect ratio
double dim = 4;      //  Size of world
const char *text[] = {"F1 Racing Circuit", "Stand", "Tree", "F1 Cars scene"};
const char *textPers[] = {"Perspective", "POV"};

float ferrariColors[3][3] = {
    {0.8, 0.1, 0.1},    // Body: Ferrari Red
    {0.05, 0.05, 0.05}, // Fins: Black
    {0.7, 0.7, 0.75}    // Halo: Silver
};

double povX = 0;   // POV X
double povY = 1.5; // POV Y
double povZ = 0;   // POV Z

// Light values
int light = 1;            // Lighting
int one = 1;              // Unit valuep
int distance = 6;         // Light distance
int smooth = 1;           // Smooth/Flat shading
int local = 1;            // Local Viewer Model
int ambient = 50;         // Ambient intensity (%)
int diffuse = 80;         // Diffuse intensity (%)
int specular = 80;        // Specular intensity (%)
int zh = 90;              // Light azimuth
float ylight = 4;         // Elevation of light
unsigned int texture[10]; // Texture names

double P[3][3] = {
    {0.255, 0.0, 0.0},
    {-0.102, 0.0, 0.470},
    {-0.526, 0.0, 0.360}};

double curve[NUM_CURVE_POINTS + 1][3];

void reshape(int width, int height)
{
   // Ratio of the width to the height of the window
   asp = (height > 0) ? (double)width / height : 1;
   //  Set the viewport to the entire window
   glViewport(0, 0, width, height);
   //  Set projection
   Project(perspective, fov, asp, dim);
}

void EvaluateBezier(double t, double result[3])
{
   double t2 = t * t;
   double mt = 1.0 - t;
   double mt2 = mt * mt;

   // Quadratic Bezier: B(t) = (1-t)²P0 + 2(1-t)tP1 + t²P2
   double b0 = mt2;
   double b1 = 2.0 * mt * t;
   double b2 = t2;

   result[0] = b0 * P[0][0] + b1 * P[1][0] + b2 * P[2][0];
   result[1] = b0 * P[0][1] + b1 * P[1][1] + b2 * P[2][1];
   result[2] = b0 * P[0][2] + b1 * P[1][2] + b2 * P[2][2];
}

void DrawBezierLine()
{
   // Evaluate curve at all sample points
   for (int i = 0; i <= NUM_CURVE_POINTS; i++)
   {
      double t = i / (double)NUM_CURVE_POINTS;
      EvaluateBezier(t, curve[i]);
   }

   // Draw the curve
   glColor3f(1, 1, 0); // Yellow
   glLineWidth(3);
   glBegin(GL_LINE_STRIP);
   for (int i = 0; i <= NUM_CURVE_POINTS; i++)
      glVertex3f(curve[i][0], curve[i][1], curve[i][2]);
   glEnd();
   glLineWidth(1);
}

void DrawRotationObject()
{
   glColor3f(0.5, 0.8, 1.0); // Cyan

   // Rotate from -180 to 0 degrees
   for (int j = 0; j < NUM_ROTATIONS; j++)
   {
      double angle1 = -180.0 + (180.0 * j / NUM_ROTATIONS);
      double angle2 = -180.0 + (180.0 * (j + 1) / NUM_ROTATIONS);

      double theta1 = angle1 * M_PI / 180.0;
      double theta2 = angle2 * M_PI / 180.0;

      glBegin(GL_QUAD_STRIP);
      for (int i = 0; i <= NUM_CURVE_POINTS; i++)
      {
         double x = curve[i][0];
         double z = curve[i][2];

         // Calculate tangent to the curve (derivative approximation)
         double dx, dz;
         if (i < NUM_CURVE_POINTS)
         {
            dx = curve[i + 1][0] - curve[i][0];
            dz = curve[i + 1][2] - curve[i][2];
         }
         else
         {
            dx = curve[i][0] - curve[i - 1][0];
            dz = curve[i][2] - curve[i - 1][2];
         }

         // For surface of revolution around X-axis:
         // Normal = (-dz, -dx*sin(θ), dx*cos(θ))
         double nx = -dz;
         double ny1 = -dx * sin(theta1);
         double nz1 = dx * cos(theta1);

         double ny2 = -dx * sin(theta2);
         double nz2 = dx * cos(theta2);

         // Normalize normal 1
         double len1 = sqrt(nx * nx + ny1 * ny1 + nz1 * nz1);
         if (len1 > 0)
         {
            nx /= len1;
            ny1 /= len1;
            nz1 /= len1;
         }

         // Store normalized values for normal 1
         double nx1_norm = nx;
         double ny1_norm = ny1;
         double nz1_norm = nz1;

         // Normalize normal 2
         double nx2 = -dz;
         double len2 = sqrt(nx2 * nx2 + ny2 * ny2 + nz2 * nz2);
         if (len2 > 0)
         {
            nx2 /= len2;
            ny2 /= len2;
            nz2 /= len2;
         }

         // Rotated positions
         double y1 = -z * sin(theta1);
         double z1 = z * cos(theta1);

         double y2 = -z * sin(theta2);
         double z2 = z * cos(theta2);

         glNormal3f(-nx1_norm, -ny1_norm, -nz1_norm);
         glVertex3f(x, y1, z1);

         glNormal3f(-nx2, -ny2, -nz2);
         glVertex3f(x, y2, z2);
      }
      glEnd();
   }

   // === FILL THE CIRCULAR END CAP at x = -0.526 (last point) ===
   double end_x = curve[NUM_CURVE_POINTS][0]; // x = -0.526
   double end_z = curve[NUM_CURVE_POINTS][2]; // z = 0.360 (radius of circle)

   glBegin(GL_TRIANGLE_FAN);

   // Normal for end cap points in -X direction (facing away from origin)
   glNormal3f(-1, 0, 0);
   glVertex3f(end_x, 0, 0); // Center of the circle (on X-axis)

   // Draw circle around the X-axis
   for (int j = 0; j <= NUM_ROTATIONS; j++)
   {
      double angle = -180.0 + (180.0 * j / NUM_ROTATIONS);
      double theta = angle * M_PI / 180.0;

      double y = -end_z * sin(theta);
      double z = end_z * cos(theta);

      glNormal3f(-1, 0, 0);
      glVertex3f(end_x, y, z);
   }
   glEnd();
}

/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display()
{
   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glClearColor(0.53, 0.81, 0.98, 1.0); // Light sky blue background
   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);
   //  Undo previous
   glLoadIdentity();

   //  Set camera based on projection mode
   switch (perspective)
   {
   case 0: // Perspective
   {
      double Ex = -2 * dim * Sin(th) * Cos(ph);
      double Ey = +2 * dim * Sin(ph);
      double Ez = +2 * dim * Cos(th) * Cos(ph);
      gluLookAt(Ex, Ey, Ez, 0, 0, 0, 0, Cos(ph), 0);
      break;
   }

   case 1: // POV view
   {
      double radTh = th * M_PI / 180.0;
      double radPh = ph * M_PI / 180.0;
      double Cx = povX + cos(radPh) * sin(radTh);
      double Cy = povY + sin(radPh);
      double Cz = povZ + cos(radPh) * cos(radTh);
      gluLookAt(povX, povY, povZ, Cx, Cy, Cz, 0, 1, 0);
      break;
   }
   }
   glShadeModel(GL_SMOOTH);

   //  Light switch
   if (light)
   {
      //  Translate intensity to color vectors
      float Ambient[] = {0.01 * ambient, 0.01 * ambient, 0.01 * ambient, 1.0};
      float Diffuse[] = {0.01 * diffuse, 0.01 * diffuse, 0.01 * diffuse, 1.0};
      float Specular[] = {0.01 * specular, 0.01 * specular, 0.01 * specular, 1.0};
      //  Light position
      float Position[] = {distance * Cos(zh), ylight, distance * Sin(zh), 1.0};
      //  Draw light position as ball (still no lighting here)
      glColor3f(1, 1, 1);
      ball(Position[0], Position[1], Position[2], 0.1);
      //  OpenGL should normalize normal vectors
      glEnable(GL_NORMALIZE);
      //  Enable lighting
      glEnable(GL_LIGHTING);
      //  Location of viewer for specular calculations
      glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, local);
      //  glColor sets ambient and diffuse color materials
      glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
      glEnable(GL_COLOR_MATERIAL);
      //  Enable light 0
      glEnable(GL_LIGHT0);
      //  Set ambient, diffuse, specular components and position of light 0
      glLightfv(GL_LIGHT0, GL_AMBIENT, Ambient);
      glLightfv(GL_LIGHT0, GL_DIFFUSE, Diffuse);
      glLightfv(GL_LIGHT0, GL_SPECULAR, Specular);
      glLightfv(GL_LIGHT0, GL_POSITION, Position);
   }
   else
      glDisable(GL_LIGHTING);

   //  Decide what to draw
   switch (mode)
   {
   case 0:
      drawRoad(0, 0.02, 40, 4, 80, 0, texture);
   case 1:
      glDisable(GL_COLOR_MATERIAL);
      drawF1Garage(0, 0, 0, 1, texture);
      glEnable(GL_COLOR_MATERIAL);
      break;
   case 2:
      drawPitComplex(texture);
      break;
   case 3:
      glDisable(GL_COLOR_MATERIAL);
      drawF1Car(1, 1, 1, ferrariColors);
      glEnable(GL_COLOR_MATERIAL);
      break;
   }
   //  Draw axes - no lighting
   glDisable(GL_LIGHTING);
   glColor3f(1, 1, 1);
   if (axes)
   {
      const double len = 2.0; //  Length of axes
      glBegin(GL_LINES);
      glVertex3d(0.0, 0.0, 0.0);
      glVertex3d(len, 0.0, 0.0);
      glVertex3d(0.0, 0.0, 0.0);
      glVertex3d(0.0, len, 0.0);
      glVertex3d(0.0, 0.0, 0.0);
      glVertex3d(0.0, 0.0, len);
      glEnd();
      //  Label axes
      glRasterPos3d(len, 0.0, 0.0);
      Print("X");
      glRasterPos3d(0.0, len, 0.0);
      Print("Y");
      glRasterPos3d(0.0, 0.0, len);
      Print("Z");
   }
   //  Five pixels from the lower left corner of the window
   glWindowPos2i(5, 5);
   //  Print the text string
   Print("Angle=%d,%d, Perspective=%s, Mode=%s, Light Y=%.1f, Distance=%d", th, ph, textPers[perspective], text[mode], ylight, distance);

   ErrCheck("display");
   glFlush();
   glutSwapBuffers();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void idle()
{
   //  Elapsed time in seconds
   double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
   zh = fmod(90 * t, 360.0);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when an arrow key is pressed
 */
void special(int key, int x, int y)
{
   //  Right arrow key - increase angle by 5 degrees
   if (key == GLUT_KEY_LEFT)
      th += 5;
   //  Left arrow key - decrease angle by 5 degrees
   else if (key == GLUT_KEY_RIGHT)
      th -= 5;
   //  Up arrow key - increase elevation by 5 degrees
   else if (key == GLUT_KEY_UP)
      ph += 5;
   //  Down arrow key - decrease elevation by 5 degrees
   else if (key == GLUT_KEY_DOWN)
      ph -= 5;
   //  PageUp key - increase dim
   else if (key == GLUT_KEY_F1)
      dim += 0.1;
   //  PageDown key - decrease dim
   else if (key == GLUT_KEY_F2 && dim > 1)
      dim -= 0.1;
   //  F3 key - toggle light distance
   else if (key == GLUT_KEY_F3)
      distance = (distance == 1) ? 6 : 1;

   //  Keep angles to +/-360 degrees
   th %= 360;
   ph %= 360;
   //  Update projection
   Project(perspective, fov, asp, dim);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when a key is pressed
 */
void key(unsigned char ch, int x, int y)
{
   double step = 0.2; // movement speed
   //  Exit on ESC
   if (ch == 27)
      exit(0);
   //  Reset view angle
   else if (ch == '0')
   {
      th = -50;
      ph = 25;
   }
   //  Cycle through different modes
   //  Cycle through different modes
   else if (ch == 'm' || ch == 'M')
   {
      mode = (mode + 1) % 4;

      // Adjust viewing distance based on mode
      if (mode == 2) // Pit complex needs larger view
         dim = 12;
      else // Other modes are smaller
         dim = 10;
   }
   //  Cycle through different perspectives
   else if (ch == 'p' || ch == 'P')
   {
      perspective = (perspective + 1) % 2;
      if (perspective == 2) // Entering POV mode
      {
         // Set POV position to ground level at a good viewing spot
         th = 80;
         ph = 20;
         povX = 0;
         povY = 1.5;
         povZ = 0;
      }
   }
   //  Light elevation
   else if (ch == '[')
      ylight -= 0.1;
   else if (ch == ']')
      ylight += 0.1;
   //  Toggle light
   else if (ch == 'l' || ch == 'L')
      light = !light;
   // Handle POV movement only in POV mode
   if (perspective == 1)
   {
      // Calculate forward/backward
      double radTh = th * M_PI / 180.0;

      if (ch == 'w' || ch == 'W') // Forward
      {
         povX += step * sin(radTh);
         povZ += step * cos(radTh);
      }
      else if (ch == 's' || ch == 'S') // Backward
      {
         povX -= step * sin(radTh);
         povZ -= step * cos(radTh);
      }
      else if (ch == 'a' || ch == 'A') // Strafe left
      {
         povX += step * sin(radTh + M_PI / 2);
         povZ += step * cos(radTh + M_PI / 2);
      }
      else if (ch == 'd' && ch != 'D') // Strafe right
      {
         povX -= step * sin(radTh + M_PI / 2);
         povZ -= step * cos(radTh + M_PI / 2);
      }
   }

   // Handle axes toggle
   if (ch == 'q')
      axes = 1 - axes;
   Project(perspective, fov, asp, dim);
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when the window is resized
 */

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc, char *argv[])
{
   //  Initialize GLUT and process user parameters
   glutInit(&argc, argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitWindowSize(600, 600);
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);

   //  Create the window
   glutCreateWindow("hw6 darshan vijayaraghavan");

   texture[0] = LoadTexBMP("asphalt.bmp");    // Track texture
   texture[1] = LoadTexBMP("concrete.bmp");   // Building texture
   texture[2] = LoadTexBMP("grass.bmp");      // grass texture
   texture[3] = LoadTexBMP("curb.bmp");       // curb texture
   texture[4] = LoadTexBMP("bark.bmp");       // bark texture
   texture[5] = LoadTexBMP("bush.bmp");       // bush texture
   texture[6] = LoadTexBMP("yellowside.bmp"); // yellow side texture
   texture[7] = LoadTexBMP("violetside.bmp"); // violet side texture
   texture[8] = LoadTexBMP("fireside.bmp");   // fire side texture

#ifdef USEGLEW
   //  Initialize GLEW
   if (glewInit() != GLEW_OK)
      Fatal("Error initializing GLEW\n");
#endif
   //  Tell GLUT to call "display" when the scene should be drawn
   glutDisplayFunc(display);
   //  Tell GLUT to call "reshape" when the window is resized
   glutReshapeFunc(reshape);
   //  Tell GLUT to call "special" when an arrow key is pressed
   glutSpecialFunc(special);
   //  Tell GLUT to call "key" when a key is pressed
   glutKeyboardFunc(key);
   //  Tell GLUT to call "idle" when there is nothing else to do
   glutIdleFunc(idle);
   //  Pass control to GLUT so it can interact with the user
   ErrCheck("init");
   glutMainLoop();
   return 0;
}