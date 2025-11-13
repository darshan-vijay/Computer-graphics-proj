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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#ifdef USEGLEW
#include <GL/glew.h>
#endif
//  OpenGL with prototypes for glext
#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <GLUT/glut.h>
// Tell Xcode IDE to not gripe about OpenGL deprecation
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#else
#include <GL/glut.h>
#endif

int th = -50;        //  Azimuth of view angle
int ph = 25;         //  Elevation of view angle
int axes = 1;        //  Display axes
int mode = 0;        //  What to display
int perspective = 0; // Perspective
int fov = 60;        //  Field of view (for perspective)
double asp = 1;      //  Aspect ratio
double dim = 4;      //  Size of world
const char *text[] = {"F1 Racing Circuit", "Stand", "Tree", "F1 Cars scene"};
const char *textPers[] = {"Orthogonal", "Perspective", "POV"};

double povX = 0;   // POV X
double povY = 1.5; // POV Y
double povZ = 0;   // POV Z

// Light values
int light = 1;            // Lighting
int one = 1;              // Unit valuep
int distance = 6;         // Light distance
int inc = 10;             // Ball increment
int smooth = 1;           // Smooth/Flat shading
int local = 1;            // Local Viewer Model
int emission = 0;         // Emission intensity (%)
int ambient = 50;         // Ambient intensity (%)
int diffuse = 80;         // Diffuse intensity (%)
int specular = 80;        // Specular intensity (%)
int zh = 90;              // Light azimuth
float ylight = 4;         // Elevation of light
unsigned int texture[10]; // Texture names

float ferrariColors[3][3] = {
    {0.8, 0.1, 0.1},    // Body: Ferrari Red
    {0.05, 0.05, 0.05}, // Fins: Black
    {0.7, 0.7, 0.75}    // Halo: Silver
};
float mercedesColors[3][3] = {
    {0.6, 0.6, 0.65},   // Body: Silver
    {0.05, 0.05, 0.05}, // Fins: Black
    {0.0, 0.6, 0.6}     // Halo: Teal
};
float redBullColors[3][3] = {
    {0.1, 0.2, 0.6}, // Body: Dark Blue
    {1.0, 0.8, 0.0}, // Fins: Yellow
    {0.8, 0.1, 0.2}  // Halo: Red
};
float mclarenColors[3][3] = {
    {1.0, 0.5, 0.1}, // Body: Papaya Orange
    {0.0, 0.4, 0.8}, // Fins: McLaren Blue
    {0.8, 0.8, 0.85} // Halo: Chrome
};

double P[3][3] = {
    {0.255, 0.0, 0.0},
    {-0.102, 0.0, 0.470},
    {-0.526, 0.0, 0.360}};

#define NUM_CURVE_POINTS 50
#define NUM_ROTATIONS 50

double curve[NUM_CURVE_POINTS + 1][3];

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

//  Cosine and Sine in degrees
#define Cos(x) (cos((x) * 3.14159265 / 180))
#define Sin(x) (sin((x) * 3.14159265 / 180))

/*
 *  Convenience routine to output raster text
 *  Use VARARGS to make this more flexible
 */
#define LEN 8192 //  Maximum length of text string
void Print(const char *format, ...)
{
   char buf[LEN];
   char *ch = buf;
   va_list args;
   //  Turn the parameters into a character string
   va_start(args, format);
   vsnprintf(buf, LEN, format, args);
   va_end(args);
   //  Display the characters one at a time at the current raster position
   while (*ch)
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *ch++);
}

/*
 *  Check for OpenGL errors
 */
void ErrCheck(const char *where)
{
   int err = glGetError();
   if (err)
      fprintf(stderr, "ERROR: %s [%s]\n", gluErrorString(err), where);
}

/*
 *  Print message to stderr and exit
 */
void Fatal(const char *format, ...)
{
   va_list args;
   va_start(args, format);
   vfprintf(stderr, format, args);
   va_end(args);
   exit(1);
}

//
//  Reverse n bytes
//
static void Reverse(void *x, const int n)
{
   char *ch = (char *)x;
   for (int k = 0; k < n / 2; k++)
   {
      char tmp = ch[k];
      ch[k] = ch[n - 1 - k];
      ch[n - 1 - k] = tmp;
   }
}

//
//  Load texture from BMP file
//
unsigned int LoadTexBMP(const char *file)
{
   //  Open file
   FILE *f = fopen(file, "rb");
   if (!f)
      Fatal("Cannot open file %s\n", file);
   //  Check image magic
   unsigned short magic;
   if (fread(&magic, 2, 1, f) != 1)
      Fatal("Cannot read magic from %s\n", file);
   if (magic != 0x4D42 && magic != 0x424D)
      Fatal("Image magic not BMP in %s\n", file);
   //  Read header
   unsigned int dx, dy, off, k; // Image dimensions, offset and compression
   unsigned short nbp, bpp;     // Planes and bits per pixel
   if (fseek(f, 8, SEEK_CUR) || fread(&off, 4, 1, f) != 1 ||
       fseek(f, 4, SEEK_CUR) || fread(&dx, 4, 1, f) != 1 || fread(&dy, 4, 1, f) != 1 ||
       fread(&nbp, 2, 1, f) != 1 || fread(&bpp, 2, 1, f) != 1 || fread(&k, 4, 1, f) != 1)
      Fatal("Cannot read header from %s\n", file);
   //  Reverse bytes on big endian hardware (detected by backwards magic)
   if (magic == 0x424D)
   {
      Reverse(&off, 4);
      Reverse(&dx, 4);
      Reverse(&dy, 4);
      Reverse(&nbp, 2);
      Reverse(&bpp, 2);
      Reverse(&k, 4);
   }
   //  Check image parameters
   unsigned int max;
   glGetIntegerv(GL_MAX_TEXTURE_SIZE, (int *)&max);
   if (dx < 1 || dx > max)
      Fatal("%s image width %d out of range 1-%d\n", file, dx, max);
   if (dy < 1 || dy > max)
      Fatal("%s image height %d out of range 1-%d\n", file, dy, max);
   if (nbp != 1)
      Fatal("%s bit planes is not 1: %d\n", file, nbp);
   if (bpp != 24)
      Fatal("%s bits per pixel is not 24: %d\n", file, bpp);
   if (k != 0)
      Fatal("%s compressed files not supported\n", file);
#ifndef GL_VERSION_2_0
   //  OpenGL 2.0 lifts the restriction that texture size must be a power of two
   for (k = 1; k < dx; k *= 2)
      ;
   if (k != dx)
      Fatal("%s image width not a power of two: %d\n", file, dx);
   for (k = 1; k < dy; k *= 2)
      ;
   if (k != dy)
      Fatal("%s image height not a power of two: %d\n", file, dy);
#endif

   //  Allocate image memory
   unsigned int size = 3 * dx * dy;
   unsigned char *image = (unsigned char *)malloc(size);
   if (!image)
      Fatal("Cannot allocate %d bytes of memory for image %s\n", size, file);
   //  Seek to and read image
   if (fseek(f, off, SEEK_SET) || fread(image, size, 1, f) != 1)
      Fatal("Error reading data from image %s\n", file);
   fclose(f);
   //  Reverse colors (BGR -> RGB)
   for (k = 0; k < size; k += 3)
   {
      unsigned char temp = image[k];
      image[k] = image[k + 2];
      image[k + 2] = temp;
   }

   //  Sanity check
   ErrCheck("LoadTexBMP");
   //  Generate 2D texture
   unsigned int texture;
   glGenTextures(1, &texture);
   glBindTexture(GL_TEXTURE_2D, texture);
   //  Copy image
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, dx, dy, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
   if (glGetError())
      Fatal("Error in glTexImage2D %s %dx%d\n", file, dx, dy);
   //  Scale linearly when image size doesn't match
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

   //  Free image memory
   free(image);
   //  Return texture name
   return texture;
}

/*
 *  Set material properties
 */
void SetMaterial(float ambient_r, float ambient_g, float ambient_b,
                 float diffuse_r, float diffuse_g, float diffuse_b,
                 float specular_r, float specular_g, float specular_b,
                 float shininess)
{
   float amb[] = {ambient_r, ambient_g, ambient_b, 1.0};
   float dif[] = {diffuse_r, diffuse_g, diffuse_b, 1.0};
   float spe[] = {specular_r, specular_g, specular_b, 1.0};
   float emi[] = {0.0, 0.0, 0.0, 1.0};

   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, amb);
   glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, dif);
   glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);
   glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emi);
   glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
}

/*
 *  Draw vertex in polar coordinates with normal
 */
static void Vertex(double th, double ph)
{
   double x = Sin(th) * Cos(ph);
   double y = Cos(th) * Cos(ph);
   double z = Sin(ph);
   //  For a sphere at the origin, the position
   //  and normal vectors are the same
   glNormal3d(x, y, z);
   glVertex3d(x, y, z);
}

/*
 *  Set projection
 */
static void Project()
{
   //  Tell OpenGL we want to manipulate the projection matrix
   glMatrixMode(GL_PROJECTION);
   //  Undo previous transformations
   glLoadIdentity();
   //  Perspective transformation
   if (perspective > 0)
      gluPerspective(fov, asp, 0.01, 500);
   //  Orthogonal projection
   else
      glOrtho(-asp * dim, +asp * dim, -dim, +dim, -dim, 500);
   //  Switch to manipulating the model matrix
   glMatrixMode(GL_MODELVIEW);
   //  Undo previous transformations
   glLoadIdentity();
}

/*
 *  Draw a cube
 *     at (x,y,z)
 *     dimensions (dx,dy,dz)
 *     rotated th about the y axis
 */
static void cube(double x, double y, double z,
                 double dx, double dy, double dz,
                 double th,
                 int useTexture,
                 double texRepeatU,
                 double texRepeatV)
{
   glPushMatrix();
   glTranslated(x, y, z);
   glRotated(th, 0, 1, 0);
   glScaled(dx, dy, dz);

   glBegin(GL_QUADS);

   // Front
   glNormal3f(0, 0, 1);
   glTexCoord2f(0, 0);
   glVertex3f(-1, -1, 1);
   glTexCoord2f(texRepeatU, 0);
   glVertex3f(+1, -1, 1);
   glTexCoord2f(texRepeatU, texRepeatV);
   glVertex3f(+1, +1, 1);
   glTexCoord2f(0, texRepeatV);
   glVertex3f(-1, +1, 1);

   // Back
   glNormal3f(0, 0, -1);
   glTexCoord2f(0, 0);
   glVertex3f(+1, -1, -1);
   glTexCoord2f(texRepeatU, 0);
   glVertex3f(-1, -1, -1);
   glTexCoord2f(texRepeatU, texRepeatV);
   glVertex3f(-1, +1, -1);
   glTexCoord2f(0, texRepeatV);
   glVertex3f(+1, +1, -1);

   // Right
   glNormal3f(+1, 0, 0);
   glTexCoord2f(0, 0);
   glVertex3f(+1, -1, +1);
   glTexCoord2f(texRepeatU, 0);
   glVertex3f(+1, -1, -1);
   glTexCoord2f(texRepeatU, texRepeatV);
   glVertex3f(+1, +1, -1);
   glTexCoord2f(0, texRepeatV);
   glVertex3f(+1, +1, +1);

   // Left
   glNormal3f(-1, 0, 0);
   glTexCoord2f(0, 0);
   glVertex3f(-1, -1, -1);
   glTexCoord2f(texRepeatU, 0);
   glVertex3f(-1, -1, +1);
   glTexCoord2f(texRepeatU, texRepeatV);
   glVertex3f(-1, +1, +1);
   glTexCoord2f(0, texRepeatV);
   glVertex3f(-1, +1, -1);

   // Top
   glNormal3f(0, +1, 0);
   glTexCoord2f(0, 0);
   glVertex3f(-1, +1, +1);
   glTexCoord2f(texRepeatU, 0);
   glVertex3f(+1, +1, +1);
   glTexCoord2f(texRepeatU, texRepeatV);
   glVertex3f(+1, +1, -1);
   glTexCoord2f(0, texRepeatV);
   glVertex3f(-1, +1, -1);

   // Bottom
   glNormal3f(0, -1, 0);
   glTexCoord2f(0, 0);
   glVertex3f(-1, -1, -1);
   glTexCoord2f(texRepeatU, 0);
   glVertex3f(+1, -1, -1);
   glTexCoord2f(texRepeatU, texRepeatV);
   glVertex3f(+1, -1, +1);
   glTexCoord2f(0, texRepeatV);
   glVertex3f(-1, -1, +1);

   glEnd();

   glPopMatrix();
}

