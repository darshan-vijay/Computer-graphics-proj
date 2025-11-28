#include "CSCIx229.h"

#define NUM_CURVE_POINTS 50
#define NUM_ROTATIONS 50

double P[3][3] = {
    {0.255, 0.0, 0.0},
    {-0.102, 0.0, 0.470},
    {-0.526, 0.0, 0.360}};

double curve[NUM_CURVE_POINTS + 1][3];

void EvaluateBezier(double t, double result[3])
{
    double t2 = t * t;
    double mt = 1.0 - t;
    double mt2 = mt * mt;

    // Quadratic Bezier: B(t) = (1-t)^2 * P0 + 2(1-t)tP1 + t^2* P2
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
    glLineWidth(3);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= NUM_CURVE_POINTS; i++)
        glVertex3f(curve[i][0], curve[i][1], curve[i][2]);
    glEnd();
    glLineWidth(1);
}

void DrawRotationObject()
{

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

    // circular end cap at X = -0.526
    double end_x = curve[NUM_CURVE_POINTS][0]; // x = -0.526
    double end_z = curve[NUM_CURVE_POINTS][2]; // z = 0.360 (radius of circle)

    glBegin(GL_TRIANGLE_FAN);

    // Normal for end cap points in -X direction
    glNormal3f(-1, 0, 0);
    glVertex3f(end_x, 0, 0); // Center of the circle

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

void drawF1Car(float length, float width, float breadth, unsigned int texture[], float colors[][3], float steeringAngle, int isBraking)
{
    // colors[0] = body color
    // colors[1] = fin/wing color (rear and front wings)
    // colors[2] = reinforcement bar color
    // steeringAngle - angle to rotate front wheels
    // isBraking - 1 if braking, 0 otherwise (for brake light)

    // Scaling factors
    float scaleX = length;
    float scaleY = breadth;
    float scaleZ = width;

    glPushMatrix();
    glTranslated(0, 0.65 * scaleY, 0);
    glScalef(scaleX, scaleY, scaleZ);

    // Base metal body - using body color
    SetMaterial(colors[0][0], colors[0][1], colors[0][2],
                colors[0][0] * 1.3, colors[0][1] * 1.3, colors[0][2] * 1.3,
                1.0, 1.0, 1.0, 100);

    // First trapezoid aligned along X, base on ground
    glPushMatrix();
    trapezoid(0, 0, 0,
              1, 1, 1,
              0, 0, -90,
              0.2, 0.2,
              0.35, 0.35,
              1.5);
    glPopMatrix();

    // --- Second trapezoid front taper
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

    // Third trapezoid nose tip
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

    // Rear body larger segment behind cockpit
    glPushMatrix();
    glTranslated(-1.5, 0, 0);
    trapezoid(0, 0, 0,
              1, 1, 1,
              0, 0, -90,
              0.35, 0.35,
              0.35, 1,
              1.5);
    glPopMatrix();

    // Rear cube section
    glPushMatrix();
    glTranslated(-2.25, 0, 0);
    noTexCube(0, 0, 0,
              0.75, 0.35, 1,
              0);
    glPopMatrix();

    // Rear connection trapezoid
    glPushMatrix();
    glTranslated(-4, 0, 0);
    trapezoid(0, 0, 0,
              1, 1, 1,
              0, 0, -90,
              0.35, 1,
              0.35, 0.6,
              1.0);
    glPopMatrix();

    // Rear cube end
    glPushMatrix();
    glTranslated(-4.3, 0, 0);
    noTexCube(0, 0, 0,
              0.6, 0.35, 0.6,
              0);
    glPopMatrix();

    // Rear wing and fin
    SetMaterial(colors[1][0], colors[1][1], colors[1][2],
                colors[1][0] * 1.2, colors[1][1] * 1.2, colors[1][2] * 1.2,
                0.8, 0.8, 0.8, 80);

    rectangle(-4.4, 1.06, 0, 0.45, 1.6, -90, 0, 0); // back fin
    rectangle(-4.4, 1.11, 0.8, 0.45, 0.1, 0, 0, 0);
    rectangle(-4.4, 1.11, -0.8, 0.45, 0.1, 0, 0, 0);
    rectangle(-4.4, 0.16, -0.39, 0.45, 1, 0, 0, 0);
    rectangle(-4.4, 0.16, 0.39, 0.45, 1, 0, 0, 0);
    rectangle(-4.4, 0.85, -0.6, 0.45, 0.62, -45, 0, 0);
    rectangle(-4.4, 0.85, 0.6, 0.45, 0.62, 45, 0, 0);

    // rear light
    if (isBraking)
    {
        // red glowing brake lights when braking
        SetMaterial(1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.3, 0.3, 50);
        glDisable(GL_LIGHTING);
        glColor3f(1.0, 0.0, 0.0);
    }
    else
    {
        // Dim red when not braking
        SetMaterial(0.3, 0.0, 0.0, 0.3, 0.0, 0.0, 0.1, 0.0, 0.0, 10);
    }

    // brake light
    glPushMatrix();
    glTranslated(-4.65, 0.5, 0);
    noTexCube(0, 0, 0, 0.08, 0.15, 0.1, 0);
    glPopMatrix();

    if (isBraking)
    {
        glEnable(GL_LIGHTING); // Re-enable lighting
    }

    // front wings
    SetMaterial(colors[1][0], colors[1][1], colors[1][2],
                colors[1][0] * 1.2, colors[1][1] * 1.2, colors[1][2] * 1.2,
                0.8, 0.8, 0.8, 80);
    rectangle(2.4, -0.15, 0.6, 0.45, 1.5, -90, 0, -18);
    rectangle(2.4, -0.15, -0.6, 0.45, 1.5, -90, 0, 18);

    // Suspension
    SetMaterial(0.25, 0.25, 0.28, 0.4, 0.4, 0.43, 1.0, 1.0, 1.0, 100);

    // Front left suspension
    cylinder(1, 0.11, -0.175, 0.025, 2.8, 20, 90, 10, -10, 0, 0, 0);
    cylinder(1, -0.11, -0.175, 0.025, 2.8, 20, 90, 10, 10, 0, 0, 0);
    // Front right suspension
    cylinder(1, 0.11, 0.175, 0.025, 2.8, 20, 90, 10, 10, 0, 0, 0);
    cylinder(1, -0.11, 0.175, 0.025, 2.8, 20, 90, 10, -10, 0, 0, 0);
    // Rear left suspension
    cylinder(-4, 0.15, -0.175, 0.025, 2.8, 20, 90, 10, -10, 0, 0, 0);
    cylinder(-4, -0.15, -0.175, 0.025, 2.8, 20, 90, 10, 10, 0, 0, 0);
    // Rear right suspension
    cylinder(-4, 0.15, 0.175, 0.025, 2.8, 20, 90, 10, 10, 0, 0, 0);
    cylinder(-4, -0.15, 0.175, 0.025, 2.8, 20, 90, 10, -10, 0, 0, 0);

    // top handle on bezier

    cylinder(-2.2, 0.88, 0, 0.02, 0.38, 8, 0, 0, 0, 0, 0, 0);

    cylinder(-2.2, 1.06, 0, 0.02, 0.2, 8, 90, 0, 0, 0, 0, 0);

    // Wheels
    // Black for tires
    SetMaterial(0.01, 0.01, 0.01, 0.05, 0.05, 0.05, 0.1, 0.1, 0.1, 5);

    // Front wheels with steering rotation
    glPushMatrix();
    glRotated(-steeringAngle * 0.4, 0, 1, 0); // Rotate around Y axis for steering
    cylinder(1, 0, -1.35, 0.6, 0.6, 20, 90, 0, 0, 0, 0, 0);
    glPopMatrix();

    glPushMatrix();
    glRotated(-steeringAngle * 0.4, 0, 1, 0); // Rotate around Y axis for steering
    cylinder(1, 0, 1.35, 0.6, 0.6, 20, 90, 0, 0, 0, 0, 0);
    glPopMatrix();

    // Rear wheels
    cylinder(-4, 0, -1.35, 0.6, 0.6, 20, 90, 0, 0, 0, 0, 0);
    cylinder(-4, 0, 1.35, 0.6, 0.6, 20, 90, 0, 0, 0, 0, 0);

    // cockpit bezier and halo
    SetMaterial(colors[0][0], colors[0][1], colors[0][2],
                colors[0][0] * 1.3, colors[0][1] * 1.3, colors[0][2] * 1.3,
                1.0, 1.0, 1.0, 100);

    glPushMatrix();
    glTranslated(-3.7, 0.15, 0);
    glRotatef(180, 0, 1, 0);
    glScalef(3, 2.2, 1);
    DrawBezierLine();
    DrawRotationObject();
    glPopMatrix();

    // Front cockpit trapezoid
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

    // Halo reinforcement bars
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

void squareBracketMarking()
{
    // Square bracket like marking
    SetMaterial(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.3, 0.3, 0.3, 30);
    glPushMatrix();
    glScaled(0.3, 0.3, 0.3);
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    // top left small line
    glVertex3f(5.2, 0.01, 2.5);
    glVertex3f(4.4, 0.01, 2.5);
    glVertex3f(4.4, 0.01, 2);
    glVertex3f(5.2, 0.01, 2);
    // top right small line
    glVertex3f(5.2, 0.01, -2.5);
    glVertex3f(4.4, 0.01, -2.5);
    glVertex3f(4.4, 0.01, -2);
    glVertex3f(5.2, 0.01, -2);
    // Yellow line top
    glVertex3f(5, 0.01, -2.5);
    glVertex3f(5.2, 0.01, -2.5);
    glVertex3f(5.2, 0.01, 2.5);
    glVertex3f(5, 0.01, 2.5);
    glEnd();
    glPopMatrix();
}

void drawF1Garage(double x, double y, double z, double scale, unsigned int texture[], float colors[][3])
{

    glPushMatrix();
    glTranslated(x, y, z);
    glScaled(scale, scale, scale);

    // Square bracket like marking
    SetMaterial(1.0, 1.0, 0.1, 1.0, 1.0, 0.2, 0.3, 0.3, 0.3, 30);
    glPushMatrix();
    glTranslated(-2, 0.01, 12);
    glRotatef(180, 0, 1, 0);
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    // top left small line
    glVertex3f(5.2, 0.01, 2.5);
    glVertex3f(4.8, 0.01, 2.5);
    glVertex3f(4.8, 0.01, 2);
    glVertex3f(5.2, 0.01, 2);
    // top right small line
    glVertex3f(5.2, 0.01, -2.5);
    glVertex3f(4.8, 0.01, -2.5);
    glVertex3f(4.8, 0.01, -2);
    glVertex3f(5.2, 0.01, -2);
    // Yellow line top
    glVertex3f(5, 0.01, -2.5);
    glVertex3f(5.2, 0.01, -2.5);
    glVertex3f(5.2, 0.01, 2.5);
    glVertex3f(5, 0.01, 2.5);
    glEnd();
    glPopMatrix();

    // Garage dimensions - 16 units wide, 12 units deep, 6 units tall

    // Floor
    SetMaterial(0.15, 0.15, 0.15, 0.3, 0.3, 0.3, 0.1, 0.1, 0.1, 10);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[1]); // Concrete texture
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glTexCoord2f(0, 0);
    glVertex3f(-8, 0, -6);
    glTexCoord2f(8, 0);
    glVertex3f(8, 0, -6);
    glTexCoord2f(8, 6);
    glVertex3f(8, 0, 6);
    glTexCoord2f(0, 6);
    glVertex3f(-8, 0, 6);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    // back wall
    SetMaterial(0.2, 0.2, 0.22, 0.4, 0.4, 0.45, 0.1, 0.1, 0.1, 10);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    glTexCoord2f(0, 0);
    glVertex3f(-8, 0, -6);
    glTexCoord2f(8, 0);
    glVertex3f(8, 0, -6);
    glTexCoord2f(8, 3);
    glVertex3f(8, 6, -6);
    glTexCoord2f(0, 3);
    glVertex3f(-8, 6, -6);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    // side walls
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    // Left wall
    glBegin(GL_QUADS);
    glNormal3f(1, 0, 0);
    glTexCoord2f(0, 0);
    glVertex3f(-8, 0, -6);
    glTexCoord2f(6, 0);
    glVertex3f(-8, 0, 6);
    glTexCoord2f(6, 3);
    glVertex3f(-8, 6, 6);
    glTexCoord2f(0, 3);
    glVertex3f(-8, 6, -6);
    glEnd();
    // right wall
    glBegin(GL_QUADS);
    glNormal3f(-1, 0, 0);
    glTexCoord2f(0, 0);
    glVertex3f(8, 0, 6);
    glTexCoord2f(6, 0);
    glVertex3f(8, 0, -6);
    glTexCoord2f(6, 3);
    glVertex3f(8, 6, -6);
    glTexCoord2f(0, 3);
    glVertex3f(8, 6, 6);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    // ceiling
    SetMaterial(0.25, 0.25, 0.25, 0.5, 0.5, 0.5, 0.2, 0.2, 0.2, 20);
    glBegin(GL_QUADS);
    glNormal3f(0, -1, 0);
    glVertex3f(-8, 6, -6);
    glVertex3f(8, 6, -6);
    glVertex3f(8, 6, 6);
    glVertex3f(-8, 6, 6);
    glEnd();

    // Banner
    glPushMatrix();
    glRotatef(90, 0, 1, 0);
    SetMaterial(0.2, 0.2, 0.2, 0.4, 0.4, 0.4, 0.2, 0.2, 0.2, 30);
    glColor3f(0.3, 0.3, 0.3);
    noTexCube(0, 7, 0, 0.1, 1.2, 2.5, 0);
    glPopMatrix();

    // Overhead lighting
    SetMaterial(0.9, 0.9, 0.85, 1.0, 1.0, 0.95, 0.8, 0.8, 0.8, 80);
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            double lx = -4 + i * 4;
            double lz = -2 + j * 4;
            glBegin(GL_QUADS);
            glNormal3f(0, -1, 0);
            glVertex3f(lx - 0.9, 5.95, lz - 0.6);
            glVertex3f(lx + 0.9, 5.95, lz - 0.6);
            glVertex3f(lx + 0.9, 5.95, lz + 0.6);
            glVertex3f(lx - 0.9, 5.95, lz + 0.6);
            glEnd();
        }
    }

    glPushMatrix();
    glTranslatef(-0.3, 0, 0.3);
    // Red tool cabinet
    SetMaterial(0.7, 0.1, 0.1, 0.9, 0.2, 0.2, 0.3, 0.3, 0.3, 40);
    noTexCube(-6, 1.2, -4.5, 1.0, 1.2, 1.5, 0);

    // Cabinet top surface
    SetMaterial(0.5, 0.5, 0.5, 0.7, 0.7, 0.7, 0.4, 0.4, 0.4, 50);
    noTexCube(-6, 2.45, -4.5, 1.05, 0.06, 1.55, 0);

    // Cabinet drawers
    SetMaterial(0.5, 0.08, 0.08, 0.7, 0.15, 0.15, 0.2, 0.2, 0.2, 30);
    for (int i = 0; i < 5; i++)
    {
        noTexCube(-5.5, 0.4 + i * 0.45, -4.5, 0.9, 0.2, 1.4, 0);
    }

    glPopMatrix();

    // brown bench
    SetMaterial(0.4, 0.35, 0.3, 0.6, 0.5, 0.45, 0.1, 0.1, 0.1, 10);
    noTexCube(6, 1.1, -4.3, 1.2, 0.08, 1.5, 0);

    // Workbench side panels
    SetMaterial(0.35, 0.3, 0.25, 0.55, 0.45, 0.4, 0.1, 0.1, 0.1, 10);
    noTexCube(4.9, 0.55, -4.5, 0.08, 0.55, 1.4, 0);
    noTexCube(7.1, 0.55, -4.5, 0.08, 0.55, 1.4, 0);

    // Tire rack
    SetMaterial(0.2, 0.2, 0.22, 0.4, 0.4, 0.42, 0.3, 0.3, 0.3, 30);
    // Rack frame vertical bars
    cylinder(6.5, 2, 0.5, 0.05, 4, 8, 0, 0, 0, 0, 0, 0);
    cylinder(7.5, 2, 0.5, 0.05, 4, 8, 0, 0, 0, 0, 0, 0);
    // Horizontal bars
    cylinder(7, 0.8, 0.5, 0.05, 1, 8, 90, 0, -90, 0, 0, 0);
    cylinder(7, 1.6, 0.5, 0.05, 1, 8, 90, 0, -90, 0, 0, 0);
    cylinder(7, 2.4, 0.5, 0.05, 1, 8, 90, 0, -90, 0, 0, 0);
    cylinder(7, 3.2, 0.5, 0.05, 1, 8, 90, 0, -90, 0, 0, 0);

    // Tires on rack
    SetMaterial(0.01, 0.01, 0.01, 0.05, 0.05, 0.05, 0.1, 0.1, 0.1, 5);
    for (int i = 0; i < 4; i++)
    {
        cylinder(7, 0.8 + i * 0.8, 0.5, 0.35, 0.25, 16, 90, 0, -90, 0, 0, 0);
    }

    // Computer table
    glPushMatrix();
    glTranslated(-6.85, 0, 2);
    glRotated(90, 0, 1, 0);

    // Desk
    SetMaterial(0.15, 0.15, 0.17, 0.3, 0.3, 0.35, 0.2, 0.2, 0.2, 30);
    noTexCube(0, 0.8, 0, 1.0, 0.08, 1.0, 0);

    // Desk legs
    SetMaterial(0.2, 0.2, 0.22, 0.4, 0.4, 0.42, 0.3, 0.3, 0.3, 30);
    cylinder(-0.8, 0.4, -0.4, 0.04, 0.8, 6, 0, 0, 0, 0, 0, 0);
    cylinder(-0.8, 0.4, 0.4, 0.04, 0.8, 6, 0, 0, 0, 0, 0, 0);
    cylinder(0.8, 0.4, -0.4, 0.04, 0.8, 6, 0, 0, 0, 0, 0, 0);
    cylinder(0.8, 0.4, 0.4, 0.04, 0.8, 6, 0, 0, 0, 0, 0, 0);

    // Monitor
    SetMaterial(0.05, 0.05, 0.05, 0.1, 0.1, 0.1, 0.3, 0.3, 0.3, 40);
    noTexCube(0, 1.3, 0, 0.55, 0.4, 0.04, 0);

    // Monitor bezel
    SetMaterial(0.02, 0.02, 0.02, 0.05, 0.05, 0.05, 0.2, 0.2, 0.2, 50);
    noTexCube(0, 1.3, 0.04, 0.57, 0.42, 0.02, 0);

    // Screen
    SetMaterial(0.1, 0.3, 0.6, 0.2, 0.5, 1.0, 0.5, 0.7, 1.0, 60);
    noTexCube(0, 1.3, 0.055, 0.5, 0.35, 0.01, 0);

    // Monitor stand
    SetMaterial(0.3, 0.3, 0.32, 0.5, 0.5, 0.52, 0.4, 0.4, 0.4, 50);
    noTexCube(0, 0.9, 0, 0.08, 0.06, 0.08, 0);
    cylinder(0, 0.95, 0, 0.025, 0.28, 6, 0, 0, 0, 0, 0, 0);

    // Keyboard
    SetMaterial(0.1, 0.1, 0.12, 0.2, 0.2, 0.22, 0.1, 0.1, 0.1, 20);
    noTexCube(0, 0.88, 0.45, 0.4, 0.025, 0.18, 0);

    // Mouse
    SetMaterial(0.15, 0.15, 0.17, 0.25, 0.25, 0.27, 0.3, 0.3, 0.3, 40);
    noTexCube(0.7, 0.88, 0.25, 0.07, 0.02, 0.1, 0);

    glPopMatrix();

    // Poster background
    SetMaterial(colors[0][0] * 0.9, colors[0][1] * 0.9, colors[0][2] * 0.9,
                colors[0][0], colors[0][1], colors[0][2],
                0.3, 0.3, 0.3, 40);
    noTexCube(0, 4.0, -5.95, 2.8, 1.4, 0.02, 0);

    // White border around poster
    SetMaterial(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 60);
    noTexCube(0, 4.0, -5.93, 2.9, 1.5, 0.015, 0);

    // Team logo elements
    SetMaterial(colors[1][0], colors[1][1], colors[1][2],
                colors[1][0] * 1.2, colors[1][1] * 1.2, colors[1][2] * 1.2,
                0.8, 0.8, 0.8, 80);
    noTexCube(-0.9, 4.0, -5.90, 0.3, 1.0, 0.01, 0);
    noTexCube(0.9, 4.0, -5.90, 0.3, 1.0, 0.01, 0);
    noTexCube(0, 4.0, -5.90, 0.4, 0.7, 0.01, 0);

    // board leaned against wall
    SetMaterial(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.2, 0.2, 0.2, 10);
    glPushMatrix();
    glTranslated(-7.7, 1.0, -1);
    glRotated(15, 0, 0, 1);
    noTexCube(0, 0, 0, 0.03, 0.7, 0.5, 0);
    glPopMatrix();

    // Pit board handle
    SetMaterial(0.3, 0.3, 0.3, 0.5, 0.5, 0.5, 0.2, 0.2, 0.2, 20);
    cylinder(-7.7, 0.3, -1, 0.02, 0.7, 6, 0, 0, 0, 0, 0, 0);

    // Tool Board
    SetMaterial(0.6, 0.5, 0.4, 0.8, 0.7, 0.6, 0.1, 0.1, 0.1, 10);
    noTexCube(7.95, 3, 3, 0.03, 1.5, 1.2, 0);

    // Tools hanging on board
    SetMaterial(0.5, 0.5, 0.55, 0.8, 0.8, 0.85, 1.0, 1.0, 1.0, 100);
    for (int i = 0; i < 4; i++)
    {
        double tz = 2.2 + i * 0.5;
        cylinder(7.9, 3.5, tz, 0.02, 0.4, 6, 90, 0, 0, 0, 0, 0);
        cylinder(7.9, 2.5, tz, 0.02, 0.4, 6, 90, 0, 0, 0, 0, 0);
    }

    // Air compressor
    SetMaterial(0.9, 0.5, 0.1, 1.0, 0.6, 0.2, 0.4, 0.4, 0.4, 40);
    noTexCube(5.5, 0.4, -1.5, 0.4, 0.4, 0.5, 0);
    // Compressor tank
    cylinder(5.5, 0.8, -1.5, 0.25, 0.6, 12, 0, 0, 0, 0, 0, 0);
    // Pressure gauge
    SetMaterial(0.8, 0.8, 0.8, 1.0, 1.0, 1.0, 0.6, 0.6, 0.6, 70);
    cylinder(5.5, 0.5, -1.5, 0.06, 0.025, 10, 90, 0, 0, 0, 0, 0);

    // Jack stands
    SetMaterial(0.9, 0.7, 0.1, 1.0, 0.8, 0.2, 0.5, 0.5, 0.5, 60);
    // Left jack
    glPushMatrix();
    glTranslated(-2.5, 0, 1.8);
    noTexCube(0, 0.15, 0, 0.15, 0.15, 0.15, 0);
    cylinder(0, 0.28, 0, 0.04, 0.4, 6, 0, 0, 0, 0, 0, 0);
    noTexCube(0, 0.5, 0, 0.12, 0.03, 0.12, 0);
    glPopMatrix();

    // Right jack
    glPushMatrix();
    glTranslated(2.5, 0, 1.5);
    noTexCube(0, 0.15, 0, 0.15, 0.15, 0.15, 0);
    cylinder(0, 0.28, 0, 0.04, 0.4, 6, 0, 0, 0, 0, 0, 0);
    noTexCube(0, 0.5, 0, 0.12, 0.03, 0.12, 0);
    glPopMatrix();

    // Safety yellow lines on garage floor
    SetMaterial(1.0, 1.0, 0.1, 1.0, 1.0, 0.2, 0.3, 0.3, 0.3, 30);
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    // Yellow line left
    glVertex3f(3, 0.01, -5);
    glVertex3f(3.2, 0.01, -5);
    glVertex3f(3.2, 0.01, 5);
    glVertex3f(3, 0.01, 5);
    // Yellow line right
    glVertex3f(-3, 0.01, -5);
    glVertex3f(-3.2, 0.01, -5);
    glVertex3f(-3.2, 0.01, 5);
    glVertex3f(-3, 0.01, 5);
    glEnd();

    // THE F1 CAR
    glPushMatrix();
    glRotated(-90, 0, 1, 0); // Rotate to face front opening
    drawF1Car(1, 1, 1, texture, colors, 0, 0);
    glPopMatrix();

    glPopMatrix();
}

