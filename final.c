/*
 *   Project F1 Racing Circuit
 *
 *  F1 Racing Circuit
 *
 *  Key bindings:
 *  p/P        Cycle through different Perspectives
 *  w/a/s/d    Move POV/Drive car (in POV mode)
 *  m/M        Cycle through different display modes
 *  q          Toggle axes
 *  arrows     Change view angle in perspective mode
 *  0          Reset view angle
 *  ESC        Exit
 *  [          Lower light
 *  ]          Higher light
 *  l,L        Toggle light
 *  F3         Toggle light distance
 */

#include "CSCIx229.h"
#include <time.h>

int th = 105;        //  Azimuth of view angle
int ph = 20;         //  Elevation of view angle
int axes = 1;        //  Display axes
int mode = 0;        //  What to display
int perspective = 0; // Perspective
int fov = 60;        //  Field of view (for perspective)
double asp = 1;      //  Aspect ratio
double dim = 8;      //  Size of world
const char *text[] = {"F1 Racing Circuit", "Stand", "Tree", "F1 Cars scene"};
const char *textPers[] = {"Perspective", "POV"};

int numRainDrops = 5000;

unsigned int rainVBO = 0;
int rainShader = 0;

float rainTime = 0.0f;
int useRain = 1;

float fogIntensity = 0.04f;

typedef struct
{
   float offsetX;
   float offsetZ;
   float speed;
   float length;
} Drop;

// Colors in order: Body, Fins, Halo
// Ferrari
float ferrariColors[3][3] = {
    {0.9, 0.0, 0.0}, // Bright Ferrari Red
    {0.0, 0.0, 0.0}, // Black
    {0.0, 0.0, 0.0}};

// McLaren
float mclarenColors[3][3] = {
    {1.0, 0.5, 0.0},  // Vibrant Orange
    {0.0, 0.4, 0.75}, // McLaren Blue
    {0.0, 0.4, 0.75}};

// Mercedes
float mercedesColors[3][3] = {
    {0.0, 0.85, 0.75},  // Petronas Teal
    {0.05, 0.05, 0.05}, // Black
    {0.05, 0.05, 0.05}};

// Red Bull Racing
float redBullColors[3][3] = {
    {0.0, 0.15, 0.45}, // Deep Navy Blue
    {1.0, 0.9, 0.0},   // Bright Yellow
    {1.0, 0.9, 0.0}};

// Aston Martin
float astonMartinColors[3][3] = {
    {0.0, 0.35, 0.25}, // Deep British Racing Green
    {0.0, 0.8, 0.4},   // Bright Lime Green accent
    {0.9, 0.75, 0.3}   // Gold
};

// McLaren car position and physics
double mclarenX = 4.0;
double mclarenY = 0.0;
double mclarenZ = 1;
double carHeading = 0.0;     // Actual direction car is facing (for movement)
double carVelocity = 0.1;    // Current forward velocity
double maxVelocity = 0.3;    // Maximum velocity
double acceleration = 0.06;  // Acceleration rate
double deceleration = 0.008; // Deceleration/friction
double turnSpeed = 2.5;      // Degrees per key press

// Steering and braking
double steeringAngle = 0.0; // Current steering angle for front wheels
int isBraking = 0;          // Brake light state

double povX = 2;    // POV X
double povY = 0.45; // POV Y
double povZ = 0.5;  // POV Z

void updatePOVPosition()
{
   // Camera is positioned behind the car's current heading
   double radHeading = (90.0 + carHeading) * M_PI / 180.0;
   double offsetDistance = 2.0; // Distance behind car

   povX = mclarenX - offsetDistance * sin(radHeading);
   povY = mclarenY + 0.45; // Height above car
   povZ = mclarenZ - offsetDistance * cos(radHeading);
}