/*
 * Draw a torus (or partial torus)
 * centerX, centerY, centerZ - position
 * majorRadius - distance from center to tube center
 * minorRadius - tube radius
 * numMajor - segments around major circle
 * numMinor - segments around minor circle (tube)
 * startAngle - starting angle in degrees (0-360)
 * endAngle - ending angle in degrees (0-360)
 */
void drawTorus(double centerX, double centerY, double centerZ,
               double majorRadius, double minorRadius,
               int numMajor, int numMinor,
               double startAngle, double endAngle)
{
   glPushMatrix();
   glTranslated(centerX, centerY, centerZ);

   // Convert angles to radians
   double startRad = startAngle * M_PI / 180.0;
   double endRad = endAngle * M_PI / 180.0;
   double angleRange = endRad - startRad;

   for (int i = 0; i < numMajor; i++)
   {
      double u1 = startRad + angleRange * i / numMajor;
      double u2 = startRad + angleRange * (i + 1) / numMajor;

      glBegin(GL_QUAD_STRIP);
      for (int j = 0; j <= numMinor; j++)
      {
         double v = 2.0 * M_PI * j / numMinor;

         // First vertex (u1, v)
         double x1 = (majorRadius + minorRadius * cos(v)) * cos(u1);
         double y1 = (majorRadius + minorRadius * cos(v)) * sin(u1);
         double z1 = minorRadius * sin(v);

         // Normal at (u1, v)
         double nx1 = cos(v) * cos(u1);
         double ny1 = cos(v) * sin(u1);
         double nz1 = sin(v);

         glNormal3f(nx1, ny1, nz1);
         glVertex3f(x1, y1, z1);

         // Second vertex (u2, v)
         double x2 = (majorRadius + minorRadius * cos(v)) * cos(u2);
         double y2 = (majorRadius + minorRadius * cos(v)) * sin(u2);
         double z2 = minorRadius * sin(v);

         // Normal at (u2, v)
         double nx2 = cos(v) * cos(u2);
         double ny2 = cos(v) * sin(u2);
         double nz2 = sin(v);

         glNormal3f(nx2, ny2, nz2);
         glVertex3f(x2, y2, z2);
      }
      glEnd();
   }

   // === OPTIONAL: Draw end caps if not a full circle ===
   if (fabs(angleRange - 2.0 * M_PI) > 0.01) // Not a full torus
   {
      // Start cap at startAngle
      glBegin(GL_TRIANGLE_FAN);
      glNormal3f(-sin(startRad), cos(startRad), 0);                            // Normal perpendicular to cut
      glVertex3f(majorRadius * cos(startRad), majorRadius * sin(startRad), 0); // Center
      for (int j = 0; j <= numMinor; j++)
      {
         double v = 2.0 * M_PI * j / numMinor;
         double x = (majorRadius + minorRadius * cos(v)) * cos(startRad);
         double y = (majorRadius + minorRadius * cos(v)) * sin(startRad);
         double z = minorRadius * sin(v);
         glVertex3f(x, y, z);
      }
      glEnd();

      // End cap at endAngle
      glBegin(GL_TRIANGLE_FAN);
      glNormal3f(sin(endRad), -cos(endRad), 0);                            // Normal perpendicular to cut
      glVertex3f(majorRadius * cos(endRad), majorRadius * sin(endRad), 0); // Center
      for (int j = 0; j <= numMinor; j++)
      {
         double v = 2.0 * M_PI * j / numMinor;
         double x = (majorRadius + minorRadius * cos(v)) * cos(endRad);
         double y = (majorRadius + minorRadius * cos(v)) * sin(endRad);
         double z = minorRadius * sin(v);
         glVertex3f(x, y, z);
      }
      glEnd();
   }

   glPopMatrix();
}
// Draws a cube centered at (x, y, z) with scaling and rotation
// No texture binding or texture coordinates
static void noTexCube(double x, double y, double z,
                      double dx, double dy, double dz,
                      double th)
{
   glPushMatrix();

   // Apply transformations
   glTranslated(x, y, z);
   glRotated(th, 0, 1, 0);
   glScaled(dx, dy, dz);

   // Begin drawing faces
   glBegin(GL_QUADS);

   // --- Front (+Z)
   glNormal3f(0, 0, 1);
   glVertex3f(-1, -1, 1);
   glVertex3f(+1, -1, 1);
   glVertex3f(+1, +1, 1);
   glVertex3f(-1, +1, 1);

   // --- Back (-Z)
   glNormal3f(0, 0, -1);
   glVertex3f(+1, -1, -1);
   glVertex3f(-1, -1, -1);
   glVertex3f(-1, +1, -1);
   glVertex3f(+1, +1, -1);

   // --- Right (+X)
   glNormal3f(+1, 0, 0);
   glVertex3f(+1, -1, +1);
   glVertex3f(+1, -1, -1);
   glVertex3f(+1, +1, -1);
   glVertex3f(+1, +1, +1);

   // --- Left (-X)
   glNormal3f(-1, 0, 0);
   glVertex3f(-1, -1, -1);
   glVertex3f(-1, -1, +1);
   glVertex3f(-1, +1, +1);
   glVertex3f(-1, +1, -1);

   // --- Top (+Y)
   glNormal3f(0, +1, 0);
   glVertex3f(-1, +1, +1);
   glVertex3f(+1, +1, +1);
   glVertex3f(+1, +1, -1);
   glVertex3f(-1, +1, -1);

   // --- Bottom (-Y)
   glNormal3f(0, -1, 0);
   glVertex3f(-1, -1, -1);
   glVertex3f(+1, -1, -1);
   glVertex3f(+1, -1, +1);
   glVertex3f(-1, -1, +1);

   glEnd();

   glPopMatrix();
}

// Bottom rests on ground (y=0), height along +Y
static void trapezoid(double x, double y, double z,
                      double dx, double dy, double dz,    // scale
                      double thX, double thY, double thZ, // rotations
                      double topWidthX, double topWidthZ,
                      double bottomWidthX, double bottomWidthZ,
                      double height)
{
   glPushMatrix();

   // Transformations
   glTranslated(x, y, z);
   glRotated(thX, 1, 0, 0);
   glRotated(thY, 0, 1, 0);
   glRotated(thZ, 0, 0, 1);
   glScaled(dx, dy, dz);

   double bx = bottomWidthX;
   double bz = bottomWidthZ;
   double tx = topWidthX;
   double tz = topWidthZ;
   double h = height;

   // Draw
   glBegin(GL_QUADS);

   // --- Front (+Z)
   {
      double nx = 0;
      double ny = (bz - tz);
      double nz = 2.0;
      double len = sqrt(nx * nx + ny * ny + nz * nz);
      glNormal3f(nx / len, ny / len, nz / len);
   }
   glVertex3f(-bx, 0, bz);
   glVertex3f(bx, 0, bz);
   glVertex3f(tx, h, tz);
   glVertex3f(-tx, h, tz);

   // --- Back (-Z)
   {
      double nx = 0;
      double ny = (bz - tz);
      double nz = -2.0;
      double len = sqrt(nx * nx + ny * ny + nz * nz);
      glNormal3f(nx / len, ny / len, nz / len);
   }
   glVertex3f(bx, 0, -bz);
   glVertex3f(-bx, 0, -bz);
   glVertex3f(-tx, h, -tz);
   glVertex3f(tx, h, -tz);

   // --- Right (+X)
   {
      double nx = 2.0;
      double ny = (bx - tx);
      double nz = 0.0;
      double len = sqrt(nx * nx + ny * ny + nz * nz);
      glNormal3f(nx / len, ny / len, nz / len);
   }
   glVertex3f(bx, 0, bz);
   glVertex3f(bx, 0, -bz);
   glVertex3f(tx, h, -tz);
   glVertex3f(tx, h, tz);

   // --- Left (-X)
   {
      double nx = -2.0;
      double ny = (bx - tx);
      double nz = 0.0;
      double len = sqrt(nx * nx + ny * ny + nz * nz);
      glNormal3f(nx / len, ny / len, nz / len);
   }
   glVertex3f(-bx, 0, -bz);
   glVertex3f(-bx, 0, bz);
   glVertex3f(-tx, h, tz);
   glVertex3f(-tx, h, -tz);

   // --- Top face
   glNormal3f(0, 1, 0);
   glVertex3f(-tx, h, tz);
   glVertex3f(tx, h, tz);
   glVertex3f(tx, h, -tz);
   glVertex3f(-tx, h, -tz);

   // --- Bottom face
   glNormal3f(0, -1, 0);
   glVertex3f(-bx, 0, -bz);
   glVertex3f(bx, 0, -bz);
   glVertex3f(bx, 0, bz);
   glVertex3f(-bx, 0, bz);

   glEnd();
   glPopMatrix();
}