// Draw a stack of tires as a tire barrier with numTires in stack and colors
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

        // Inner hole
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
        float *color = colors[(i / 5) % 3];
        drawTireBarrier(startX + i * spacing, y, z, 4, color[0], color[1], color[2]);
    }
}

// not used now
// Function to draw a building with windows
void drawStand(double x, double y, double z, double width, double height, double depth, unsigned int texture[])
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

// not used now
// Draw trees
void drawTree(double x, double y, double z, double scale, unsigned int texture[])
{
    // Enable texture
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[4]);
    SetMaterial(0.08, 0.04, 0.02, // Brown ambient
                0.4, 0.2, 0.1,    // Brown diffuse
                0.05, 0.05, 0.05, // Very low specular
                2);               // Very low shininess
    glColor3f(0.4, 0.2, 0.1);     // Brown

    // Draw a textured bark cylinder
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
    cube(0, 0, 0, 1, 1, 1, 0, 1, 1, 1);
    cube(0, 0.3, 0, 0.8, 0.8, 0.8, 0, 1, 1, 1);
    cube(0, -0.3, 0, 0.8, 0.8, 0.8, 0, 1, 1, 1);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

// not used now
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

// Draw a road block with red and white curbs on edges and barricades
void drawRoadBlockWithCurbs(double x, double y, double z, double width, double length, double rotation, unsigned int texture[], int curbs, unsigned int barricadeTextures[], int numBarricadeTextures)
{
    glPushMatrix();
    glTranslated(x, y, z);
    glRotated(rotation, 0, 1, 0);

    // Main road surface
    SetMaterial(0.4, 0.4, 0.4, 0.7, 0.7, 0.7, 0.2, 0.2, 0.2, 10);
    glColor3f(1.0, 1.0, 1.0);

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

    // Curb parameters
    double curbWidth = 0.2;
    int numSegments = 8;
    double segmentLength = length / numSegments;
    if (curbs)
    {
        // Draw alternating red and white curb segments
        for (int i = 0; i < numSegments; i++)
        {
            double segZ = -length / 2 + i * segmentLength;

            // Alternate between red and white
            if (i % 2 == 0)
            {
                SetMaterial(1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.3, 0.3, 0.3, 20);
                glColor3f(1.0, 0.0, 0.0);
            }
            else
            {
                SetMaterial(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.3, 0.3, 0.3, 20);
                glColor3f(1.0, 1.0, 1.0);
            }

            // Left curb segment
            glBegin(GL_QUADS);
            glNormal3f(0, 1, 0);
            glVertex3f(-width / 2, 0.01, segZ);
            glVertex3f(-width / 2 + curbWidth, 0.01, segZ);
            glVertex3f(-width / 2 + curbWidth, 0.01, segZ + segmentLength);
            glVertex3f(-width / 2, 0.01, segZ + segmentLength);
            glEnd();

            // Right curb segment
            glBegin(GL_QUADS);
            glNormal3f(0, 1, 0);
            glVertex3f(width / 2 - curbWidth, 0.01, segZ);
            glVertex3f(width / 2, 0.01, segZ);
            glVertex3f(width / 2, 0.01, segZ + segmentLength);
            glVertex3f(width / 2 - curbWidth, 0.01, segZ + segmentLength);
            glEnd();
        }
    }

    glColor3f(1.0, 1.0, 1.0);
    SetMaterial(0.8, 0.8, 0.8, 0.9, 0.9, 0.9, 0.5, 0.5, 0.5, 50);

    // Add barricades on both sides
    if (barricadeTextures != NULL && numBarricadeTextures > 0)
    {
        int barricadeCount = 0;
        double barricadeOffset = width / 2;

        for (double zPos = -length / 2; zPos < length / 2; zPos += 1.0)
        {
            int texIndex = (barricadeCount / 10) % numBarricadeTextures;

            // Left side barricade (facing inward toward road)
            drawBarricade(-barricadeOffset, 0, zPos, 0, barricadeTextures[texIndex]);

            // Right side barricade (facing inward toward road)
            drawBarricade(barricadeOffset, 0, zPos, 180, barricadeTextures[texIndex]);

            barricadeCount++;
        }
    }

    glPopMatrix();
}