// Light values
int light = 1;                    // Lighting
int one = 1;                      // Unit valuep
int distance = 6;                 // Light distance
int smooth = 1;                   // Smooth/Flat shading
int local = 1;                    // Local Viewer Model
int ambient = 50;                 // Ambient intensity (%)
int diffuse = 80;                 // Diffuse intensity (%)
int specular = 80;                // Specular intensity (%)
int zh = 90;                      // Light azimuth
float ylight = 4;                 // Elevation of light
unsigned int texture[11];         // Texture names
unsigned int barricadeTexture[5]; // Barricade Texture names

void reshape(int width, int height)
{
   // Ratio of the width to the height of the window
   asp = (height > 0) ? (double)width / height : 1;
   //  Set the viewport to the entire window
   glViewport(0, 0, width, height);
   //  Set projection
   Project(perspective, fov, asp, dim);
}

void setupRain()
{
   float rainArea = 120.0f; // centered square -120..120
   Drop *drops = (Drop *)malloc(numRainDrops * sizeof(Drop));

   srand(time(NULL));

   for (int i = 0; i < numRainDrops; i++)
   {
      // centered at (0,0)
      float rx = ((float)rand() / RAND_MAX) * 2.0f - 1.0f; // -1..1
      float rz = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;

      drops[i].offsetX = rx * (rainArea * 0.5f);
      drops[i].offsetZ = rz * (rainArea * 0.5f);

      drops[i].speed = 5.0f + ((float)rand() / RAND_MAX) * 6.0f;
      drops[i].length = 0.2f + ((float)rand() / RAND_MAX) * 0.6f;
   }

   glGenBuffers(1, &rainVBO);
   glBindBuffer(GL_ARRAY_BUFFER, rainVBO);
   glBufferData(GL_ARRAY_BUFFER,
                numRainDrops * sizeof(Drop),
                drops,
                GL_STATIC_DRAW);
   glBindBuffer(GL_ARRAY_BUFFER, 0);

   free(drops);
}

void renderRain()
{
   glUseProgram(rainShader);

   glUniform1f(glGetUniformLocation(rainShader, "uTime"), rainTime);
   glUniform1f(glGetUniformLocation(rainShader, "uHeight"), 30.0f);

   glUniform1f(glGetUniformLocation(rainShader, "windStrength"), 0.25f);
   glUniform1f(glGetUniformLocation(rainShader, "turbulenceAmp"), 0.12f);
   glUniform1f(glGetUniformLocation(rainShader, "turbulenceFreq"), 12.0f);
   glUniform1f(glGetUniformLocation(rainShader, "turbulenceSpeed"), 4.0f);

   glEnable(GL_POINT_SPRITE);
   glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
   glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
   glEnable(GL_POINT_SMOOTH);

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glDepthMask(GL_FALSE);

   glBindBuffer(GL_ARRAY_BUFFER, rainVBO);

   glEnableVertexAttribArray(0);
   glVertexAttribPointer(
       0, 4, GL_FLOAT, GL_FALSE,
       sizeof(Drop), (void *)0);

   glDrawArrays(GL_POINTS, 0, numRainDrops);

   glDisableVertexAttribArray(0);
   glBindBuffer(GL_ARRAY_BUFFER, 0);

   glDepthMask(GL_TRUE);
   glUseProgram(0);
}