static void cylinder(double x, double y, double z,
                     double radius, double height,
                     int slices,
                     double thX, double thY, double thZ,
                     int useTexture,
                     double texRepeatU, double texRepeatV)
{
   glPushMatrix();

   // Position
   glTranslated(x, y, z);

   // Rotations around X, Y, Z
   glRotated(thX, 1, 0, 0);
   glRotated(thY, 0, 1, 0);
   glRotated(thZ, 0, 0, 1);

   // Cylinder Side
   glBegin(GL_QUAD_STRIP);
   for (int i = 0; i <= slices; i++)
   {
      double theta = 2.0 * M_PI * i / slices;
      double dx = radius * cos(theta);
      double dz = radius * sin(theta);
      double u = (double)i / slices * texRepeatU; // Texture U wraps around

      glNormal3d(cos(theta), 0, sin(theta));
      if (useTexture)
         glTexCoord2f(u, 0);
      glVertex3f(dx, -height / 2, dz); // bottom

      if (useTexture)
         glTexCoord2f(u, texRepeatV);
      glVertex3f(dx, height / 2, dz); // top
   }
   glEnd();

   // Top cap
   glBegin(GL_TRIANGLE_FAN);
   glNormal3d(0, 1, 0);
   if (useTexture)
      glTexCoord2f(0.5, 0.5);    // Center of texture
   glVertex3f(0, height / 2, 0); // center
   for (int i = 0; i <= slices; i++)
   {
      double theta = 2.0 * M_PI * i / slices;
      double dx = radius * cos(theta);
      double dz = radius * sin(theta);

      if (useTexture)
      {
         // Map circle to texture space (0.5, 0.5) = center
         double u = 0.5 + 0.5 * cos(theta);
         double v = 0.5 + 0.5 * sin(theta);
         glTexCoord2f(u, v);
      }
      glVertex3f(dx, height / 2, dz);
   }
   glEnd();

   // Bottom cap
   glBegin(GL_TRIANGLE_FAN);
   glNormal3d(0, -1, 0);
   if (useTexture)
      glTexCoord2f(0.5, 0.5);     // Center
   glVertex3f(0, -height / 2, 0); // center
   for (int i = 0; i <= slices; i++)
   {
      double theta = 2.0 * M_PI * i / slices;
      double dx = radius * cos(theta);
      double dz = radius * sin(theta);

      if (useTexture)
      {
         double u = 0.5 + 0.5 * cos(theta);
         double v = 0.5 + 0.5 * sin(theta);
         glTexCoord2f(u, v);
      }
      glVertex3f(dx, -height / 2, dz);
   }
   glEnd();

   glPopMatrix();
}
// Draw a triangular prism
void prism(double base, double height, double depth)
{
   double halfBase = base / 2.0;
   double halfDepth = depth / 2.0;

   // Front triangle (z = +halfDepth)
   glBegin(GL_TRIANGLES);
   glNormal3f(0, 0, 1);
   glVertex3f(-halfBase, -height / 2, halfDepth); // left
   glVertex3f(halfBase, -height / 2, halfDepth);  // right
   glVertex3f(0.0, height / 2, halfDepth);        // top
   glEnd();

   // Back triangle (z = -halfDepth)
   glBegin(GL_TRIANGLES);
   glNormal3f(0, 0, -1);
   glVertex3f(-halfBase, -height / 2, -halfDepth);
   glVertex3f(halfBase, -height / 2, -halfDepth);
   glVertex3f(0.0, height / 2, -halfDepth);
   glEnd();

   // Left side
   double len_left = sqrt(height * height + halfBase * halfBase);
   glBegin(GL_QUADS);
   glNormal3f(-height / len_left, halfBase / len_left, 0);
   glVertex3f(-halfBase, -height / 2, halfDepth);
   glVertex3f(-halfBase, -height / 2, -halfDepth);
   glVertex3f(0.0, height / 2, -halfDepth);
   glVertex3f(0.0, height / 2, halfDepth);
   glEnd();

   // Right side
   double len_right = sqrt(height * height + halfBase * halfBase);
   glBegin(GL_QUADS);
   glNormal3f(height / len_right, halfBase / len_right, 0);
   glVertex3f(halfBase, -height / 2, halfDepth);
   glVertex3f(halfBase, -height / 2, -halfDepth);
   glVertex3f(0.0, height / 2, -halfDepth);
   glVertex3f(0.0, height / 2, halfDepth);
   glEnd();

   // Bottom side
   glBegin(GL_QUADS);
   glNormal3f(0, -1, 0);
   glVertex3f(-halfBase, -height / 2, halfDepth);
   glVertex3f(halfBase, -height / 2, halfDepth);
   glVertex3f(halfBase, -height / 2, -halfDepth);
   glVertex3f(-halfBase, -height / 2, -halfDepth);
   glEnd();
}

// static void Canopy(double th, double ph)
// {
//    glNormal3d(Sin(th) * Cos(ph), Sin(ph), Cos(th) * Cos(ph));
//    glVertex3d(Sin(th) * Cos(ph), Sin(ph), Cos(th) * Cos(ph));
// }

static void rectangle(double x, double y, double z,    // center position
                      double w, double h,              // width and height
                      double rx, double ry, double rz) // rotations (deg) about x,y,z
{
   glPushMatrix();

   // Move to position
   glTranslated(x, y, z);

   // Apply rotations
   glRotated(rx, 1, 0, 0); // rotate about X
   glRotated(ry, 0, 1, 0); // rotate about Y
   glRotated(rz, 0, 0, 1); // rotate about Z

   // Draw filled rectangle (in XY plane, z=0)
   glBegin(GL_QUADS);
   glNormal3f(0, 0, 1);
   glVertex3f(-w / 2, -h / 2, 0); // bottom left
   glVertex3f(+w / 2, -h / 2, 0); // bottom right
   glVertex3f(+w / 2, +h / 2, 0); // top right
   glVertex3f(-w / 2, +h / 2, 0); // top left
   glEnd();

   glPopMatrix();
}

void drawF1Car(float length, float width, float breadth, float colors[][3])
{
   // colors array structure:
   // colors[0] = body color (main chassis)
   // colors[1] = fin/wing color (rear and front wings)
   // colors[2] = halo/reinforcement bar color
   // Derived colors (automatically calculated):
   // - Suspension: darker metallic gray
   // - Wheels: black rubber
   // - Cockpit: slightly darker body color with transparency effect

   // Scaling factors
   float scaleX = length;  // Length scaling (front to back)
   float scaleY = breadth; // Height scaling (vertical)
   float scaleZ = width;   // Width scaling (side to side)

   glPushMatrix();
   glScalef(scaleX, scaleY, scaleZ);

   // === MATERIAL SETTINGS ===
   // Base metal body - using body color
   SetMaterial(colors[0][0], colors[0][1], colors[0][2],
               colors[0][0] * 1.3, colors[0][1] * 1.3, colors[0][2] * 1.3,
               1.0, 1.0, 1.0, 100);

   // === BODY STRUCTURE ===
   // --- First trapezoid: aligned along X, base on ground
   glPushMatrix();
   trapezoid(0, 0, 0,
             1, 1, 1,
             0, 0, -90,
             0.2, 0.2,
             0.35, 0.35,
             1.5);
   glPopMatrix();

   // --- Second trapezoid: front taper
   glPushMatrix();
   glTranslated(1.45, 0, 0);
   glRotated(-8, 0, 0, 1);
   trapezoid(0, 0, 0,
             1, 1, 1,
             0, 0, -90,
             0.12, 0.12,
             0.21, 0.21,
             1.0);
   glPopMatrix();

   // --- Third trapezoid: nose tip
   glPushMatrix();
   glTranslated(2.42, -0.12, 0);
   glRotated(-15, 0, 0, 1);
   trapezoid(0, 0, 0,
             1, 1, 1,
             0, 0, -90,
             0.1, 0.1,
             0.12, 0.12,
             0.4);
   glPopMatrix();

   // --- Rear body: larger segment behind cockpit
   glPushMatrix();
   glTranslated(-1.5, 0, 0);
   trapezoid(0, 0, 0,
             1, 1, 1,
             0, 0, -90,
             0.35, 0.35,
             0.35, 1,
             1.5);
   glPopMatrix();

   // --- Rear cube section
   glPushMatrix();
   glTranslated(-2.25, 0, 0);
   noTexCube(0, 0, 0,
             0.75, 0.35, 1,
             0);
   glPopMatrix();

   // --- Rear connection trapezoid
   glPushMatrix();
   glTranslated(-4, 0, 0);
   trapezoid(0, 0, 0,
             1, 1, 1,
             0, 0, -90,
             0.35, 1,
             0.35, 0.4,
             1.0);
   glPopMatrix();

   // --- Rear cube end
   glPushMatrix();
   glTranslated(-4.3, 0, 0);
   noTexCube(0, 0, 0,
             0.4, 0.35, 0.4,
             0);
   glPopMatrix();

   // === REAR WINGS AND CONNECTORS ===
   // Using fin/wing color
   SetMaterial(colors[1][0], colors[1][1], colors[1][2],
               colors[1][0] * 1.2, colors[1][1] * 1.2, colors[1][2] * 1.2,
               0.8, 0.8, 0.8, 80);

   rectangle(-4.4, 1.08, 0, 0.45, 2.8, -90, 0, 0); // back fin
   rectangle(-4.4, 0.2, -0.39, 0.45, 1, 0, 0, 0);
   rectangle(-4.4, 0.2, 0.39, 0.45, 1, 0, 0, 0);
   rectangle(-4.4, 0.85, -0.6, 0.45, 0.6, -45, 0, 0);
   rectangle(-4.4, 0.85, 0.6, 0.45, 0.6, 45, 0, 0);

   // === FRONT WING ===
   rectangle(2.4, -0.15, 0.6, 0.45, 1.5, -90, 0, -18);
   rectangle(2.4, -0.15, -0.6, 0.45, 1.5, -90, 0, 18);

   // === SUSPENSION ARMS ===
   // Dark metallic gray for suspension
   SetMaterial(0.25, 0.25, 0.28, 0.4, 0.4, 0.43, 1.0, 1.0, 1.0, 100);

   // Front LEFT suspension (2 diagonal arms)
   cylinder(1, 0.11, -0.175, 0.025, 2.8, 20, 90, 10, -10, 0, 0, 0);
   cylinder(1, -0.11, -0.175, 0.025, 2.8, 20, 90, 10, 10, 0, 0, 0);
   // Front RIGHT suspension
   cylinder(1, 0.11, 0.175, 0.025, 2.8, 20, 90, 10, 10, 0, 0, 0);
   cylinder(1, -0.11, 0.175, 0.025, 2.8, 20, 90, 10, -10, 0, 0, 0);
   // Rear LEFT suspension
   cylinder(-4, 0.15, -0.175, 0.025, 2.8, 20, 90, 10, -10, 0, 0, 0);
   cylinder(-4, -0.15, -0.175, 0.025, 2.8, 20, 90, 10, 10, 0, 0, 0);
   // Rear RIGHT suspension
   cylinder(-4, 0.15, 0.175, 0.025, 2.8, 20, 90, 10, 10, 0, 0, 0);
   cylinder(-4, -0.15, 0.175, 0.025, 2.8, 20, 90, 10, -10, 0, 0, 0);

   // === WHEELS ===
   // Black rubber for tires
   SetMaterial(0.01, 0.01, 0.01, 0.05, 0.05, 0.05, 0.1, 0.1, 0.1, 5);
   // Front
   cylinder(1, 0, -1.35, 0.6, 0.6, 20, 90, 0, 0, 0, 0, 0);
   cylinder(1, 0, 1.35, 0.6, 0.6, 20, 90, 0, 0, 0, 0, 0);
   // Rear
   cylinder(-4, 0, -1.35, 0.6, 0.6, 20, 90, 0, 0, 0, 0, 0);
   cylinder(-4, 0, 1.35, 0.6, 0.6, 20, 90, 0, 0, 0, 0, 0);

   // === COCKPIT ===
   // Slightly darker body color for cockpit (more matte finish)
   SetMaterial(colors[0][0], colors[0][1], colors[0][2],
               colors[0][0] * 1.3, colors[0][1] * 1.3, colors[0][2] * 1.3,
               1.0, 1.0, 1.0, 100);

   glPushMatrix();
   glTranslated(-3.7, 0.15, 0);
   glRotatef(180, 0, 1, 0);
   glScalef(3, 2.2, 1);
   DrawBezierLine();     // Function 2: Draw the curve
   DrawRotationObject(); // Function 3: Draw rotated surface
   glPopMatrix();

   // --- Front cockpit trapezoid
   glPushMatrix();
   glTranslated(-1.3, 0.5, 0);
   glRotated(-25, 0, 0, 1);
   trapezoid(0, 0, 0,
             1, 1, 1,
             0, 0, -90,
             0.3, 0.3,
             0.2, 0.2,
             1.5);
   glPopMatrix();

   // === Halo reinforcement bars ===
   // Using halo/reinforcement color
   SetMaterial(colors[2][0], colors[2][1], colors[2][2],
               colors[2][0] * 1.4, colors[2][1] * 1.4, colors[2][2] * 1.4,
               1.2, 1.2, 1.2, 120);

   glPushMatrix();
   glTranslated(-1.8, 0.8, 0);
   glRotated(90, 1, 0, 0);
   glRotated(-5, 0, 1, 0);
   glScalef(1.4, 1, 0.5);
   drawTorus(0, 0, 0, .4, 0.06, 32, 10, 150, -150); // Halo ring
   glPopMatrix();

   // Top horizontal bar
   cylinder(-1.15, 0.36, 0, 0.04, 0.8, 4, 0, 0, 10, 0, 0, 0);

   glPopMatrix(); // End scaling transformation
}