// Draw a right turn road block with red and white curbs
void drawRoadBlockRightTurn(double x, double y, double z, double innerRadius, double width, double rotation, double degreeTurn, unsigned int texture[], int curbs)
{
    glPushMatrix();
    glTranslated(x, y, z);
    glRotated(rotation, 0, 1, 0);

    double outerRadius = innerRadius + width;
    int segments = 16;                        // Number of segments for smooth curve
    double angleStep = degreeTurn / segments; // degree turn

    // Main road surface
    SetMaterial(0.4, 0.4, 0.4, 0.7, 0.7, 0.7, 0.2, 0.2, 0.2, 10);
    glColor3f(1.0, 1.0, 1.0);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[0]);

    for (int i = 0; i < segments; i++)
    {
        double angle1 = i * angleStep * M_PI / 180.0;
        double angle2 = (i + 1) * angleStep * M_PI / 180.0;

        glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);
        glTexCoord2f(0, (float)i / segments);
        glVertex3f(innerRadius * cos(angle1), 0, innerRadius * sin(angle1));
        glTexCoord2f(1, (float)i / segments);
        glVertex3f(outerRadius * cos(angle1), 0, outerRadius * sin(angle1));
        glTexCoord2f(1, (float)(i + 1) / segments);
        glVertex3f(outerRadius * cos(angle2), 0, outerRadius * sin(angle2));
        glTexCoord2f(0, (float)(i + 1) / segments);
        glVertex3f(innerRadius * cos(angle2), 0, innerRadius * sin(angle2));
        glEnd();
    }

    glDisable(GL_TEXTURE_2D);

    // Curb parameters
    double curbWidth = 0.2;
    int curbSegments = 8;
    double curbAngleStep = degreeTurn / curbSegments;
    if (curbs)
    {
        // Draw alternating red and white curb segments along the curve
        for (int i = 0; i < curbSegments; i++)
        {
            double angle1 = i * curbAngleStep * M_PI / 180.0;
            double angle2 = (i + 1) * curbAngleStep * M_PI / 180.0;

            // Alternate between red and white
            if (i % 2 == 0)
            {
                SetMaterial(1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.3, 0.3, 0.3, 20);
                glColor3f(1.0, 0.0, 0.0);
            }
            else
            {
                SetMaterial(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.3, 0.3, 0.3, 20);
                glColor3f(1.0, 1.0, 1.0);
            }

            // Inner curb segment
            glBegin(GL_QUADS);
            glNormal3f(0, 1, 0);
            glVertex3f(innerRadius * cos(angle1), 0.01, innerRadius * sin(angle1));
            glVertex3f((innerRadius + curbWidth) * cos(angle1), 0.01, (innerRadius + curbWidth) * sin(angle1));
            glVertex3f((innerRadius + curbWidth) * cos(angle2), 0.01, (innerRadius + curbWidth) * sin(angle2));
            glVertex3f(innerRadius * cos(angle2), 0.01, innerRadius * sin(angle2));
            glEnd();

            // Outer curb segment
            glBegin(GL_QUADS);
            glNormal3f(0, 1, 0);
            glVertex3f((outerRadius - curbWidth) * cos(angle1), 0.01, (outerRadius - curbWidth) * sin(angle1));
            glVertex3f(outerRadius * cos(angle1), 0.01, outerRadius * sin(angle1));
            glVertex3f(outerRadius * cos(angle2), 0.01, outerRadius * sin(angle2));
            glVertex3f((outerRadius - curbWidth) * cos(angle2), 0.01, (outerRadius - curbWidth) * sin(angle2));
            glEnd();
        }
    }

    glColor3f(1.0, 1.0, 1.0);

    glPopMatrix();
}

