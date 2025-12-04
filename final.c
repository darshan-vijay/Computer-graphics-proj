/*
 *   Project F1 Racing Circuit
 *
 *  F1 Racing Circuit
 *
 *  Key bindings:
 *  p/P        Cycle through different Perspectives
 *  w/a/s/d    Move POV/Drive car (in POV mode)
 *  m/M        Cycle through different display modes
 *  n/N        Cycle through Day/Night modes
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
const char *textPers[] = {"Perspective", "Start line", "POV"};

// for Shaders rain and splash
typedef struct
{
   float xPos;
   float zPos;
   float speed;
   float length;
} DropData;

typedef struct
{
   float xPos;
   float zPos;
   float collisionTime;
} SplashData;

int numRainDrops = 7000;
int maxSplashes = 300;
float rainHeight = 30.0f;
float rainArea = 120.0f;

GLuint rainVBO;
GLuint splashVBO;
GLuint rainShader, splashShader;

SplashData *splashBuffer;
int splashWriteIndex = 0;
DropData *rainDrops = NULL;

// For rain animation
float rainTime = 0.0f;
float lastCheckTime = -1.0f;

float fogIntensity = 0.04f;

int dayNightMode = 1; // 0 = day, 1 = night
const char *textDayNight[] = {"Day", "Night"};
GLuint nightSky[6]; // Night skybox textures
GLuint mornSky[6];  // Morning skybox textures

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

double headingAngle = 0.0; // Actual direction car is facing (for movement)
double carVelocity = 0.0;  // Current forward velocity
double maxVelocity = 0.15; // Maximum velocity

double acceleration = 0.02;  // Acceleration rate
double deceleration = 0.008; // Deceleration/friction
double turnDegrees = 0.8;    // Degrees per key press

// Steering and braking
double steeringAngle = 0.0; // Current steering angle for front wheels
int isBraking = 0;          // Brake light state

double povX = 2;    // POV X
double povY = 0.45; // POV Y
double povZ = 0.5;  // POV Z

void updatePOVPosition()
{
   // Camera is positioned behind the car's current heading
   double radHeading = (90.0 + headingAngle) * M_PI / 180.0; // adding 90 because the car model faces +X initially
   double offsetDistance = 2.0;                              // Distance behind car

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

Mix_Music *rainBG;
// Mix_Chunk *engineStart;
Mix_Chunk *engineAcc;
int firstAcc = 0;
int engineChannel;
int engineAccChannel;

void reshape(SDL_Window *window)
{
   int width, height;
   SDL_GetWindowSize(window, &width, &height);
   // Ratio of the width to the height of the window
   asp = (height > 0) ? (double)width / height : 1;
   //  Set the viewport to the entire window
   glViewport(0, 0, width, height);
   //  Set projection
   Project(perspective, fov, asp, dim);
}

// This function calculates initial rain drop positions and speeds at random within a defined area
void calculateRainPositions()
{
   rainDrops = (DropData *)malloc(numRainDrops * sizeof(DropData)); // Allocate memory for rain drops

   // Calculate random positions and speeds
   for (int i = 0; i < numRainDrops; i++)
   {
      float rx = ((float)rand() / RAND_MAX) * 2.0f - 1.0f; // Random X between -1 and 1
      float rz = ((float)rand() / RAND_MAX) * 2.0f - 1.0f; // Random Z between -1 and 1

      rainDrops[i].xPos = rx * (rainArea * 0.5f); // Spread over -rainArea/2 till rainArea/2
      rainDrops[i].zPos = rz * (rainArea * 0.5f);
      rainDrops[i].speed = 8.0f + ((float)rand() / RAND_MAX) * 6.0f;  // from 8 to 14 units/sec
      rainDrops[i].length = 0.2f + ((float)rand() / RAND_MAX) * 0.6f; // from 0.2 to 0.8 units
   }

   glGenBuffers(1, &rainVBO);                                                                 // Generate VBO for rain drops
   glBindBuffer(GL_ARRAY_BUFFER, rainVBO);                                                    // Select the buffer
   glBufferData(GL_ARRAY_BUFFER, numRainDrops * sizeof(DropData), rainDrops, GL_STATIC_DRAW); // transfer data to GPU
   glBindBuffer(GL_ARRAY_BUFFER, 0);                                                          // Unbind the buffer

   // Splash buffer initialization
   splashBuffer = (SplashData *)calloc(maxSplashes, sizeof(SplashData));
   for (int i = 0; i < maxSplashes; i++)
   {
      splashBuffer[i].collisionTime = -999.0f;
   }
   // Setup splash VBO
   glGenBuffers(1, &splashVBO);
   glBindBuffer(GL_ARRAY_BUFFER, splashVBO);
   glBufferData(GL_ARRAY_BUFFER, maxSplashes * sizeof(SplashData), splashBuffer, GL_DYNAMIC_DRAW);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void checkForSplashes()
{
   if (lastCheckTime < 0)
   {
      lastCheckTime = rainTime;
      return;
   }

   float now = rainTime;
   float before = lastCheckTime;

   for (int i = 0; i < numRainDrops; i++)
   {
      float speed = rainDrops[i].speed;

      // position of drop before and now
      float beforeY = rainHeight - fmod(before * speed, rainHeight);
      float nowY = rainHeight - fmod(now * speed, rainHeight);

      // if it has crossed the ground level (Y=0) between last check and now
      if (beforeY > 0.5f && nowY <= 0.5f)
      { // Register a splash at this drop's X,Z position at current time
         splashBuffer[splashWriteIndex].xPos = rainDrops[i].xPos;
         splashBuffer[splashWriteIndex].zPos = rainDrops[i].zPos;
         splashBuffer[splashWriteIndex].collisionTime = now;

         splashWriteIndex = (splashWriteIndex + 1) % maxSplashes;
      }
   }

   lastCheckTime = now;

   // Send updated splashes to GPU
   glBindBuffer(GL_ARRAY_BUFFER, splashVBO);
   glBufferSubData(GL_ARRAY_BUFFER, 0, maxSplashes * sizeof(SplashData), splashBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void renderRain()
{
   glUseProgram(rainShader);
   // Send uniform variables
   glUniform1f(glGetUniformLocation(rainShader, "currTime"), rainTime);
   glUniform1f(glGetUniformLocation(rainShader, "height"), rainHeight);
   // Enable point sprites and point size from shader
   glEnable(GL_POINT_SPRITE);
   glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
   glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
   glEnable(GL_POINT_SMOOTH);
   // Enable blending for transparency
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   // Bind VBO and set vertex attribute
   glBindBuffer(GL_ARRAY_BUFFER, rainVBO);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(DropData), (void *)0);
   glDrawArrays(GL_POINTS, 0, numRainDrops);
   glDisableVertexAttribArray(0);
   glBindBuffer(GL_ARRAY_BUFFER, 0);

   glUseProgram(0);
}

void renderSplashes()
{
   glUseProgram(splashShader);
   // Send uniform variables
   glUniform1f(glGetUniformLocation(splashShader, "time"), rainTime);
   // Enable point sprites and point size from shader
   glEnable(GL_POINT_SPRITE);
   glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
   glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

   // Enable blending for transparency
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   // Bind VBO and set vertex attribute
   glBindBuffer(GL_ARRAY_BUFFER, splashVBO);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SplashData), (void *)0);
   glDrawArrays(GL_POINTS, 0, maxSplashes);
   glDisableVertexAttribArray(0);
   glBindBuffer(GL_ARRAY_BUFFER, 0);

   glUseProgram(0);
}

void ApplyFog()
{
   if (dayNightMode == 1) // Night mode - enable fog
   {
      if (!Mix_PlayingMusic())      // Play only once
         Mix_PlayMusic(rainBG, -1); // Loop rain

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
   else // Remove fog in day mode
   {
      Mix_FadeOutMusic(1000); // Fade out rain
      glDisable(GL_FOG);
   }
}

void DrawSkybox(float boxSize, GLuint *skyTextures)
{
   glPushMatrix();
   glScaled(boxSize, boxSize, boxSize);
   glColor3f(1, 1, 1);

   glPushAttrib(GL_ENABLE_BIT);

   glDisable(GL_FOG);
   glDisable(GL_LIGHTING);
   glDisable(GL_CULL_FACE);
   glDepthMask(GL_FALSE);
   glEnable(GL_TEXTURE_2D);

   // Side face  +X  px
   glBindTexture(GL_TEXTURE_2D, skyTextures[0]);
   glBegin(GL_QUADS);
   glTexCoord2f(1, 0);
   glVertex3f(1, -1, -1);
   glTexCoord2f(0, 0);
   glVertex3f(1, -1, 1);
   glTexCoord2f(0, 1);
   glVertex3f(1, 1, 1);
   glTexCoord2f(1, 1);
   glVertex3f(1, 1, -1);
   glEnd();

   // Side face -X nx
   glBindTexture(GL_TEXTURE_2D, skyTextures[1]);
   glBegin(GL_QUADS);
   glTexCoord2f(1, 0);
   glVertex3f(-1, -1, 1);
   glTexCoord2f(0, 0);
   glVertex3f(-1, -1, -1);
   glTexCoord2f(0, 1);
   glVertex3f(-1, 1, -1);
   glTexCoord2f(1, 1);
   glVertex3f(-1, 1, 1);
   glEnd();

   // Top face +Y py
   glBindTexture(GL_TEXTURE_2D, skyTextures[2]);
   glBegin(GL_QUADS);
   glTexCoord2f(0, 1);
   glVertex3f(-1, 1, -1);
   glTexCoord2f(1, 1);
   glVertex3f(1, 1, -1);
   glTexCoord2f(1, 0);
   glVertex3f(1, 1, 1);
   glTexCoord2f(0, 0);
   glVertex3f(-1, 1, 1);
   glEnd();

   // bottom face  -Y ny
   glBindTexture(GL_TEXTURE_2D, skyTextures[3]);
   glBegin(GL_QUADS);
   glTexCoord2f(0, 0);
   glVertex3f(-1, -1, 1);
   glTexCoord2f(1, 0);
   glVertex3f(1, -1, 1);
   glTexCoord2f(1, 1);
   glVertex3f(1, -1, -1);
   glTexCoord2f(0, 1);
   glVertex3f(-1, -1, -1);
   glEnd();

   // front face +Z pz
   glBindTexture(GL_TEXTURE_2D, skyTextures[4]);
   glBegin(GL_QUADS);
   glTexCoord2f(0, 0);
   glVertex3f(-1, -1, 1);
   glTexCoord2f(1, 0);
   glVertex3f(1, -1, 1);
   glTexCoord2f(1, 1);
   glVertex3f(1, 1, 1);
   glTexCoord2f(0, 1);
   glVertex3f(-1, 1, 1);
   glEnd();

   // back face -Z nz
   glBindTexture(GL_TEXTURE_2D, skyTextures[5]);
   glBegin(GL_QUADS);
   glTexCoord2f(0, 0);
   glVertex3f(1, -1, -1);
   glTexCoord2f(1, 0);
   glVertex3f(-1, -1, -1);
   glTexCoord2f(1, 1);
   glVertex3f(-1, 1, -1);
   glTexCoord2f(0, 1);
   glVertex3f(1, 1, -1);
   glEnd();

   glDepthMask(GL_TRUE);
   glPopAttrib();
   glPopMatrix();
}

/*
 *  SDL calls this routine to display the scene
 */