// Complete F1 Garage Function with all details
// Complete F1 Garage Function - Bigger, Better Arranged
void drawF1Garage(double x, double y, double z, double scale)
{
   glPushMatrix();
   glTranslated(x, y, z);
   glScaled(scale, scale, scale);

   // Garage dimensions: 16 units wide, 12 units deep, 6 units tall
   // Front opening at Z = +6, back wall at Z = -6

   // === GARAGE FLOOR ===
   SetMaterial(0.15, 0.15, 0.15, 0.3, 0.3, 0.3, 0.1, 0.1, 0.1, 10);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, texture[1]); // Concrete texture
   glBegin(GL_QUADS);
   glNormal3f(0, 1, 0);
   glTexCoord2f(0, 0);
   glVertex3f(-8, 0, -6);
   glTexCoord2f(15, 0);
   glVertex3f(8, 0, -6);
   glTexCoord2f(15, 15);
   glVertex3f(8, 0, 6);
   glTexCoord2f(0, 15);
   glVertex3f(-8, 0, 6);
   glEnd();
   glDisable(GL_TEXTURE_2D);

   // === BACK WALL (at Z = -6) ===
   SetMaterial(0.2, 0.2, 0.22, 0.4, 0.4, 0.45, 0.1, 0.1, 0.1, 10);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, texture[1]);
   glBegin(GL_QUADS);
   glNormal3f(0, 0, 1);
   glTexCoord2f(0, 0);
   glVertex3f(-8, 0, -6);
   glTexCoord2f(15, 0);
   glVertex3f(8, 0, -6);
   glTexCoord2f(15, 8);
   glVertex3f(8, 6, -6);
   glTexCoord2f(0, 8);
   glVertex3f(-8, 6, -6);
   glEnd();
   glDisable(GL_TEXTURE_2D);

   // === SIDE WALLS ===
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, texture[1]);
   // Left wall (X = -8)
   glBegin(GL_QUADS);
   glNormal3f(1, 0, 0);
   glTexCoord2f(0, 0);
   glVertex3f(-8, 0, -6);
   glTexCoord2f(15, 0);
   glVertex3f(-8, 0, 6);
   glTexCoord2f(15, 8);
   glVertex3f(-8, 6, 6);
   glTexCoord2f(0, 8);
   glVertex3f(-8, 6, -6);
   glEnd();
   // Right wall (X = +8)
   glBegin(GL_QUADS);
   glNormal3f(-1, 0, 0);
   glTexCoord2f(0, 0);
   glVertex3f(8, 0, 6);
   glTexCoord2f(15, 0);
   glVertex3f(8, 0, -6);
   glTexCoord2f(15, 8);
   glVertex3f(8, 6, -6);
   glTexCoord2f(0, 8);
   glVertex3f(8, 6, 6);
   glEnd();
   glDisable(GL_TEXTURE_2D);

   // === CEILING ===
   SetMaterial(0.25, 0.25, 0.25, 0.5, 0.5, 0.5, 0.2, 0.2, 0.2, 20);
   glBegin(GL_QUADS);
   glNormal3f(0, -1, 0);
   glVertex3f(-8, 6, -6);
   glVertex3f(8, 6, -6);
   glVertex3f(8, 6, 6);
   glVertex3f(-8, 6, 6);
   glEnd();

   // === OVERHEAD LIGHTING PANELS ===
   SetMaterial(0.9, 0.9, 0.85, 1.0, 1.0, 0.95, 0.8, 0.8, 0.8, 80);
   for (int i = 0; i < 4; i++)
   {
      for (int j = 0; j < 3; j++)
      {
         double lx = -6 + i * 4;
         double lz = -4 + j * 4;
         glBegin(GL_QUADS);
         glNormal3f(0, -1, 0);
         glVertex3f(lx - 0.9, 5.95, lz - 0.6);
         glVertex3f(lx + 0.9, 5.95, lz - 0.6);
         glVertex3f(lx + 0.9, 5.95, lz + 0.6);
         glVertex3f(lx - 0.9, 5.95, lz + 0.6);
         glEnd();
      }
   }

   // === IMPROVED TOOL CABINETS (Left side, back) ===
   // Main cabinet body (properly sized to fit wall)
   SetMaterial(0.7, 0.1, 0.1, 0.9, 0.2, 0.2, 0.3, 0.3, 0.3, 40);
   noTexCube(-6, 1.5, -4.5, 1.2, 1.5, 2.0, 0);

   // Cabinet top surface
   SetMaterial(0.5, 0.5, 0.5, 0.7, 0.7, 0.7, 0.4, 0.4, 0.4, 50);
   noTexCube(-6, 3.05, -4.5, 1.25, 0.08, 2.05, 0);

   // Cabinet drawers (properly arranged)
   SetMaterial(0.5, 0.08, 0.08, 0.7, 0.15, 0.15, 0.2, 0.2, 0.2, 30);
   for (int i = 0; i < 6; i++)
   {
      noTexCube(-5.4, 0.5 + i * 0.48, -4.5, 1.08, 0.22, 1.9, 0);
   }

   // Cabinet handles (silver, centered on drawers)
   SetMaterial(0.5, 0.5, 0.55, 0.8, 0.8, 0.85, 1.0, 1.0, 1.0, 100);
   for (int i = 0; i < 6; i++)
   {
      cylinder(-4.5, 0.5 + i * 0.48, -4.5, 0.025, 0.3, 8, 90, 0, 0, 0, 0, 0);
   }

   // === IMPROVED WORKBENCH (Right side, back - properly fitted) ===
   // Workbench top (longer and more realistic)
   SetMaterial(0.4, 0.35, 0.3, 0.6, 0.5, 0.45, 0.1, 0.1, 0.1, 10);
   noTexCube(6.2, 1.2, -4.5, 1.5, 0.1, 2.0, 0);

   // Workbench side panels
   SetMaterial(0.35, 0.3, 0.25, 0.55, 0.45, 0.4, 0.1, 0.1, 0.1, 10);
   noTexCube(4.8, 0.65, -4.5, 0.1, 0.65, 1.9, 0);
   noTexCube(7.6, 0.65, -4.5, 0.1, 0.65, 1.9, 0);

   // Workbench back panel
   noTexCube(6.2, 0.65, -5.8, 1.4, 0.65, 0.1, 0);

   // Legs (properly positioned at corners)
   SetMaterial(0.2, 0.2, 0.22, 0.4, 0.4, 0.42, 0.3, 0.3, 0.3, 30);
   cylinder(4.9, 0.6, -5.5, 0.06, 1.2, 8, 0, 0, 0, 0, 0, 0);
   cylinder(7.5, 0.6, -5.5, 0.06, 1.2, 8, 0, 0, 0, 0, 0, 0);
   cylinder(4.9, 0.6, -3.5, 0.06, 1.2, 8, 0, 0, 0, 0, 0, 0);
   cylinder(7.5, 0.6, -3.5, 0.06, 1.2, 8, 0, 0, 0, 0, 0, 0);

   // === TIRE RACK (Right side, middle) ===
   SetMaterial(0.2, 0.2, 0.22, 0.4, 0.4, 0.42, 0.3, 0.3, 0.3, 30);
   // Rack frame
   cylinder(6.5, 2, 0, 0.05, 4, 8, 0, 0, 0, 0, 0, 0);
   cylinder(7.5, 2, 0, 0.05, 4, 8, 0, 0, 0, 0, 0, 0);
   cylinder(7, 0.8, 0, 0.05, 1, 8, 90, 0, 0, 0, 0, 0);
   cylinder(7, 1.6, 0, 0.05, 1, 8, 90, 0, 0, 0, 0, 0);
   cylinder(7, 2.4, 0, 0.05, 1, 8, 90, 0, 0, 0, 0, 0);
   cylinder(7, 3.2, 0, 0.05, 1, 8, 90, 0, 0, 0, 0, 0);
   // Tires on rack
   SetMaterial(0.01, 0.01, 0.01, 0.05, 0.05, 0.05, 0.1, 0.1, 0.1, 5);
   for (int i = 0; i < 4; i++)
   {
      cylinder(7, 0.8 + i * 0.8, 0, 0.4, 0.3, 20, 90, 0, 0, 0, 0, 0);
   }

   // === IMPROVED COMPUTER/TELEMETRY STATION (Left wall, middle) ===
   // Modern desk with proper proportions
   // === IMPROVED COMPUTER/TELEMETRY STATION (Left wall, middle) ===
   glPushMatrix();
   glTranslated(-6.5, 0, 2);
   glRotated(-90, 0, 1, 0);
   // Modern desk with proper proportions
   SetMaterial(0.15, 0.15, 0.17, 0.3, 0.3, 0.35, 0.2, 0.2, 0.2, 30);
   noTexCube(0, 1.0, 0, 1.3, 0.1, 1.2, 0);

   // Desk legs (4 corners)
   SetMaterial(0.2, 0.2, 0.22, 0.4, 0.4, 0.42, 0.3, 0.3, 0.3, 30);
   cylinder(-1.1, 0.5, -0.5, 0.05, 1.0, 8, 0, 0, 0, 0, 0, 0);
   cylinder(-1.1, 0.5, 0.5, 0.05, 1.0, 8, 0, 0, 0, 0, 0, 0);
   cylinder(1.1, 0.5, -0.5, 0.05, 1.0, 8, 0, 0, 0, 0, 0, 0);
   cylinder(1.1, 0.5, 0.5, 0.05, 1.0, 8, 0, 0, 0, 0, 0, 0);

   // Modern widescreen monitor (improved proportions)
   SetMaterial(0.05, 0.05, 0.05, 0.1, 0.1, 0.1, 0.3, 0.3, 0.3, 40);
   noTexCube(0, 1.7, 0, 0.7, 0.5, 0.05, 0);

   // Monitor bezel (thin modern bezel)
   SetMaterial(0.02, 0.02, 0.02, 0.05, 0.05, 0.05, 0.2, 0.2, 0.2, 50);
   noTexCube(0, 1.7, 0.04, 0.72, 0.52, 0.02, 0);

   // Screen (blue glow - proper widescreen aspect)
   SetMaterial(0.1, 0.3, 0.6, 0.2, 0.5, 1.0, 0.5, 0.7, 1.0, 60);
   noTexCube(0, 1.7, 0.05, 0.65, 0.42, 0.01, 0);

   // Monitor stand (modern slim design)
   SetMaterial(0.3, 0.3, 0.32, 0.5, 0.5, 0.52, 0.4, 0.4, 0.4, 50);
   noTexCube(0, 1.12, 0, 0.15, 0.08, 0.15, 0);
   cylinder(0, 1.2, 0, 0.03, 0.35, 8, 0, 0, 0, 0, 0, 0);

   // Keyboard (modern mechanical keyboard)
   SetMaterial(0.1, 0.1, 0.12, 0.2, 0.2, 0.22, 0.1, 0.1, 0.1, 20);
   noTexCube(0, 1.12, 0.5, 0.45, 0.03, 0.2, 0);

   // Mouse
   SetMaterial(0.15, 0.15, 0.17, 0.25, 0.25, 0.27, 0.3, 0.3, 0.3, 40);
   noTexCube(0.9, 1.12, 0.3, 0.08, 0.025, 0.12, 0);

   glPopMatrix();

   // === LARGE TEAM LOGO/POSTER (Back wall, centered - MUCH BIGGER) ===
   SetMaterial(0.9, 0.1, 0.1, 1.0, 0.2, 0.2, 0.3, 0.3, 0.3, 40);
   noTexCube(0, 4.0, -5.95, 3.5, 1.8, 0.03, 0);

   // White border around poster
   SetMaterial(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 60);
   noTexCube(0, 4.0, -5.93, 3.6, 1.9, 0.02, 0);

   // Team logo letters/emblem (larger and more prominent)
   SetMaterial(1.0, 0.0, 0.0, 0.4, 0.0, 0.0, 0.8, 0.8, 0.8, 80);
   // Large letters
   noTexCube(-1.2, 4.0, -5.90, 0.35, 1.2, 0.02, 0);
   noTexCube(1.2, 4.0, -5.90, 0.35, 1.2, 0.02, 0);
   noTexCube(0, 4.0, -5.90, 0.5, 0.8, 0.02, 0);

   // === PIT BOARD (Against left wall) ===
   SetMaterial(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.2, 0.2, 0.2, 10);
   glPushMatrix();
   glTranslated(-7.7, 1.2, -1);
   glRotated(15, 0, 0, 1);
   noTexCube(0, 0, 0, 0.04, 0.9, 0.6, 0);
   glPopMatrix();
   // Pit board handle
   SetMaterial(0.3, 0.3, 0.3, 0.5, 0.5, 0.5, 0.2, 0.2, 0.2, 20);
   glPushMatrix();
   glTranslated(-7.7, 0.4, -1);
   cylinder(0, 0, 0, 0.025, 0.8, 8, 0, 0, 0, 0, 0, 0);
   glPopMatrix();

   // === TOOL BOARD/PEGBOARD (Right wall) ===
   SetMaterial(0.6, 0.5, 0.4, 0.8, 0.7, 0.6, 0.1, 0.1, 0.1, 10);
   noTexCube(7.95, 3, 3, 0.04, 1.8, 1.5, 0);
   // Tools hanging on board
   SetMaterial(0.5, 0.5, 0.55, 0.8, 0.8, 0.85, 1.0, 1.0, 1.0, 100);
   for (int i = 0; i < 5; i++)
   {
      double tz = 2.0 + i * 0.5;
      cylinder(7.9, 3.5, tz, 0.025, 0.5, 6, 90, 0, 0, 0, 0, 0);
      cylinder(7.9, 2.5, tz, 0.025, 0.5, 6, 90, 0, 0, 0, 0, 0);
   }

   // === CABLE REELS (Right side, front) ===
   SetMaterial(1.0, 0.6, 0.0, 1.0, 0.7, 0.1, 0.3, 0.3, 0.3, 30);
   cylinder(6.5, 0.2, 4, 0.3, 0.12, 16, 90, 0, 0, 0, 0, 0);
   SetMaterial(0.1, 0.1, 0.1, 0.2, 0.2, 0.2, 0.1, 0.1, 0.1, 10);
   cylinder(6.5, 0.2, 4, 0.06, 0.14, 12, 90, 0, 0, 0, 0, 0);

   // === AIR COMPRESSOR (Back right corner) ===
   SetMaterial(0.9, 0.5, 0.1, 1.0, 0.6, 0.2, 0.4, 0.4, 0.4, 40);
   noTexCube(5, 0.5, -1, 0.5, 0.5, 0.6, 0);
   // Compressor tank
   cylinder(5, 1, -1, 0.3, 0.8, 16, 0, 0, 0, 0, 0, 0);
   // Pressure gauge
   SetMaterial(0.8, 0.8, 0.8, 1.0, 1.0, 1.0, 0.6, 0.6, 0.6, 70);
   cylinder(5, 0.6, -1, 0.08, 0.03, 12, 90, 0, 0, 0, 0, 0);

   // === JACK STANDS (near car) ===
   SetMaterial(0.9, 0.7, 0.1, 1.0, 0.8, 0.2, 0.5, 0.5, 0.5, 60);
   // Left jack
   glPushMatrix();
   glTranslated(-2.5, 0, 1.8);
   noTexCube(0, 0.2, 0, 0.2, 0.2, 0.2, 0);
   cylinder(0, 0.35, 0, 0.05, 0.5, 8, 0, 0, 0, 0, 0, 0);
   noTexCube(0, 0.6, 0, 0.15, 0.04, 0.15, 0);
   glPopMatrix();
   // Right jack
   glPushMatrix();
   glTranslated(2.5, 0, 1.5);
   noTexCube(0, 0.2, 0, 0.2, 0.2, 0.2, 0);
   cylinder(0, 0.35, 0, 0.05, 0.5, 8, 0, 0, 0, 0, 0, 0);
   noTexCube(0, 0.6, 0, 0.15, 0.04, 0.15, 0);
   glPopMatrix();

   // === FLOOR MARKINGS (safety lines) ===
   SetMaterial(1.0, 1.0, 0.1, 1.0, 1.0, 0.2, 0.3, 0.3, 0.3, 30);
   glBegin(GL_QUADS);
   glNormal3f(0, 1, 0);
   // Yellow line left
   glVertex3f(3, 0.01, -5);
   glVertex3f(3.3, 0.01, -5);
   glVertex3f(3.3, 0.01, 5);
   glVertex3f(3, 0.01, 5);
   // Yellow line right
   glVertex3f(-3, 0.01, -5);
   glVertex3f(-3.3, 0.01, -5);
   glVertex3f(-3.3, 0.01, 5);
   glVertex3f(-3, 0.01, 5);
   glEnd();

   // === THE F1 CAR (centered, facing front/opening) ===
   glPushMatrix();
   glTranslated(0, 0.6, 0);
   glRotated(-90, 0, 1, 0); // Rotate to face front opening
   glDisable(GL_COLOR_MATERIAL);
   drawF1Car(1, 1, 1, ferrariColors);
   glEnable(GL_COLOR_MATERIAL);
   glPopMatrix();

   glPopMatrix();
}