// Draw a left turn road block with red and white curbs
void drawRoadBlockLeftTurn(double x, double y, double z, double innerRadius, double width, double rotation, double degreeTurn, unsigned int texture[], int curbs)
{
    glPushMatrix();
    glTranslated(x, y, z);
    glRotated(rotation, 0, 1, 0);

    double outerRadius = innerRadius + width;
    int segments = 16; // Number of segments for smooth curve
    double angleStep = degreeTurn / segments;

    // Main road surface
    SetMaterial(0.4, 0.4, 0.4, 0.7, 0.7, 0.7, 0.2, 0.2, 0.2, 10);
    glColor3f(1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[0]);

    for (int i = 0; i < segments; i++)
    {
        double angle1 = i * angleStep * M_PI / 180.0;
        double angle2 = (i + 1) * angleStep * M_PI / 180.0;

        glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);
        glTexCoord2f(0, (float)i / segments);
        glVertex3f(innerRadius * cos(angle1), 0, -innerRadius * sin(angle1));
        glTexCoord2f(1, (float)i / segments);
        glVertex3f(outerRadius * cos(angle1), 0, -outerRadius * sin(angle1));
        glTexCoord2f(1, (float)(i + 1) / segments);
        glVertex3f(outerRadius * cos(angle2), 0, -outerRadius * sin(angle2));
        glTexCoord2f(0, (float)(i + 1) / segments);
        glVertex3f(innerRadius * cos(angle2), 0, -innerRadius * sin(angle2));
        glEnd();
    }

    glDisable(GL_TEXTURE_2D);

    // Curb parameters
    double curbWidth = 0.2;
    int curbSegments = 8;
    double curbAngleStep = degreeTurn / curbSegments;

    if (curbs)
    {
        // Draw alternating red and white curb segments along the curve
        for (int i = 0; i < curbSegments; i++)
        {
            double angle1 = i * curbAngleStep * M_PI / 180.0;
            double angle2 = (i + 1) * curbAngleStep * M_PI / 180.0;

            // Alternate between red and white
            if (i % 2 == 0)
            {
                SetMaterial(1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.3, 0.3, 0.3, 20);
                glColor3f(1.0, 0.0, 0.0);
            }
            else
            {
                SetMaterial(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.3, 0.3, 0.3, 20);
                glColor3f(1.0, 1.0, 1.0);
            }

            // Inner curb segment
            glBegin(GL_QUADS);
            glNormal3f(0, 1, 0);
            glVertex3f(innerRadius * cos(angle1), 0.01, -innerRadius * sin(angle1));
            glVertex3f((innerRadius + curbWidth) * cos(angle1), 0.01, -(innerRadius + curbWidth) * sin(angle1));
            glVertex3f((innerRadius + curbWidth) * cos(angle2), 0.01, -(innerRadius + curbWidth) * sin(angle2));
            glVertex3f(innerRadius * cos(angle2), 0.01, -innerRadius * sin(angle2));
            glEnd();

            // Outer curb segment
            glBegin(GL_QUADS);
            glNormal3f(0, 1, 0);
            glVertex3f((outerRadius - curbWidth) * cos(angle1), 0.01, -(outerRadius - curbWidth) * sin(angle1));
            glVertex3f(outerRadius * cos(angle1), 0.01, -outerRadius * sin(angle1));
            glVertex3f(outerRadius * cos(angle2), 0.01, -outerRadius * sin(angle2));
            glVertex3f((outerRadius - curbWidth) * cos(angle2), 0.01, -(outerRadius - curbWidth) * sin(angle2));
            glEnd();
        }
    }

    // Reset state after curbs
    glColor3f(1.0, 1.0, 1.0);

    glPopMatrix();
}