void ApplyFog()
{
   glEnable(GL_FOG);

   GLfloat fogColor[4] = {0.65f, 0.70f, 0.75f, 1.0f};
   glFogfv(GL_FOG_COLOR, fogColor);

   if (perspective == 1)
   {
      glFogi(GL_FOG_MODE, GL_EXP2);
      glFogf(GL_FOG_DENSITY, 0.06f);
   }
   else
   {
      glFogi(GL_FOG_MODE, GL_LINEAR);
      glFogf(GL_FOG_START, 10.0f);
      glFogf(GL_FOG_END, 40.0f);
   }
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
   ApplyFog();
   //  Set camera based on projection mode
   switch (perspective)
   {
   case 0: // Perspective
   {
      double Ex = -2 * dim * Sin(th) * Cos(ph);
      double Ey = +2 * dim * Sin(ph);
      double Ez = +2 * dim * Cos(th) * Cos(ph);
      gluLookAt(Ex, Ey, Ez, 0, 0, 0, 0, Cos(ph), 0);
      fogIntensity = 0.04f;
      break;
   }
   case 1: // POV view - looking at car from behind
   {
      // Camera looks at the car (centered in view)
      gluLookAt(povX, povY, povZ, mclarenX, mclarenY + 0.2, mclarenZ, 0, 1, 0);
      fogIntensity = 0.12f;
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

      // Support banner
      glPushMatrix();
      glTranslated(12, 0, -3);
      drawSupportBanner(3, 5.5, 0.5);
      glPopMatrix();

      // Circuit with barricades
      glPushMatrix();
      glTranslated(-15, 0, 0);
      drawCircuit(texture, barricadeTexture, sizeof(barricadeTexture) / sizeof(barricadeTexture[0]), ferrariColors);
      glPopMatrix();

      // start marking 1
      glPushMatrix();
      glTranslated(6, 0, -1);
      squareBracketMarking();
      glPopMatrix();
      // car1
      glPushMatrix();
      glTranslated(6, 0, -1);
      glScaled(0.2, 0.2, 0.2);
      drawF1Car(1, 1, 1, texture, ferrariColors, 0, 0);
      glPopMatrix();

      // start marking 2
      glPushMatrix();
      glTranslated(4, 0, 1);
      squareBracketMarking();
      glPopMatrix();

      // McLaren car - moving car
      glPushMatrix();
      glTranslated(mclarenX, mclarenY, mclarenZ);
      glRotated(carHeading, 0, 1, 0); // heading direction
      glScaled(0.2, 0.2, 0.2);
      drawF1Car(1, 1, 1, texture, mclarenColors, steeringAngle, isBraking);
      glPopMatrix();

      // start marking 3
      glPushMatrix();
      glTranslated(2, 0, -1);
      squareBracketMarking();
      glPopMatrix();

      glPushMatrix();
      glTranslated(2, 0, -1);
      glScaled(0.2, 0.2, 0.2);
      drawF1Car(1, 1, 1, texture, mercedesColors, 0, 0);
      glPopMatrix();

      // start marking 4
      glPushMatrix();
      glTranslated(0, 0, 1);
      squareBracketMarking();
      glPopMatrix();

      glPushMatrix();
      glTranslated(0, 0, 1);
      glScaled(0.2, 0.2, 0.2);
      drawF1Car(1, 1, 1, texture, redBullColors, 0, 0);
      glPopMatrix();

      // start marking 5
      glPushMatrix();
      glTranslated(-2, 0, -1);
      squareBracketMarking();
      glPopMatrix();

      glPushMatrix();
      glTranslated(-2, 0, -1);
      glScaled(0.2, 0.2, 0.2);
      drawF1Car(1, 1, 1, texture, astonMartinColors, 0, 0);
      glPopMatrix();

      break;
   case 1:
      drawF1Garage(0, 0, 0, 1, texture, ferrariColors);
      break;

   case 2:
      drawF1Car(1, 1, 1, texture, ferrariColors, 0, 0);
      break;
   }
   renderRain();
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
   Print("Angle=%d,%d, Perspective=%s, Mode=%s, Velocity=%.2f, Heading=%.1f, Steering=%.1f", th, ph, textPers[perspective], text[mode], carVelocity, carHeading, steeringAngle);

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

   rainTime += 0.05;
   if (rainTime > 1000.0)
      rainTime = 0.0;

   // Apply velocity and friction
   if (carVelocity >= 0)
   {
      carVelocity -= deceleration;
      if (carVelocity < 0)
         carVelocity = 0;
      isBraking = 0; // Turn off brake light when coasting
   }
   else if (carVelocity < 0)
   {
      carVelocity += deceleration;
      if (carVelocity > 0)
         carVelocity = 0;
   }

   // Gradually return steering to center
   if (steeringAngle > 0)
   {
      steeringAngle -= 0.5;
      if (steeringAngle < 0)
         steeringAngle = 0;
   }
   else if (steeringAngle < 0)
   {
      steeringAngle += 0.5;
      if (steeringAngle > 0)
         steeringAngle = 0;
   }

   // Update car position based on velocity in car's facing direction
   if (fabs(carVelocity) > 0.001)
   {
      // Use car's heading for movement direction
      double radRot = (90.0 + carHeading) * M_PI / 180.0;
      mclarenX += carVelocity * sin(radRot);
      mclarenZ += carVelocity * cos(radRot);
   }

   // Always update POV position to follow car
   updatePOVPosition();

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
   //  Exit on ESC
   if (ch == 27)
      exit(0);
   //  Reset view angle
   else if (ch == '0')
   {
      th = -50;
      ph = 15;
   }
   else if (ch == 'r' || ch == 'R')
      useRain = !useRain;
   //  Cycle through different modes
   else if (ch == 'm' || ch == 'M')
   {
      mode = (mode + 1) % 3;

      // Adjust viewing distance based on mode
      if (mode == 0)
      {
         dim = 8;
      }
      else if (mode == 1)
      {
         dim = 10;
         th = -25;
         ph = 10;
      }
      else
      {
         dim = 4;
         th = -125;
         ph = 15;
      }
   }
   //  Cycle through different perspectives
   else if (ch == 'p' || ch == 'P')
   {
      perspective = (perspective + 1) % 2;
   }
   //  Light elevation
   else if (ch == '[')
      ylight -= 0.1;
   else if (ch == ']')
      ylight += 0.1;
   //  Toggle light
   else if (ch == 'l' || ch == 'L')
      light = !light;

   // Handle car driving in POV mode
   if (perspective == 1 && mode == 0)
   {
      if (ch == 'w' || ch == 'W') // Forward with acceleration
      {
         carVelocity += acceleration;
         if (carVelocity > maxVelocity)
            carVelocity = maxVelocity;
         isBraking = 0;
      }
      else if (ch == 's' || ch == 'S') // Backward with acceleration
      {
         carVelocity -= acceleration;
         if (carVelocity < -maxVelocity * 0.5)
            carVelocity = -maxVelocity * 0.5;
         isBraking = 1; // Braking
      }
      else if (ch == 'a' || ch == 'A') // Turn left
      {
         carHeading += turnSpeed;
         if (carHeading >= 360)
            carHeading -= 360;
         steeringAngle = -25.0; // Turn wheels left
      }
      else if (ch == 'd' || ch == 'D') // Turn right
      {
         carHeading -= turnSpeed;
         if (carHeading < 0)
            carHeading += 360;
         steeringAngle = 25.0; // Turn wheels right
      }
   }
   else
   {
      perspective = 0; // Force perspective if not in circuit mode
   }

   // Handle axes toggle
   if (ch == 'q')
      axes = 1 - axes;

   Project(perspective, fov, asp, dim);
   glutPostRedisplay();
}

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
   glutCreateWindow("Project darshan vijayaraghavan");

   texture[0] = LoadTexBMP("asphalt.bmp");    // Track texture
   texture[1] = LoadTexBMP("concrete.bmp");   // Building texture
   texture[2] = LoadTexBMP("grass.bmp");      // grass texture
   texture[3] = LoadTexBMP("curb.bmp");       // curb texture
   texture[4] = LoadTexBMP("bark.bmp");       // bark texture
   texture[5] = LoadTexBMP("bush.bmp");       // bush texture
   texture[6] = LoadTexBMP("yellowside.bmp"); // yellow side texture
   texture[7] = LoadTexBMP("violetside.bmp"); // violet side texture
   texture[8] = LoadTexBMP("fireside.bmp");   // fire side texture

   barricadeTexture[0] = LoadTexBMP("pirelli.bmp"); // pirelli texture
   barricadeTexture[1] = LoadTexBMP("redbull.bmp"); // redbull texture
   barricadeTexture[2] = LoadTexBMP("nvidia.bmp");  // nvidia texture

   // Initialize rain system
   setupRain();

   // Create rain shader (works on macOS without GLEW)
   printf("Creating rain shader...\n");
   rainShader = CreateShaderProg("rain.vert", "rain.frag");
   printf("Rain shader created: %d\n", rainShader);

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