void drawUTurnTrack()
{
   double trackWidth = 1.0;
   double straightLength = 4.0;
   double turnRadius = 2.0;

   // Track surface - matte asphalt
   SetMaterial(0.15, 0.15, 0.15, // ambient
               0.25, 0.25, 0.25, // diffuse
               0.05, 0.05, 0.05, // Very subtle specular (matte)
               5);               // Very low shininess
   glColor3f(1.0, 1.0, 1.0);

   // Enable texturing
   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glBindTexture(GL_TEXTURE_2D, texture[0]); // Asphalt texture

   // Draw bottom straight section
   glBegin(GL_QUADS);
   glNormal3f(0, 1, 0);
   glTexCoord2f(0, 0);
   glVertex3f(-straightLength / 2, 0.0, -turnRadius - trackWidth / 2);
   glTexCoord2f(25, 0);
   glVertex3f(straightLength / 2, 0.0, -turnRadius - trackWidth / 2);
   glTexCoord2f(25, 25);
   glVertex3f(straightLength / 2, 0.0, -turnRadius + trackWidth / 2);
   glTexCoord2f(0, 25);
   glVertex3f(-straightLength / 2, 0.0, -turnRadius + trackWidth / 2);
   glEnd();

   // Draw top straight section
   glBegin(GL_QUADS);
   glNormal3f(0, 1, 0);
   glTexCoord2f(0, 0);
   glVertex3f(-straightLength / 2, 0.0, turnRadius - trackWidth / 2);
   glTexCoord2f(25, 0);
   glVertex3f(straightLength / 2, 0.0, turnRadius - trackWidth / 2);
   glTexCoord2f(25, 25);
   glVertex3f(straightLength / 2, 0.0, turnRadius + trackWidth / 2);
   glTexCoord2f(0, 25);
   glVertex3f(-straightLength / 2, 0.0, turnRadius + trackWidth / 2);
   glEnd();

   // Draw right U-turn
   int segments = 32;
   glBegin(GL_QUAD_STRIP);
   for (int i = 0; i <= segments; i++)
   {
      double angle = -M_PI / 2 + M_PI * i / segments; // -PI/2 to PI/2
      double t = (double)i / segments;                // Texture parameter [0,1]

      // Outer edge of turn
      double x_outer = straightLength / 2 + (turnRadius + trackWidth / 2) * cos(angle);
      double z_outer = (turnRadius + trackWidth / 2) * sin(angle);

      // Inner edge of turn
      double x_inner = straightLength / 2 + (turnRadius - trackWidth / 2) * cos(angle);
      double z_inner = (turnRadius - trackWidth / 2) * sin(angle);

      glNormal3f(0, 1, 0);
      glTexCoord2f(t * 25, 25);
      glVertex3f(x_outer, 0.0, z_outer);
      glTexCoord2f(t * 25, 0);
      glVertex3f(x_inner, 0.0, z_inner);
   }
   glEnd();

   // Draw left U-turn
   glBegin(GL_QUAD_STRIP);
   for (int i = 0; i <= segments; i++)
   {
      double angle = M_PI / 2 + M_PI * i / segments; // PI/2 to 3*PI/2
      double t = (double)i / segments;

      // Outer edge of turn
      double x_outer = -straightLength / 2 + (turnRadius + trackWidth / 2) * cos(angle);
      double z_outer = (turnRadius + trackWidth / 2) * sin(angle);

      // Inner edge of turn
      double x_inner = -straightLength / 2 + (turnRadius - trackWidth / 2) * cos(angle);
      double z_inner = (turnRadius - trackWidth / 2) * sin(angle);

      glNormal3f(0, 1, 0);
      glTexCoord2f(t * 25, 25);
      glVertex3f(x_outer, 0.0, z_outer);
      glTexCoord2f(t * 25, 0);
      glVertex3f(x_inner, 0.0, z_inner);
   }
   glEnd();

   glDisable(GL_TEXTURE_2D);
}
// Draw curbs
void drawCurbs()
{
   double trackWidth = 1.0;
   double straightLength = 4.0;
   double turnRadius = 2.0;
   double curbHeight = 0.01; // Slightly raised for visibility
   double curbWidth = 0.2;

   // Red and white curb pattern
   SetMaterial(0.24, 0.06, 0.06, // Red ambient
               0.9, 0.3, 0.3,    // Red diffuse
               0.2, 0.2, 0.2,    // Low specular
               8);
   glColor3f(0.9, 0.3, 0.3);

   // Enable curb texture
   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glBindTexture(GL_TEXTURE_2D, texture[3]); // Curb texture

   // Bottom straight outer curb
   glBegin(GL_QUADS);
   glNormal3f(0, 1, 0);
   glTexCoord2f(0, 0);
   glVertex3f(-straightLength / 2, curbHeight, -turnRadius - trackWidth / 2 - curbWidth);
   glTexCoord2f(straightLength * 5, 0);
   glVertex3f(straightLength / 2, curbHeight, -turnRadius - trackWidth / 2 - curbWidth);
   glTexCoord2f(straightLength * 5, 1);
   glVertex3f(straightLength / 2, curbHeight, -turnRadius - trackWidth / 2);
   glTexCoord2f(0, 1);
   glVertex3f(-straightLength / 2, curbHeight, -turnRadius - trackWidth / 2);
   glEnd();

   // Bottom straight inner curb
   glBegin(GL_QUADS);
   glNormal3f(0, 1, 0);
   glTexCoord2f(0, 0);
   glVertex3f(-straightLength / 2, curbHeight, -turnRadius + trackWidth / 2);
   glTexCoord2f(straightLength * 5, 0);
   glVertex3f(straightLength / 2, curbHeight, -turnRadius + trackWidth / 2);
   glTexCoord2f(straightLength * 5, 1);
   glVertex3f(straightLength / 2, curbHeight, -turnRadius + trackWidth / 2 + curbWidth);
   glTexCoord2f(0, 1);
   glVertex3f(-straightLength / 2, curbHeight, -turnRadius + trackWidth / 2 + curbWidth);
   glEnd();

   // Top straight outer curb
   glBegin(GL_QUADS);
   glNormal3f(0, 1, 0);
   glTexCoord2f(0, 0);
   glVertex3f(-straightLength / 2, curbHeight, turnRadius + trackWidth / 2);
   glTexCoord2f(straightLength * 5, 0);
   glVertex3f(straightLength / 2, curbHeight, turnRadius + trackWidth / 2);
   glTexCoord2f(straightLength * 5, 1);
   glVertex3f(straightLength / 2, curbHeight, turnRadius + trackWidth / 2 + curbWidth);
   glTexCoord2f(0, 1);
   glVertex3f(-straightLength / 2, curbHeight, turnRadius + trackWidth / 2 + curbWidth);
   glEnd();

   // Top straight inner curb
   glBegin(GL_QUADS);
   glNormal3f(0, 1, 0);
   glTexCoord2f(0, 0);
   glVertex3f(-straightLength / 2, curbHeight, turnRadius - trackWidth / 2 - curbWidth);
   glTexCoord2f(straightLength * 5, 0);
   glVertex3f(straightLength / 2, curbHeight, turnRadius - trackWidth / 2 - curbWidth);
   glTexCoord2f(straightLength * 5, 1);
   glVertex3f(straightLength / 2, curbHeight, turnRadius - trackWidth / 2);
   glTexCoord2f(0, 1);
   glVertex3f(-straightLength / 2, curbHeight, turnRadius - trackWidth / 2);
   glEnd();

   // For curved sections, use parametric mapping with consistent scale
   int segments = 32;
   double textureScale = 5.0; // Adjust this to match straight section density

   // Right U-turn outer curb
   double outerRadius = turnRadius + trackWidth / 2;
   double outerArcLength = M_PI * outerRadius;

   glBegin(GL_QUAD_STRIP);
   for (int i = 0; i <= segments; i++)
   {
      double angle = -M_PI / 2 + M_PI * i / segments;
      double t = (double)i / segments;
      double texU = t * outerArcLength * textureScale;

      double x1 = straightLength / 2 + (outerRadius + curbWidth) * cos(angle);
      double z1 = (outerRadius + curbWidth) * sin(angle);
      double x2 = straightLength / 2 + outerRadius * cos(angle);
      double z2 = outerRadius * sin(angle);

      glNormal3f(0, 1, 0);
      glTexCoord2f(texU, 1);
      glVertex3f(x1, curbHeight, z1);
      glTexCoord2f(texU, 0);
      glVertex3f(x2, curbHeight, z2);
   }
   glEnd();

   // Right U-turn inner curb
   double innerRadius = turnRadius - trackWidth / 2;
   double innerArcLength = M_PI * innerRadius;

   glBegin(GL_QUAD_STRIP);
   for (int i = 0; i <= segments; i++)
   {
      double angle = -M_PI / 2 + M_PI * i / segments;
      double t = (double)i / segments;
      double texU = t * innerArcLength * textureScale;

      double x1 = straightLength / 2 + innerRadius * cos(angle);
      double z1 = innerRadius * sin(angle);
      double x2 = straightLength / 2 + (innerRadius - curbWidth) * cos(angle);
      double z2 = (innerRadius - curbWidth) * sin(angle);

      glNormal3f(0, 1, 0);
      glTexCoord2f(texU, 0);
      glVertex3f(x1, curbHeight, z1);
      glTexCoord2f(texU, 1);
      glVertex3f(x2, curbHeight, z2);
   }
   glEnd();

   // Left U-turn outer curb
   glBegin(GL_QUAD_STRIP);
   for (int i = 0; i <= segments; i++)
   {
      double angle = M_PI / 2 + M_PI * i / segments;
      double t = (double)i / segments;
      double texU = t * outerArcLength * textureScale;

      double x1 = -straightLength / 2 + (outerRadius + curbWidth) * cos(angle);
      double z1 = (outerRadius + curbWidth) * sin(angle);
      double x2 = -straightLength / 2 + outerRadius * cos(angle);
      double z2 = outerRadius * sin(angle);

      glNormal3f(0, 1, 0);
      glTexCoord2f(texU, 1);
      glVertex3f(x1, curbHeight, z1);
      glTexCoord2f(texU, 0);
      glVertex3f(x2, curbHeight, z2);
   }
   glEnd();

   // Left U-turn inner curb
   glBegin(GL_QUAD_STRIP);
   for (int i = 0; i <= segments; i++)
   {
      double angle = M_PI / 2 + M_PI * i / segments;
      double t = (double)i / segments;
      double texU = t * innerArcLength * textureScale;

      double x1 = -straightLength / 2 + innerRadius * cos(angle);
      double z1 = innerRadius * sin(angle);
      double x2 = -straightLength / 2 + (innerRadius - curbWidth) * cos(angle);
      double z2 = (innerRadius - curbWidth) * sin(angle);

      glNormal3f(0, 1, 0);
      glTexCoord2f(texU, 0);
      glVertex3f(x1, curbHeight, z1);
      glTexCoord2f(texU, 1);
      glVertex3f(x2, curbHeight, z2);
   }
   glEnd();

   glDisable(GL_TEXTURE_2D);
}