// Draws the entire circuit scene, along with garages, pit fence, tire barriers, and grass areas
void drawCircuit(unsigned int texture[], unsigned int barricadeTextures[], int numBarricadeTextures, float colors[][3])
{

    // Draw light grey ground rectangle
    SetMaterial(0.5, 0.5, 0.5, 0.6, 0.6, 0.6, 0.2, 0.2, 0.2, 10);
    glColor3f(0.6, 0.6, 0.6);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[1]); // Grass texture

    glPushMatrix();
    glTranslated(0, -0.01, 0); // Slightly below road level
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);

    glTexCoord2f(0, 0);
    glVertex3f(-80, 0, -80);

    glTexCoord2f(0, 40);
    glVertex3f(80, 0, -80);

    glTexCoord2f(40, 40);
    glVertex3f(80, 0, 80);

    glTexCoord2f(40, 0);
    glVertex3f(-80, 0, 80);
    glEnd();
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);

    // Grass area beside the right side of the track
    SetMaterial(0.1, 0.3, 0.1, 0.2, 0.5, 0.2, 0.05, 0.1, 0.05, 5);
    glColor3f(0.2, 0.5, 0.2);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[2]); // Grass texture

    // Right grass strip
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glTexCoord2f(0, 0);
    glVertex3f(-3, -0.001, 0);
    glTexCoord2f(0, 5);
    glVertex3f(-3, -0.001, 11);
    glTexCoord2f(40, 5);
    glVertex3f(43, -0.001, 11);
    glTexCoord2f(40, 0);
    glVertex3f(43, -0.001, 00);
    glEnd();

    glDisable(GL_TEXTURE_2D);

    // Five F1 garages
    float garagePositions[5] = {
        5,
        12,
        19,
        26,
        33};

    for (int i = 0; i < 5; i++)
    {
        glPushMatrix();
        glTranslated(garagePositions[i], 0, 14);
        glRotated(180, 0, 1, 0);
        glScalef(0.22, 0.22, 0.22);
        drawF1Garage(0, 0, 0, 0.8, texture, colors);
        glPopMatrix();
    }

    glPushMatrix();
    glTranslatef(0, 0, 8);
    glScalef(0.3, 0.3, 0.3);
    drawTireBarrierRow(0, 0, 0, 60, 2);
    glPopMatrix();

    // Pit fence
    SetMaterial(0.3, 0.3, 0.3, 0.5, 0.5, 0.5, 0.3, 0.3, 0.3, 40);
    glColor3f(0.4, 0.4, 0.4);

    glPushMatrix();
    glScaled(0.7, 0.7, 0.7);
    for (int i = 0; i < 15; i++)
    {
        double fenceX = 0 + i * 4;
        cylinder(fenceX, 0, 7, 0.05, 3.0, 8, 0, 0, 0, 0, 0, 0);
    }

    // Horizontal fence bars
    glLineWidth(2.0);
    glBegin(GL_LINES);
    for (int j = 0; j < 6; j++)
    {
        double barY = 0.3 + j * 0.2;
        glVertex3f(0, barY, 7);
        glVertex3f(56, barY, 7);
    }
    glEnd();
    glLineWidth(1.0); // Reset to default
    glPopMatrix();

    glPushMatrix();
    glTranslated(20, 0, 0);
    // Race start/finish straight
    drawRoadBlockWithCurbs(0, 0, 0, 4, 35, 90, texture, 0, barricadeTextures, 3);

    // pit road
    drawRoadBlockWithCurbs(0, 0, 10, 2.5, 35, 90, texture, 0, barricadeTextures, 0);
    drawRoadBlockRightTurn(17.5, 0, 16.25, 5, 2.5, 90, 90, texture, 0);

    // 1st right turn
    drawRoadBlockRightTurn(17.5, 0, 7, 5, 4, 90, 90, texture, 1);

    // Right straight
    drawRoadBlockWithCurbs(24.5, 0, 24, 4, 35, 0, texture, 1, barricadeTextures, 0);

    // 2nd right turn, 130 degrees
    glPushMatrix();
    glTranslated(17.5, 0, 41.5);
    glRotated(-60, 0, 1, 0);
    drawRoadBlockRightTurn(0, 0, 0, 5, 4, 60, 130, texture, 1);
    glPopMatrix();

    // Angled straight
    drawRoadBlockWithCurbs(0, 0, 35.9, 4, 35, 50, texture, 0, barricadeTextures, 3);

    glPopMatrix();

    // angled end
    drawRoadBlockWithCurbs(4.5, 0, 23, 4, 18, 50, texture, 0, barricadeTextures, 0);

    glPushMatrix();
    glTranslated(2.5, 0, 7);
    glRotated(180, 0, 1, 0);
    drawRoadBlockRightTurn(0, 0, 0, 5, 4, 0, 90, texture, 1);
    glPopMatrix();

    // pit road
    glPushMatrix();
    glTranslated(2.5, 0, 16.25);
    glRotated(180, 0, 1, 0);
    drawRoadBlockRightTurn(0, 0, 0, 5, 2.5, 0, 90, texture, 0);
    glPopMatrix();

    drawRoadBlockWithCurbs(-4.5, 0, 9.8, 4, 6, 0, texture, 0, barricadeTextures, 0);

    glPushMatrix();
    glTranslated(2.5, 0, 12.2);
    drawRoadBlockLeftTurn(0, 0, 0, 5, 4, 180, 50, texture, 0);
    glPopMatrix();
}