void display(SDL_Window *window)
{
   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);
   //  Undo previous
   glLoadIdentity();
   glUseProgram(0); // turn off shaders before skybox

   // Select skybox based on day/night mode
   GLuint *currentSky = (dayNightMode == 0) ? mornSky : nightSky;

   //  Set camera based on projection mode
   switch (perspective)
   {
   case 0: // Perspective
   {
      double Ex = -2 * dim * Sin(th) * Cos(ph);
      double Ey = +2 * dim * Sin(ph);
      double Ez = +2 * dim * Cos(th) * Cos(ph);
      gluLookAt(Ex, Ey, Ez, 0, 0, 0, 0, Cos(ph), 0);

      // Draw skybox at camera origin
      glPushMatrix();
      glTranslatef(Ex, Ey, Ez);
      DrawSkybox(70.0f, currentSky);
      glPopMatrix();

      fogIntensity = 0.04f;
      break;
   }

   case 1: // Start line view
   {
      // CAMERA
      gluLookAt(12, 4, 3, // camera position
                0, 0, 0,
                0, 1, 0);

      // Draw skybox at camera position
      glPushMatrix();
      glTranslatef(povX, povY, povZ);
      DrawSkybox(70.0f, currentSky);
      glPopMatrix();

      fogIntensity = 0.12f;
      break;
   }
   case 2: // POV view
   {
      gluLookAt(povX, povY, povZ, mclarenX, mclarenY + 0.2, mclarenZ, 0, 1, 0);

      // Draw skybox at camera position
      glPushMatrix();
      glTranslatef(povX, povY, povZ);
      DrawSkybox(70.0f, currentSky);
      glPopMatrix();

      fogIntensity = 0.12f;
      break;
   }
   }

   ApplyFog();
   glShadeModel(GL_SMOOTH);

   //  Light switch
   if (light)
   {

      float ambientLevel = (dayNightMode == 0) ? ambient : ambient * 0.5; // Dimmer at night
      float diffuseLevel = (dayNightMode == 0) ? diffuse : diffuse * 0.5;

      float Ambient[] = {0.01 * ambientLevel, 0.01 * ambientLevel, 0.01 * ambientLevel, 1.0};
      float Diffuse[] = {0.01 * diffuseLevel, 0.01 * diffuseLevel, 0.01 * diffuseLevel, 1.0};
      float Specular[] = {0.01 * specular, 0.01 * specular, 0.01 * specular, 1.0};
      //  Light position
      float Position[] = {distance * Cos(zh), ylight, distance * Sin(zh), 1.0};
      //  Draw light position as ball (still no lighting here)
      glColor3f(1, 1, 1);
      ball(Position[0], Position[1], Position[2], 0.1);
      //  Enable lighting with normalization
      glEnable(GL_NORMALIZE);
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
   case -1:

      glPushMatrix();
      drawSupportBanner(3.0, 6.0, 0.3, 5, barricadeTexture[2]);
      glPopMatrix();
      break;

   case 0:
      // first stand near start line
      glPushMatrix();
      glTranslated(5, 0, -3.5);
      glRotatef(180, 0, 1, 0);
      glScalef(1.0f, 1.0f, 1.0f);
      drawGrandStand();
      glPopMatrix();
      // second Stand
      glPushMatrix();
      glTranslated(22, 0, -3.5);
      glRotatef(180, 0, 1, 0);
      glScalef(1.0f, 1.0f, 1.0f);
      drawGrandStand();
      glPopMatrix();
      // 3rd Stand
      glPushMatrix();
      glTranslated(35, 0, 10);
      glRotatef(90, 0, 1, 0);
      glScalef(1.0f, 1.0f, 1.0f);
      drawGrandStand();
      glPopMatrix();

      // 3rd Stand
      glPushMatrix();
      glTranslated(35, 0, 25);
      glRotatef(90, 0, 1, 0);
      glScalef(1.0f, 1.0f, 1.0f);
      drawGrandStand();
      glPopMatrix();

      // support banner with textures
      glPushMatrix();
      glTranslated(33, 0, 20);
      glRotatef(-90, 0, 1, 0);
      drawSupportBanner(3.0, 6.0, 0.3, 4, barricadeTexture[0]);
      glPopMatrix();

      // support banner with lights
      glPushMatrix();
      glTranslated(33, 0, 10);
      glRotatef(-90, 0, 1, 0);
      drawSupportBanner(3.0, 6.0, 0.3, 3, barricadeTexture[2]);
      glPopMatrix();

      // support banner with textures
      glPushMatrix();
      glTranslated(20, 0, -3);
      drawSupportBanner(3.0, 6.0, 0.3, 4, barricadeTexture[2]);
      glPopMatrix();

      // main start light
      glPushMatrix();
      glTranslated(12, 0, -3);
      drawSupportBanner(3.0, 6.0, 0.3, 5, barricadeTexture[2]);
      glPopMatrix();

      // support banner
      glPushMatrix();
      glTranslated(-10, 0, -3);
      drawSupportBanner(3.0, 6.0, 0.3, 4, barricadeTexture[1]);
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
      glRotated(headingAngle, 0, 1, 0); // heading direction
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

   // Only render rain in night mode
   if (dayNightMode == 1)
   {
      checkForSplashes(); // Detects ground hits
      renderRain();       // Draw rain
      renderSplashes();   // Draw splashes
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
   Print("Angle=%d,%d, Perspective=%s, Mode=%s, Time=%s, Velocity=%.2f, Heading=%.1f, Steering=%.1f",
         th, ph, textPers[perspective], text[mode], textDayNight[dayNightMode], carVelocity, headingAngle, steeringAngle);

   ErrCheck("display");
   glFlush();
   SDL_GL_SwapWindow(window);
}

void update()
{
   //  Elapsed time in seconds
   static Uint32 startTime = 0;
   if (startTime == 0)
      startTime = SDL_GetTicks();

   double t = (SDL_GetTicks() - startTime) / 1000.0;
   zh = fmod(90 * t, 360.0);

   // update for rain animation
   rainTime += 0.05;
   if (rainTime > 1000.0)
      rainTime = 0.0;

   // Get keyboard state for smooth controls
   const Uint8 *keys = SDL_GetKeyboardState(NULL);

   // Handle car driving in POV mode for each frame
   if (perspective == 2 && mode == 0)
   {
      int isAccelerating = 0;
      int isTurning = 0;
      if (keys[SDL_SCANCODE_SPACE]) // Forward with acceleration
      {
         // Mix_FadeOutChannel(engineChannel, 500);

         Mix_FadeOutChannel(engineAccChannel, 500);
         deceleration = 0.021; // Stronger deceleration when braking
         isBraking = 1;        // for brake lights
      }
      else
      {
         deceleration = 0.008; // Normal deceleration
         isBraking = 0;
      }
      if (keys[SDL_SCANCODE_W]) // Forward with acceleration
      {
         carVelocity += acceleration;
         if (carVelocity > 0 && firstAcc == 0)
         {
            // engineChannel = Mix_PlayChannel(0, engineStart, 0);  // play once
            engineAccChannel = Mix_PlayChannel(1, engineAcc, 0); // play once it start
            firstAcc = 1;
         }

         if (carVelocity > maxVelocity)
            carVelocity = maxVelocity;
         isAccelerating = 1;
      }
      if (keys[SDL_SCANCODE_S]) // Backward with acceleration
      {
         carVelocity -= acceleration;
         // Mix_FadeOutChannel(engineChannel, 500);
         Mix_FadeOutChannel(engineAccChannel, 1000);
         if (carVelocity < -maxVelocity * 0.5)
         {
            carVelocity = -maxVelocity * 0.5;
         }
         isBraking = 1;
         isAccelerating = 1;
      }

      // Steering - check if moving or has velocity
      if (keys[SDL_SCANCODE_A]) // Turn left
      {
         steeringAngle = -25.0;
         if (isAccelerating || fabs(carVelocity) > 0.01)
         {
            headingAngle += turnDegrees;
            if (headingAngle >= 360)
               headingAngle -= 360;
            isTurning = 1;
         }
      }
      else if (keys[SDL_SCANCODE_D]) // Turn right
      {
         steeringAngle = 25.0;
         if (isAccelerating || fabs(carVelocity) > 0.01)
         {
            headingAngle -= turnDegrees;
            if (headingAngle < 0)
               headingAngle += 360;
            isTurning = 1;
         }
      }

      // If not turning, gradually return steering to center
      if (!isTurning)
      {
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
      }
   }

   // Apply friction at every point in time
   if (carVelocity >= 0)
   {
      carVelocity -= deceleration;
      if (carVelocity < 0)
         carVelocity = 0;
   }
   else if (carVelocity < 0)
   {
      carVelocity += deceleration;
      if (carVelocity > 0)
      {
         Mix_FadeOutChannel(engineAccChannel, 200);
         carVelocity = 0;
         firstAcc = 0;
      }
   }

   // Update car position based on velocity
   if (fabs(carVelocity) > 0.001)
   {
      double radRot = (90.0 + headingAngle) * M_PI / 180.0;
      mclarenX += carVelocity * sin(radRot);
      mclarenZ += carVelocity * cos(radRot);
   }

   // POV positions moves according to car position
   updatePOVPosition();
}

/*
 *  Call this routine when a key is pressed
 *     Returns 1 to continue, 0 to exit
 */
int key()
{
   const Uint8 *keys = SDL_GetKeyboardState(NULL);

   //  Exit on ESC
   if (keys[SDL_SCANCODE_ESCAPE])
      return 0;
   //  Reset view angle
   else if (keys[SDL_SCANCODE_0])
   {
      th = 105;
      ph = 20;
   }
   //  Toggle Day/Night
   else if (keys[SDL_SCANCODE_N])
      dayNightMode = (dayNightMode + 1) % 2;
   //  Toggle Mode
   else if (keys[SDL_SCANCODE_M])
   {
      mode = (mode + 1) % 3;
      if (mode == 0)
         dim = 2;
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
   //  Switch projection mode
   else if (keys[SDL_SCANCODE_P])
      perspective = (perspective + 1) % 3;
   //  Toggle lighting
   else if (keys[SDL_SCANCODE_L])
      light = !light;
   //  Toggle axes
   else if (keys[SDL_SCANCODE_Q])
      axes = 1 - axes;
   //  Increase/decrease light height
   else if (keys[SDL_SCANCODE_LEFTBRACKET])
      ylight -= 0.1;
   else if (keys[SDL_SCANCODE_RIGHTBRACKET])
      ylight += 0.1;
   //  Increase/decrease asimuth
   else if (keys[SDL_SCANCODE_RIGHT])
      th += 5;
   else if (keys[SDL_SCANCODE_LEFT])
      th -= 5;
   //  PageUp key - increase dim
   else if (keys[SDL_SCANCODE_F1])
      dim += 0.1;
   //  PageDown key - decrease dim
   else if (keys[SDL_SCANCODE_F2] && dim > 1)
      dim -= 0.1;
   //  F3 key - toggle light distance
   else if (keys[SDL_SCANCODE_F3])
      distance = (distance == 1) ? 6 : 1;

   th %= 360;

   //  Update projection
   Project(perspective, fov, asp, dim);
   //  Return 1 to keep running
   return 1;
}
/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc, char *argv[])
{

   int run = 1;
   double t0 = 0;

   //  Initialize SDL
   SDL_Init(SDL_INIT_VIDEO);
   //  Set size, resizable and double buffering
   SDL_Window *window = SDL_CreateWindow("Darshan Vijayaraghavan F1", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 600, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
   if (!window)
      Fatal("Cannot create window\n");
   SDL_GL_CreateContext(window);
#ifdef USEGLEW
   //  Initialize GLEW
   if (glewInit() != GLEW_OK)
      Fatal("Error initializing GLEW\n");
#endif
   //  Set screen size
   reshape(window);

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

   // Day skybox
   mornSky[0] = LoadTexBMP("pxMorn.bmp"); // right
   mornSky[1] = LoadTexBMP("nxMorn.bmp"); // left
   mornSky[2] = LoadTexBMP("pyMorn.bmp"); // top
   mornSky[3] = LoadTexBMP("nyMorn.bmp"); // bottom
   mornSky[4] = LoadTexBMP("pzMorn.bmp"); // front
   mornSky[5] = LoadTexBMP("nzMorn.bmp"); // back

   // Night skybox
   nightSky[0] = LoadTexBMP("pxNight.bmp"); // right
   nightSky[1] = LoadTexBMP("nxNight.bmp"); // left
   nightSky[2] = LoadTexBMP("pyNight.bmp"); // top
   nightSky[3] = LoadTexBMP("nyNight.bmp"); // bottom
   nightSky[4] = LoadTexBMP("pzNight.bmp"); // front
   nightSky[5] = LoadTexBMP("nzNight.bmp"); // back

   // Initialize rain system
   calculateRainPositions();

   // Create rain shader and splash shader
   rainShader = CreateShaderProg("rain.vert", "rain.frag");
   splashShader = CreateShaderProg("splash.vert", "splash.frag");
   ErrCheck("init");

   //  Initialize audio
   Mix_Init(MIX_INIT_MP3);

   if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048))
      Fatal("Cannot initialize audio\n");
   Mix_AllocateChannels(8);
   //  Load "The Wall"
   rainBG = Mix_LoadMUS("rainBG.mp3");
   if (!rainBG)
      Fatal("Cannot load rainBG.mp3\n");
   // engineStart = Mix_LoadWAV("carEngine.mp3");
   // if (!engineStart)
   //    Fatal("Cannot load carEngine.mp3\n");
   engineAcc = Mix_LoadWAV("carAcc.mp3"); // yes, MP3 works if mpg123 enabled
   if (!engineAcc)
      Fatal("Cannot load carAcc.mp3\n");

   while (run)
   {
      //  Elapsed time in seconds
      double t = SDL_GetTicks() / 1000.0;
      //  Process all pending events
      SDL_Event event;
      while (SDL_PollEvent(&event))
         switch (event.type)
         {
         case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED)
            {
               SDL_SetWindowSize(window, event.window.data1, event.window.data2);
               reshape(window);
            }
            break;
         case SDL_QUIT:
            run = 0;
            break;

         case SDL_KEYDOWN:
            run = key();
            t0 = t + 0.5; // Wait 1/2 s before repeating
            break;
         default:
            //  Do nothing
            break;
         }
      //  Repeat key every 50 ms
      if (t - t0 > 0.05)
      {
         run = key();
         t0 = t;
      }
      update();
      //  Display
      display(window);
      //  Slow down display rate to about 100 fps by sleeping 5ms
      SDL_Delay(5);
   }
   SDL_Quit();
   return 0;
}