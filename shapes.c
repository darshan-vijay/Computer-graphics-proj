
#include "CSCIx229.h"

/*
 *  Draw a cube
 *     at (x,y,z)
 *     dimensions (dx,dy,dz)
 *     rotated th about the y axis
 */
void cube(double x, double y, double z,
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
void noTexCube(double x, double y, double z,
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
void trapezoid(double x, double y, double z,
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

void cylinder(double x, double y, double z,
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

void rectangle(double x, double y, double z,    // center position
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
 *  Draw a ball
 *     at (x,y,z)
 *     radius (r)
 */
void ball(double x, double y, double z, double r)
{
   //  Save transformation
   glPushMatrix();
   //  Offset, scale and rotate
   glTranslated(x, y, z);
   glScaled(r, r, r);
   //  White ball with yellow specular
   float yellow[] = {1.0, 1.0, 0.0, 1.0};
   float Emission[] = {0.0, 0.0, 0.01 * 1, 1.0};
   glColor3f(1, 1, 1);
   glMaterialf(GL_FRONT, GL_SHININESS, 1);
   glMaterialfv(GL_FRONT, GL_SPECULAR, yellow);
   glMaterialfv(GL_FRONT, GL_EMISSION, Emission);
   //  Bands of latitude
   for (int ph = -90; ph < 90; ph += 10)
   {
      glBegin(GL_QUAD_STRIP);
      for (int th = 0; th <= 360; th += 2 * 10)
      {
         Vertex(th, ph);
         Vertex(th, ph + 10);
      }
      glEnd();
   }
   glPopMatrix();
}