// Draw a barricade at (x, y, z) with rotation and texture
void drawBarricade(double x, double y, double z, double rotation, unsigned int texture)
{
    glPushMatrix();
    glTranslated(x, y, z);
    glRotated(rotation, 0, 1, 0);
    glScaled(0.33, 0.33, 0.33);

    glPushMatrix();
    glTranslated(0, 1.5, 0);
    // Light grey posts
    SetMaterial(0.6, 0.6, 0.6, 0.7, 0.7, 0.7, 0.9, 0.9, 0.9, 80);
    glColor3f(0.7, 0.7, 0.7);

    // First vertical post
    glPushMatrix();
    glTranslated(0, 0, 0);
    glRotated(90, 1, 0, 0);
    prism(0.1, 0.1, 3);
    glPopMatrix();

    // First diagonal support
    glPushMatrix();
    glTranslated(0.38, 1.78, 0);
    glRotated(90, 1, 0, 0);
    glRotated(-50, 0, 1, 0);
    prism(0.1, 0.1, 1);
    glPopMatrix();

    // Second vertical post
    glPushMatrix();
    glTranslated(0, 0, 3);

    glPushMatrix();
    glTranslated(0, 0, 0);
    glRotated(90, 1, 0, 0);
    prism(0.1, 0.1, 3);
    glPopMatrix();

    glPushMatrix();
    glTranslated(0.38, 1.78, 0);
    glRotated(90, 1, 0, 0);
    glRotated(-50, 0, 1, 0);
    prism(0.1, 0.1, 1);
    glPopMatrix();

    glPopMatrix();

    // Sign/board with texture
    SetMaterial(0.5, 0.5, 0.5, 0.9, 0.9, 0.9, 0.3, 0.3, 0.3, 20);
    glPushMatrix();
    glTranslated(0, -0.75, 1.5);
    rectangleTex(0, 0, 0, 3, 1.5, 0, 90, 0, texture, 1);
    glPopMatrix();

    // Wires between posts
    SetMaterial(0.6, 0.6, 0.6, 0.7, 0.7, 0.7, 0.9, 0.9, 0.9, 80);
    glColor3f(0.7, 0.7, 0.7);
    glLineWidth(1.8);

    int numWires = 8;
    float wireHeight = 1.4 / numWires;

    glBegin(GL_LINES);
    for (int i = 1; i <= numWires; i++)
    {
        glVertex3f(0, i * wireHeight, 0);
        glVertex3f(0, i * wireHeight, 3);
    }
    glEnd();

    glLineWidth(1.0);
    glPopMatrix();
    glPopMatrix();
}