// Function to draw start/finish line
void drawStartFinishLine()
{
   double trackWidth = 1.0;
   double turnRadius = 2.0;
   int squares = 8;
   double squareWidth = trackWidth / squares;

   double line_z = -turnRadius; // Center of the bottom straight
   double line_y = 0.01;        // Slightly above the track

   for (int i = 0; i < squares; i++)
   {
      if (i % 2 == 0)
      {
         SetMaterial(0.2, 0.2, 0.2, 1.0, 1.0, 1.0, 0.2, 0.2, 0.2, 10);
         glColor3f(1.0, 1.0, 1.0); // White
      }
      else
      {
         SetMaterial(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.05, 0.05, 0.05, 5);
         glColor3f(0.0, 0.0, 0.0); // Black
      }

      glBegin(GL_QUADS);
      glNormal3f(0, 1, 0);
      glVertex3f(-trackWidth / 2 + i * squareWidth, line_y, line_z - 0.02);
      glVertex3f(-trackWidth / 2 + (i + 1) * squareWidth, line_y, line_z - 0.02);
      glVertex3f(-trackWidth / 2 + (i + 1) * squareWidth, line_y, line_z + 0.02);
      glVertex3f(-trackWidth / 2 + i * squareWidth, line_y, line_z + 0.02);
      glEnd();
   }
}

// Function to draw a building with windows
void drawStand(double x, double y, double z, double width, double height, double depth)
{
   // Main building block
   SetMaterial(0.14, 0.14, 0.14, // ambient
               0.6, 0.6, 0.6,    // diffuse
               0.2, 0.2, 0.2,    // low specular
               10);              // low shininess
   glColor3f(1, 1, 1);

   // Enable texture
   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glBindTexture(GL_TEXTURE_2D, texture[1]); // Concrete texture

   // Draw textured cube
   glPushMatrix();
   glTranslated(x, y + height / 2, z);
   glScaled(width / 2, height / 2, depth / 2);

   glBegin(GL_QUADS);
   // Front
   glNormal3f(0, 0, 1);
   glTexCoord2f(0, 0);
   glVertex3f(-1, -1, 1);
   glTexCoord2f(100, 0);
   glVertex3f(+1, -1, 1);
   glTexCoord2f(100, 100);
   glVertex3f(+1, +1, 1);
   glTexCoord2f(0, 100);
   glVertex3f(-1, +1, 1);

   // Back
   glNormal3f(0, 0, -1);
   glTexCoord2f(0, 0);
   glVertex3f(+1, -1, -1);
   glTexCoord2f(100, 0);
   glVertex3f(-1, -1, -1);
   glTexCoord2f(100, 100);
   glVertex3f(-1, +1, -1);
   glTexCoord2f(0, 100);
   glVertex3f(+1, +1, -1);

   // Right
   glNormal3f(+1, 0, 0);
   glTexCoord2f(0, 0);
   glVertex3f(+1, -1, +1);
   glTexCoord2f(100, 0);
   glVertex3f(+1, -1, -1);
   glTexCoord2f(100, 100);
   glVertex3f(+1, +1, -1);
   glTexCoord2f(0, 100);
   glVertex3f(+1, +1, +1);

   // Left
   glNormal3f(-1, 0, 0);
   glTexCoord2f(0, 0);
   glVertex3f(-1, -1, -1);
   glTexCoord2f(100, 0);
   glVertex3f(-1, -1, +1);
   glTexCoord2f(100, 100);
   glVertex3f(-1, +1, +1);
   glTexCoord2f(0, 100);
   glVertex3f(-1, +1, -1);

   // Top
   glNormal3f(0, +1, 0);
   glTexCoord2f(0, 0);
   glVertex3f(-1, +1, +1);
   glTexCoord2f(100, 0);
   glVertex3f(+1, +1, +1);
   glTexCoord2f(100, 100);
   glVertex3f(+1, +1, -1);
   glTexCoord2f(0, 100);
   glVertex3f(-1, +1, -1);

   // Bottom
   glNormal3f(0, -1, 0);
   glTexCoord2f(0, 0);
   glVertex3f(-1, -1, -1);
   glTexCoord2f(100, 0);
   glVertex3f(+1, -1, -1);
   glTexCoord2f(100, 100);
   glVertex3f(+1, -1, +1);
   glTexCoord2f(0, 100);
   glVertex3f(-1, -1, +1);
   glEnd();

   glPopMatrix();

   glDisable(GL_TEXTURE_2D);

   int rows = 5;
   int cols = 6;
   double winW = (width / cols) * 0.6;
   double winH = (height / rows) * 0.6;
   double inset = 0.01;
   SetMaterial(0.1, 0.2, 0.4, //  ambient
               0.2, 0.4, 0.8, // diffuse
               0.5, 0.5, 0.5, // Medium specular
               80);           // Low shininess
   glColor3f(0.2, 0.4, 0.8);

   // Front & back faces
   for (int i = 1; i <= cols; i++)
   {
      for (int j = 1; j <= rows; j++)
      {
         double wx = x - width / 2 + (i - 0.5) * (width / cols);
         double wy = y + (j - 0.5) * (height / rows);

         // Front
         glBegin(GL_QUADS);
         glNormal3f(0, 0, 1);
         glVertex3f(wx - winW / 2, wy - winH / 2, z + depth / 2 + inset);
         glVertex3f(wx + winW / 2, wy - winH / 2, z + depth / 2 + inset);
         glVertex3f(wx + winW / 2, wy + winH / 2, z + depth / 2 + inset);
         glVertex3f(wx - winW / 2, wy + winH / 2, z + depth / 2 + inset);
         glEnd();

         // Back
         glBegin(GL_QUADS);
         glNormal3f(0, 0, -1);
         glVertex3f(wx - winW / 2, wy - winH / 2, z - depth / 2 - inset);
         glVertex3f(wx + winW / 2, wy - winH / 2, z - depth / 2 - inset);
         glVertex3f(wx + winW / 2, wy + winH / 2, z - depth / 2 - inset);
         glVertex3f(wx - winW / 2, wy + winH / 2, z - depth / 2 - inset);
         glEnd();
      }
   }
}

// Draw trees
void drawTree(double x, double y, double z, double scale)
{

   // Enable texture
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, texture[4]);
   SetMaterial(0.08, 0.04, 0.02, // Brown ambient
               0.4, 0.2, 0.1,    // Brown diffuse
               0.05, 0.05, 0.05, // Very low specular
               2);               // Very low shininess
   glColor3f(0.4, 0.2, 0.1);     // Brown

   // Draw a textured pillar/column
   cylinder(x, y + 0.3 * scale, z, 0.1 * scale, 0.6 * scale, 8, 0, 0, 0, 1, 1, 1);

   glDisable(GL_TEXTURE_2D);

   // Enable texture
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, texture[5]);
   // Leaves
   SetMaterial(0.1, 0.2, 0.1, // Light green ambient
               0.4, 0.8, 0.4, // Light green diffuse
               0.1, 0.2, 0.1, // Specular
               10);

   glColor3f(0.4, 0.8, 0.4); // Light green color

   glPushMatrix();
   glTranslated(x, y + 0.7 * scale, z);
   glScaled(0.4 * scale, 0.4 * scale, 0.4 * scale);
   // Leaf approximation using multiple cubes
   cube(0, 0, 0, 1, 1, 1, 0, 1, 1, 1);
   cube(0, 0.3, 0, 0.8, 0.8, 0.8, 0, 1, 1, 1);
   cube(0, -0.3, 0, 0.8, 0.8, 0.8, 0, 1, 1, 1);
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