void drawFrameBox()
{
    glPushMatrix();
    glTranslatef(0, 0.5, 0);
    // vertical supports
    cylinder(0, 0, 1, 0.05, 1.1, 4, 0, 0, 0, 0, 0, 0);
    cylinder(0, 0, 0, 0.05, 1.1, 4, 0, 0, 0, 0, 0, 0);
    cylinder(1, 0, 0, 0.05, 1.1, 4, 0, 0, 0, 0, 0, 0);
    cylinder(1, 0, 1, 0.05, 1.1, 4, 0, 0, 0, 0, 0, 0);
    // horizontal top connectors
    cylinder(0.5, 0.5, 1, 0.05, 1, 4, 0, 0, 90, 0, 0, 0);
    cylinder(0, 0.5, 0.5, 0.05, 1, 4, 90, 0, 0, 0, 0, 0);
    cylinder(0.5, 0.5, 0, 0.05, 1, 4, 0, 0, 90, 0, 0, 0);
    cylinder(1, 0.5, 0.5, 0.05, 1, 4, 90, 0, 0, 0, 0, 0);
    // diagonal connectors
    cylinder(0, 0, 0.5, 0.05, 1.42, 4, 45, 0, 0, 0, 0, 0);
    cylinder(0, 0, 0.5, 0.05, 1.42, 4, -45, 0, 0, 0, 0, 0);
    cylinder(1, 0, 0.5, 0.05, 1.42, 4, 45, 0, 0, 0, 0, 0);
    cylinder(1, 0, 0.5, 0.05, 1.42, 4, -45, 0, 0, 0, 0, 0);
    // horizontal bottom connectors
    cylinder(0.5, -0.5, 1, 0.05, 1, 4, 0, 0, 90, 0, 0, 0);
    cylinder(0, -0.5, 0.5, 0.05, 1, 4, 90, 0, 0, 0, 0, 0);
    cylinder(0.5, -0.5, 0, 0.05, 1, 4, 0, 0, 90, 0, 0, 0);
    cylinder(1, -0.5, 0.5, 0.05, 1, 4, 90, 0, 0, 0, 0, 0);
    glPopMatrix();
}
// To draw a support banner of given height and width using frame boxes
void drawSupportBanner(float height, float width, float boxUnit, int type)
{
    if (type == 1)
    {
        glPushMatrix();
        glTranslatef(0, height, width / 2 + 1);
        glScalef(0.16, 0.16, 0.16);
        drawTrafficLight();
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0, height, width / 2);
        glScalef(0.16, 0.16, 0.16);
        drawTrafficLight();
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0, height, width / 2 - 1);
        glScalef(0.16, 0.16, 0.16);
        drawTrafficLight();
        glPopMatrix();
    }

    if (type == 2)
    {
        glPushMatrix();
        glTranslatef(0, height + boxUnit * 1.5, width);
        glRotated(-8, 0, 1, 0);
        glRotated(10, 0, 0, 1);
        glScalef(0.3, 0.3, 0.3);
        drawCamera();
        glPopMatrix();
    }

    SetMaterial(0.3, 0.3, 0.3, 0.5, 0.5, 0.5, 0.3, 0.3, 0.3, 40);
    glColor3f(0.4, 0.4, 0.4);
    for (float i = 0; i < height; i += boxUnit)
    {
        glPushMatrix();
        glTranslatef(0, i, 0);
        glScalef(boxUnit, boxUnit, boxUnit);
        drawFrameBox();
        glPopMatrix();
    }
    for (float i = 0; i < height; i += boxUnit)
    {
        glPushMatrix();
        glTranslatef(0, i, width);
        glScalef(boxUnit, boxUnit, boxUnit);
        drawFrameBox();
        glPopMatrix();
    }
    // horizontal top
    for (float i = 0; i <= width; i += boxUnit)
    {
        glPushMatrix();
        glTranslatef(0, height, i);
        glScalef(boxUnit, boxUnit, boxUnit);
        drawFrameBox();
        glPopMatrix();
    }
}

void drawTrafficLight()
{
    // Traffic light box (dark metal/plastic housing)
    glPushMatrix();
    SetMaterial(0.1, 0.1, 0.1, // ambient (dark)
                0.1, 0.1, 0.1, // diffuse (dark)
                0.3, 0.3, 0.3, // specular (slightly shiny)
                10.0);         // shininess
    cube(0, 0, 0, 1, 3, 1, 0, 0, 0, 0);
    glPopMatrix();

    // Red light (top) - glowing red
    glPushMatrix();
    glTranslatef(-0.5, 1.5, 0);
    SetMaterial(0.3, 0.0, 0.0, // ambient (dark red)
                1.0, 0.0, 0.0, // diffuse (bright red)
                1.0, 0.5, 0.5, // specular (shiny red)
                100.0);        // shininess
    // Manually set emission for glow
    float redEmission[] = {0.8, 0.0, 0.0, 1.0};
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, redEmission);
    sphere(0, 0, 0, 0.8);
    glPopMatrix();

    // Yellow light (middle) - glowing yellow
    glPushMatrix();
    glTranslatef(-0.5, 0, 0);
    SetMaterial(0.3, 0.0, 0.0, // ambient (dark red)
                1.0, 0.0, 0.0, // diffuse (bright red)
                1.0, 0.5, 0.5, // specular (shiny red)
                100.0);        // shininess
    // Manually set emission for glow
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, redEmission);
    sphere(0, 0, 0, 0.8);
    glPopMatrix();

    // Green light (bottom) - glowing green
    glPushMatrix();
    glTranslatef(-0.5, -1.5, 0);
    SetMaterial(0.3, 0.0, 0.0, // ambient (dark red)
                1.0, 0.0, 0.0, // diffuse (bright red)
                1.0, 0.5, 0.5, // specular (shiny red)
                100.0);        // shininess
    // Manually set emission for glow
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, redEmission);
    sphere(0, 0, 0, 0.8);
    glPopMatrix();

    // Reset emission to zero for other objects
    float noEmission[] = {0.0, 0.0, 0.0, 1.0};
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, noEmission);
}

void drawCamera()
{

    // camera housing
    glPushMatrix();
    SetMaterial(0.15, 0.15, 0.15,
                0.3, 0.3, 0.3,
                0.6, 0.6, 0.6,
                60.0);
    cube(0, 0, 0, 0.9, 0.5, 0.6, 0, 0, 0, 0);
    glPopMatrix();

    glPushMatrix();
    SetMaterial(0.15, 0.15, 0.15,
                0.3, 0.3, 0.3,
                0.8, 0.7, 0.8,
                60.0);
    cube(0, 0, 0, 0.8, 0.6, 0.7, 0, 0, 0, 0);
    glPopMatrix();

    // silver connection between camera body and lens
    glPushMatrix();
    glTranslatef(-1, 0, 0);
    SetMaterial(0.3, 0.3, 0.3,
                0.5, 0.5, 0.5,
                0.9, 0.9, 0.9,
                100.0);
    cylinder(0, 0, 0, 0.2, 0.5, 20, 0, 0, 90, 0, 0, 0);
    glPopMatrix();

    // lens Cup
    glPushMatrix();
    glTranslatef(-1.6, 0, 0);
    SetMaterial(0.05, 0.05, 0.05,
                0.2, 0.2, 0.2,
                0.4, 0.4, 0.4,
                40.0);
    cylinder(0, 0, 0, 0.4, 0.6, 20, 0, 0, 0, 0, 0, 0);
    glPopMatrix();

    // camera handle
    glPushMatrix();
    glTranslatef(0.5, -0.5, 0);
    glRotated(90, 0, 0, 1);

    SetMaterial(0.3, 0.3, 0.3,
                0.5, 0.5, 0.5,
                0.9, 0.9, 0.9,
                100.0);
    cylinder(0, 0, 0, 0.15, 2, 20, 0, 0, 90, 0, 0, 0);
    glPopMatrix();

    // sphere lens
    glPushMatrix();
    glTranslatef(-1.75, 0, 0);
    SetMaterial(0.2, 0.25, 0.3,  // ambient (ice blue)
                0.3, 0.4, 0.5,   // diffuse (light sky blue)
                0.95, 0.98, 1.0, // specular (almost white highlights)
                128.0);          // shininess
    sphere(0, 0, 0, 0.28);
    glPopMatrix();

    // red light
    glPushMatrix();
    glTranslatef(-0.65, 0.65, 0.35);
    SetMaterial(0.3, 0.0, 0.0, // ambient (dark red)
                1.0, 0.0, 0.0, // diffuse (bright red)
                1.0, 0.5, 0.5, // specular (shiny red)
                100.0);        // shininess
    // Manually set emission for glow
    float redEmission[] = {0.8, 0.0, 0.0, 1.0};
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, redEmission);
    cube(0, 0, 0, 0.04, 0.04, 0.04, 0, 0, 0, 0);
    glPopMatrix();

    // Reset emission to zero for other objects
    float noEmission[] = {0.0, 0.0, 0.0, 1.0};
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, noEmission);
}