// Draw a road section
void drawRoad(double x, double y, double z, double width, double length, double rotation)
{
   SetMaterial(0.15, 0.15, 0.15, 0.25, 0.25, 0.25, 0.05, 0.05, 0.05, 5);

   glPushMatrix();
   glTranslated(x, y, z);
   glRotated(rotation, 0, 1, 0);

   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, texture[0]); // Asphalt texture

   glBegin(GL_QUADS);
   glNormal3f(0, 1, 0);
   glTexCoord2f(0, 0);
   glVertex3f(-width / 2, 0, -length / 2);
   glTexCoord2f(width * 2, 0);
   glVertex3f(width / 2, 0, -length / 2);
   glTexCoord2f(width * 2, length * 2);
   glVertex3f(width / 2, 0, length / 2);
   glTexCoord2f(0, length * 2);
   glVertex3f(-width / 2, 0, length / 2);
   glEnd();

   glDisable(GL_TEXTURE_2D);
   glPopMatrix();
}

// Draw pit lane with markings
void drawPitLane(double x, double y, double z, double width, double length)
{
   // Main pit lane surface
   SetMaterial(0.15, 0.15, 0.15, 0.25, 0.25, 0.25, 0.05, 0.05, 0.05, 5);

   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, texture[0]);

   glBegin(GL_QUADS);
   glNormal3f(0, 1, 0);
   glTexCoord2f(0, 0);
   glVertex3f(x - width / 2, y, z - length / 2);
   glTexCoord2f(width * 2, 0);
   glVertex3f(x + width / 2, y, z - length / 2);
   glTexCoord2f(width * 2, length * 2);
   glVertex3f(x + width / 2, y, z + length / 2);
   glTexCoord2f(0, length * 2);
   glVertex3f(x - width / 2, y, z + length / 2);
   glEnd();

   glDisable(GL_TEXTURE_2D);

   // Pit lane white lines
   SetMaterial(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.3, 0.3, 0.3, 20);
   glColor3f(1.0, 1.0, 1.0);

   // Left line
   glBegin(GL_QUADS);
   glNormal3f(0, 1, 0);
   glVertex3f(x - width / 2, y + 0.01, z - length / 2);
   glVertex3f(x - width / 2 + 0.1, y + 0.01, z - length / 2);
   glVertex3f(x - width / 2 + 0.1, y + 0.01, z + length / 2);
   glVertex3f(x - width / 2, y + 0.01, z + length / 2);
   glEnd();

   // Right line
   glBegin(GL_QUADS);
   glNormal3f(0, 1, 0);
   glVertex3f(x + width / 2 - 0.1, y + 0.01, z - length / 2);
   glVertex3f(x + width / 2, y + 0.01, z - length / 2);
   glVertex3f(x + width / 2, y + 0.01, z + length / 2);
   glVertex3f(x + width / 2 - 0.1, y + 0.01, z + length / 2);
   glEnd();

   // Speed limit markings (60 km/h)
   for (int i = 0; i < 3; i++)
   {
      double markZ = z - length / 2 + length / 4 + i * length / 4;
      glBegin(GL_QUADS);
      glNormal3f(0, 1, 0);
      glVertex3f(x - 0.3, y + 0.01, markZ - 0.3);
      glVertex3f(x + 0.3, y + 0.01, markZ - 0.3);
      glVertex3f(x + 0.3, y + 0.01, markZ + 0.3);
      glVertex3f(x - 0.3, y + 0.01, markZ + 0.3);
      glEnd();
   }
}

// Draw tire barrier (stack of colored tires)
void drawTireBarrier(double x, double y, double z, int numTires, float r, float g, float b)
{
   SetMaterial(r * 0.3, g * 0.3, b * 0.3, r, g, b, 0.1, 0.1, 0.1, 10);
   glColor3f(r, g, b);

   glPushMatrix();
   glTranslated(x, y, z);

   // Stack tires
   for (int i = 0; i < numTires; i++)
   {
      glPushMatrix();
      glTranslated(0, 0.15 + i * 0.28, 0);
      glRotated(90, 1, 0, 0);
      // Outer tire
      cylinder(0, 0, 0, 0.3, 0.25, 16, 90, 0, 0, 0, 0, 0);

      // Inner hole (darker)
      SetMaterial(0.05, 0.05, 0.05, 0.1, 0.1, 0.1, 0.05, 0.05, 0.05, 5);
      glColor3f(0.1, 0.1, 0.1);
      cylinder(0, 0, 0, 0.15, 0.26, 12, 90, 0, 0, 0, 0, 0);

      // Reset color for next tire
      SetMaterial(r * 0.3, g * 0.3, b * 0.3, r, g, b, 0.1, 0.1, 0.1, 10);
      glColor3f(r, g, b);

      glPopMatrix();
   }

   glPopMatrix();
}

// Draw a row of tire barriers with alternating colors
void drawTireBarrierRow(double startX, double y, double z, int count, double spacing)
{
   float colors[3][3] = {
       {0.9, 0.1, 0.1}, // Red
       {1.0, 1.0, 0.2}, // Yellow
       {0.2, 0.4, 0.9}  // Blue
   };

   for (int i = 0; i < count; i++)
   {
      float *color = colors[i % 3];
      drawTireBarrier(startX + i * spacing, y, z, 4, color[0], color[1], color[2]);
   }
}

// Complete pit complex scene
void drawPitComplex()
{
   // === GROUND PLANE - Large base ground (LOWEST LAYER) ===
   SetMaterial(0.12, 0.12, 0.12, 0.25, 0.25, 0.25, 0.05, 0.05, 0.05, 5);
   glColor3f(0.25, 0.25, 0.25);

   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, texture[1]); // Concrete texture

   glBegin(GL_QUADS);
   glNormal3f(0, 1, 0);
   glTexCoord2f(0, 0);
   glVertex3f(-50, 0, -10);
   glTexCoord2f(50, 0);
   glVertex3f(50, 0, -10);
   glTexCoord2f(50, 100);
   glVertex3f(50, 0, 90);
   glTexCoord2f(0, 100);
   glVertex3f(-50, 0, 90);
   glEnd();

   glDisable(GL_TEXTURE_2D);

   // === GRASS AREAS (LAYER 1 - y = 0.01) ===
   SetMaterial(0.1, 0.3, 0.1, 0.2, 0.5, 0.2, 0.05, 0.1, 0.05, 5);
   glColor3f(0.2, 0.5, 0.2);

   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, texture[2]); // Grass texture

   // Right grass strip (behind barriers)
   glBegin(GL_QUADS);
   glNormal3f(0, 1, 0);
   glTexCoord2f(0, 0);
   glVertex3f(5, 0.01, -5);
   glTexCoord2f(5, 0);
   glVertex3f(20, 0.01, -5);
   glTexCoord2f(5, 85);
   glVertex3f(20, 0.01, 80);
   glTexCoord2f(0, 85);
   glVertex3f(5, 0.01, 80);
   glEnd();

   glDisable(GL_TEXTURE_2D);

   // === MAIN ROAD (LAYER 2 - y = 0.02) ===
   drawRoad(0, 0.02, 40, 4, 80, 0);

   // === PIT LANE (LAYER 2 - y = 0.02) ===
   drawPitLane(-6, 0.02, 40, 4, 75);

   // === ROAD MARKINGS (LAYER 3 - y = 0.04) ===
   SetMaterial(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.3, 0.3, 0.3, 20);
   glColor3f(1.0, 1.0, 1.0);

   // Road side white stripes (left side)
   glBegin(GL_QUADS);
   glNormal3f(0, 1, 0);
   glVertex3f(-2.1, 0.04, 0);
   glVertex3f(-1.9, 0.04, 0);
   glVertex3f(-1.9, 0.04, 80);
   glVertex3f(-2.1, 0.04, 80);
   glEnd();

   // Road side white stripes (right side)
   glBegin(GL_QUADS);
   glNormal3f(0, 1, 0);
   glVertex3f(1.9, 0.04, 0);
   glVertex3f(2.1, 0.04, 0);
   glVertex3f(2.1, 0.04, 80);
   glVertex3f(1.9, 0.04, 80);
   glEnd();

   // === PIT LANE ENTRY/EXIT LINES ===
   // Entry line
   glBegin(GL_QUADS);
   glNormal3f(0, 1, 0);
   glVertex3f(-8, 0.04, 3);
   glVertex3f(-4, 0.04, 3);
   glVertex3f(-4, 0.04, 3.3);
   glVertex3f(-8, 0.04, 3.3);
   glEnd();

   // Exit line
   glBegin(GL_QUADS);
   glNormal3f(0, 1, 0);
   glVertex3f(-8, 0.04, 73);
   glVertex3f(-4, 0.04, 73);
   glVertex3f(-4, 0.04, 73.3);
   glVertex3f(-8, 0.04, 73.3);
   glEnd();

   // Pit lane center dashed line
   for (int i = 0; i < 15; i++)
   {
      double lineZ = 5 + i * 5;
      glBegin(GL_QUADS);
      glNormal3f(0, 1, 0);
      glVertex3f(-6, 0.04, lineZ);
      glVertex3f(-5.9, 0.04, lineZ);
      glVertex3f(-5.9, 0.04, lineZ + 2);
      glVertex3f(-6, 0.04, lineZ + 2);
      glEnd();
   }

   // === RED/WHITE CURBS (between pit lane and track) ===
   SetMaterial(0.8, 0.1, 0.1, 1.0, 0.2, 0.2, 0.3, 0.3, 0.3, 40);
   for (int i = 0; i < 30; i++)
   {
      double curbZ = 2 + i * 2.5;

      // Alternate red and white
      if (i % 2 == 0)
      {
         SetMaterial(0.9, 0.1, 0.1, 1.0, 0.2, 0.2, 0.3, 0.3, 0.3, 40);
         glColor3f(1.0, 0.2, 0.2);
      }
      else
      {
         SetMaterial(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.4, 0.4, 0.4, 50);
         glColor3f(1.0, 1.0, 1.0);
      }

      noTexCube(-4, 0.08, curbZ, 0.15, 0.08, 1.2, 0);
   }

   // === FOUR GARAGES - PROPERLY SPACED ===
   float garagePositions[4] = {8, 23, 38, 53};

   for (int i = 0; i < 4; i++)
   {
      glPushMatrix();
      glTranslated(-14, 0, garagePositions[i]);
      glRotated(90, 0, 1, 0);
      glDisable(GL_COLOR_MATERIAL);
      drawF1Garage(0, 0, 0, 0.8);
      glEnable(GL_COLOR_MATERIAL);
      glPopMatrix();
   }

   // === IMPROVED TIRE BARRIERS (right side) ===
   for (int i = 0; i < 16; i++)
   {
      double zPos = 3 + i * 4.5;
      int colorIdx = i % 3;

      if (colorIdx == 0)
         drawTireBarrier(3.8, 0, zPos, 3, 0.9, 0.1, 0.1); // Red
      else if (colorIdx == 1)
         drawTireBarrier(3.8, 0, zPos, 3, 1.0, 1.0, 0.2); // Yellow
      else
         drawTireBarrier(3.8, 0, zPos, 3, 0.2, 0.4, 0.9); // Blue
   }

   // === SAFETY FENCE behind tire barriers ===
   SetMaterial(0.3, 0.3, 0.3, 0.5, 0.5, 0.5, 0.3, 0.3, 0.3, 40);
   glColor3f(0.4, 0.4, 0.4);

   for (int i = 0; i < 20; i++)
   {
      double fenceZ = 0 + i * 4;
      cylinder(6.5, 1.5, fenceZ, 0.05, 3.0, 8, 0, 0, 0, 0, 0, 0);
   }

   // Horizontal fence bars
   glLineWidth(2.0); // MOVE THIS BEFORE glBegin()
   glBegin(GL_LINES);
   for (int j = 0; j < 6; j++)
   {
      double barY = 0.3 + j * 0.5;
      glVertex3f(6.5, barY, 0);
      glVertex3f(6.5, barY, 80);
   }
   glEnd();
   glLineWidth(1.0); // Reset to default

   // === PIT WALL (low wall between pit boxes and track) ===
   SetMaterial(0.7, 0.7, 0.7, 0.9, 0.9, 0.9, 0.3, 0.3, 0.3, 40);
   glColor3f(0.8, 0.8, 0.8);

   // Main wall body
   glBegin(GL_QUADS);
   // Front face
   glNormal3f(1, 0, 0);
   glVertex3f(-3.8, 0.02, 2);
   glVertex3f(-3.8, 0.02, 75);
   glVertex3f(-3.8, 1.2, 75);
   glVertex3f(-3.8, 1.2, 2);
   // Top
   glNormal3f(0, 1, 0);
   glVertex3f(-3.8, 1.2, 2);
   glVertex3f(-3.8, 1.2, 75);
   glVertex3f(-4.0, 1.2, 75);
   glVertex3f(-4.0, 1.2, 2);
   // Back face
   glNormal3f(-1, 0, 0);
   glVertex3f(-4.0, 0.02, 75);
   glVertex3f(-4.0, 0.02, 2);
   glVertex3f(-4.0, 1.2, 2);
   glVertex3f(-4.0, 1.2, 75);
   glEnd();

   // === PIT CREW EQUIPMENT - Better positioned ===
   for (int i = 0; i < 4; i++)
   {
      double boxZ = garagePositions[i] + 1;

      // Tool carts
      SetMaterial(0.7, 0.1, 0.1, 0.9, 0.2, 0.2, 0.3, 0.3, 0.3, 40);
      glColor3f(0.9, 0.2, 0.2);
      noTexCube(-5.5, 0.35, boxZ, 0.5, 0.35, 0.4, 0);

      // Wheels
      SetMaterial(0.1, 0.1, 0.1, 0.2, 0.2, 0.2, 0.1, 0.1, 0.1, 10);
      glColor3f(0.15, 0.15, 0.15);
      cylinder(-5.8, 0.1, boxZ - 0.3, 0.08, 0.08, 8, 90, 0, 0, 0, 0, 0);
      cylinder(-5.8, 0.1, boxZ + 0.3, 0.08, 0.08, 8, 90, 0, 0, 0, 0, 0);
      cylinder(-5.2, 0.1, boxZ - 0.3, 0.08, 0.08, 8, 90, 0, 0, 0, 0, 0);
      cylinder(-5.2, 0.1, boxZ + 0.3, 0.08, 0.08, 8, 90, 0, 0, 0, 0, 0);

      // Tire racks
      SetMaterial(0.2, 0.3, 0.8, 0.3, 0.4, 1.0, 0.4, 0.4, 0.4, 50);
      glColor3f(0.3, 0.4, 1.0);
      noTexCube(-7.2, 0.4, boxZ + 2, 0.6, 0.4, 0.5, 0);

      // Spare tires
      SetMaterial(0.05, 0.05, 0.05, 0.1, 0.1, 0.1, 0.05, 0.05, 0.05, 5);
      glColor3f(0.1, 0.1, 0.1);
      cylinder(-7.2, 0.6, boxZ + 2, 0.3, 0.25, 16, 90, 0, 0, 0, 0, 0);
   }

   // === TEAM NUMBER BOARDS above each garage ===
   for (int i = 0; i < 4; i++)
   {
      double boardZ = garagePositions[i];

      // Board backing
      SetMaterial(0.2, 0.2, 0.2, 0.4, 0.4, 0.4, 0.2, 0.2, 0.2, 30);
      glColor3f(0.3, 0.3, 0.3);
      noTexCube(-13.8, 5.2, boardZ, 0.1, 1.2, 2.5, 0);

      // Team color stripe
      if (i == 0)
         SetMaterial(0.8, 0.1, 0.1, 1.0, 0.2, 0.2, 0.3, 0.3, 0.3, 40); // Red
      else if (i == 1)
         SetMaterial(0.1, 0.3, 0.8, 0.2, 0.4, 1.0, 0.4, 0.4, 0.4, 50); // Blue
      else if (i == 2)
         SetMaterial(0.9, 0.6, 0.1, 1.0, 0.7, 0.2, 0.3, 0.3, 0.3, 40); // Orange
      else
         SetMaterial(0.2, 0.7, 0.2, 0.3, 0.9, 0.3, 0.3, 0.3, 0.3, 40); // Green

      noTexCube(-13.7, 5.2, boardZ, 0.08, 0.3, 2.4, 0);
   }

   // === PIT LANE SIGNS ===
   SetMaterial(0.2, 0.2, 0.2, 0.4, 0.4, 0.4, 0.1, 0.1, 0.1, 10);
   glColor3f(0.3, 0.3, 0.3);

   // Pit lane ENTRY sign
   cylinder(-6.5, 0.75, 1, 0.06, 1.5, 8, 0, 0, 0, 0, 0, 0);
   SetMaterial(1.0, 1.0, 0.2, 1.0, 1.0, 0.3, 0.3, 0.3, 0.3, 30);
   glColor3f(1.0, 1.0, 0.3);
   noTexCube(-6.45, 2.0, 1, 0.08, 0.6, 0.8, 0);
   // "PIT IN" text indicator
   SetMaterial(0.0, 0.0, 0.0, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 10);
   glColor3f(0.0, 0.0, 0.0);
   noTexCube(-6.4, 2.0, 1, 0.05, 0.3, 0.6, 0);

   // Pit lane EXIT sign
   SetMaterial(0.2, 0.2, 0.2, 0.4, 0.4, 0.4, 0.1, 0.1, 0.1, 10);
   glColor3f(0.3, 0.3, 0.3);
   cylinder(-6.5, 0.75, 75, 0.06, 1.5, 8, 0, 0, 0, 0, 0, 0);
   SetMaterial(1.0, 1.0, 0.2, 1.0, 1.0, 0.3, 0.3, 0.3, 0.3, 30);
   glColor3f(1.0, 1.0, 0.3);
   noTexCube(-6.45, 2.0, 75, 0.08, 0.6, 0.8, 0);
   // "PIT OUT" text indicator
   SetMaterial(0.0, 0.0, 0.0, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 10);
   glColor3f(0.0, 0.0, 0.0);
   noTexCube(-6.4, 2.0, 75, 0.05, 0.3, 0.6, 0);

   // === SPEED LIMIT SIGNS (80 km/h typical for pit lane) ===
   for (int i = 1; i < 4; i++)
   {
      double signZ = garagePositions[i] - 2;

      SetMaterial(0.2, 0.2, 0.2, 0.4, 0.4, 0.4, 0.1, 0.1, 0.1, 10);
      glColor3f(0.3, 0.3, 0.3);
      cylinder(-3.7, 0.6, signZ, 0.04, 1.2, 8, 0, 0, 0, 0, 0, 0);

      SetMaterial(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.3, 0.3, 0.3, 30);
      glColor3f(1.0, 1.0, 1.0);
      cylinder(-3.7, 1.6, signZ, 0.25, 0.05, 16, 90, 0, 0, 0, 0, 0);

      // Red border
      SetMaterial(0.9, 0.1, 0.1, 1.0, 0.2, 0.2, 0.3, 0.3, 0.3, 40);
      glColor3f(1.0, 0.2, 0.2);
      cylinder(-3.68, 1.6, signZ, 0.28, 0.03, 16, 90, 0, 0, 0, 0, 0);
   }

   // === LIGHTING POLES ===
   SetMaterial(0.25, 0.25, 0.25, 0.45, 0.45, 0.45, 0.3, 0.3, 0.3, 40);
   glColor3f(0.4, 0.4, 0.4);

   for (int i = 0; i < 5; i++)
   {
      double poleZ = 5 + i * 18;

      // Left side poles
      cylinder(-18, 3.5, poleZ, 0.12, 7.0, 12, 0, 0, 0, 0, 0, 0);
      // Light fixtures
      SetMaterial(0.9, 0.9, 0.8, 1.0, 1.0, 0.9, 0.8, 0.8, 0.8, 80);
      glColor3f(1.0, 1.0, 0.9);
      noTexCube(-18, 6.8, poleZ, 0.4, 0.2, 0.3, 0);

      // Right side poles
      SetMaterial(0.25, 0.25, 0.25, 0.45, 0.45, 0.45, 0.3, 0.3, 0.3, 40);
      glColor3f(0.4, 0.4, 0.4);
      cylinder(10, 3.5, poleZ, 0.12, 7.0, 12, 0, 0, 0, 0, 0, 0);
      // Light fixtures
      SetMaterial(0.9, 0.9, 0.8, 1.0, 1.0, 0.9, 0.8, 0.8, 0.8, 80);
      glColor3f(1.0, 1.0, 0.9);
      noTexCube(10, 6.8, poleZ, 0.4, 0.2, 0.3, 0);
   }
}

/*
 *  Draw a ball
 *     at (x,y,z)
 *     radius (r)
 */
static void ball(double x, double y, double z, double r)
{
   //  Save transformation
   glPushMatrix();
   //  Offset, scale and rotate
   glTranslated(x, y, z);
   glScaled(r, r, r);
   //  White ball with yellow specular
   float yellow[] = {1.0, 1.0, 0.0, 1.0};
   float Emission[] = {0.0, 0.0, 0.01 * emission, 1.0};
   glColor3f(1, 1, 1);
   glMaterialf(GL_FRONT, GL_SHININESS, 1);
   glMaterialfv(GL_FRONT, GL_SPECULAR, yellow);
   glMaterialfv(GL_FRONT, GL_EMISSION, Emission);
   //  Bands of latitude
   for (int ph = -90; ph < 90; ph += inc)
   {
      glBegin(GL_QUAD_STRIP);
      for (int th = 0; th <= 360; th += 2 * inc)
      {
         Vertex(th, ph);
         Vertex(th, ph + inc);
      }
      glEnd();
   }
   glPopMatrix();
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
   case 0: // Orthogonal
      glRotatef(ph, 1, 0, 0);
      glRotatef(th, 0, 1, 0);
      break;

   case 1: // Perspective
   {
      double Ex = -2 * dim * Sin(th) * Cos(ph);
      double Ey = +2 * dim * Sin(ph);
      double Ez = +2 * dim * Cos(th) * Cos(ph);
      gluLookAt(Ex, Ey, Ez, 0, 0, 0, 0, Cos(ph), 0);
      break;
   }

   case 2: // POV view
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
      glDisable(GL_COLOR_MATERIAL);
      drawF1Car(1, 1, 1, ferrariColors);
      glEnable(GL_COLOR_MATERIAL);
      break;

   case 1:
      glDisable(GL_COLOR_MATERIAL);
      drawF1Garage(0, 0, 0, 1);
      glEnable(GL_COLOR_MATERIAL);
      break;
   case 2:
      drawPitComplex();
      break;
   case 3:
      drawF1Car(1, 1, 1, ferrariColors);
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
   Project();
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
      perspective = (perspective + 1) % 3;
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
   if (perspective == 2)
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
      else if (ch == 'a' || ch == 'A') // Strafe right
      {
         povX += step * sin(radTh + M_PI / 2);
         povZ += step * cos(radTh + M_PI / 2);
      }
      else if (ch == 'd' && ch != 'D') // Strafe left
      {
         povX -= step * sin(radTh + M_PI / 2);
         povZ -= step * cos(radTh + M_PI / 2);
      }
   }

   // Handle axes toggle
   if (ch == 'q')
      axes = 1 - axes;
   Project();
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void reshape(int width, int height)
{
   // Ratio of the width to the height of the window
   asp = (height > 0) ? (double)width / height : 1;
   //  Set the viewport to the entire window
   glViewport(0, 0, width, height);
   //  Set projection
   Project();
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