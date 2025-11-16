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
void drawF1Car(float length, float width, float breadth, unsigned int texture[], float colors[][3], float steeringAngle, int isBraking)
{
    // colors array structure:
    // colors[0] = body color (main chassis)
    // colors[1] = fin/wing color (rear and front wings)
    // colors[2] = halo/reinforcement bar color
    // Derived colors (automatically calculated):
    // - Suspension: darker metallic gray
    // - Wheels: black rubber
    // - Cockpit: slightly darker body color with transparency effect
    // New parameters:
    // steeringAngle: angle to rotate front wheels (positive = right, negative = left)
    // isBraking: 1 if braking, 0 otherwise (for brake light)

    // Scaling factors
    float scaleX = length;  // Length scaling (front to back)
    float scaleY = breadth; // Height scaling (vertical)
    float scaleZ = width;   // Width scaling (side to side)

    glPushMatrix();
    glTranslated(0, 0.35 * scaleY, 0);
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
              0.35, 0.6,
              1.0);
    glPopMatrix();

    // --- Rear cube end
    glPushMatrix();
    glTranslated(-4.3, 0, 0);
    noTexCube(0, 0, 0,
              0.6, 0.35, 0.6,
              0);
    glPopMatrix();

    // === REAR WINGS AND CONNECTORS ===
    // Using fin/wing color
    SetMaterial(colors[1][0], colors[1][1], colors[1][2],
                colors[1][0] * 1.2, colors[1][1] * 1.2, colors[1][2] * 1.2,
                0.8, 0.8, 0.8, 80);

    rectangle(-4.4, 1.08, 0, 0.45, 1.6, -90, 0, 0); // back fin
    rectangle(-4.4, 1.13, 0.8, 0.45, 0.1, 0, 0, 0);
    rectangle(-4.4, 1.13, -0.8, 0.45, 0.1, 0, 0, 0);
    rectangle(-4.4, 0.16, -0.39, 0.45, 1, 0, 0, 0);
    rectangle(-4.4, 0.16, 0.39, 0.45, 1, 0, 0, 0);
    rectangle(-4.4, 0.85, -0.6, 0.45, 0.62, -45, 0, 0);
    rectangle(-4.4, 0.85, 0.6, 0.45, 0.62, 45, 0, 0);

    // === BRAKE LIGHTS ===
    if (isBraking)
    {
        // Bright red glowing brake lights when braking
        SetMaterial(1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.3, 0.3, 50);
        glDisable(GL_LIGHTING); // Make brake lights emit light
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

    // === FRONT WING ===
    SetMaterial(colors[1][0], colors[1][1], colors[1][2],
                colors[1][0] * 1.2, colors[1][1] * 1.2, colors[1][2] * 1.2,
                0.8, 0.8, 0.8, 80);
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

    // top handle on bezier

    cylinder(-2.2, 0.88, 0, 0.02, 0.38, 8, 0, 0, 0, 0, 0, 0);

    cylinder(-2.2, 1.06, 0, 0.02, 0.2, 8, 90, 0, 0, 0, 0, 0);

    // === WHEELS ===
    // Black rubber for tires
    SetMaterial(0.01, 0.01, 0.01, 0.05, 0.05, 0.05, 0.1, 0.1, 0.1, 5);

    // Front wheels with steering rotation
    glPushMatrix();
    glTranslated(1, 0, -1.35);
    glRotated(-steeringAngle, 0, 1, 0); // Rotate around Y axis for steering
    glTranslated(-1, 0, 1.2);
    cylinder(1, 0, -1.35, 0.6, 0.6, 20, 90, 0, 0, 0, 0, 0);
    glPopMatrix();

    glPushMatrix();
    glTranslated(1, 0, 1.35);
    glRotated(-steeringAngle, 0, 1, 0); // Rotate around Y axis for steering
    glTranslated(-1, 0, -1.2);
    cylinder(1, 0, 1.35, 0.6, 0.6, 20, 90, 0, 0, 0, 0, 0);
    glPopMatrix();

    // Rear wheels - no steering
    cylinder(-4, 0, -1.2, 0.6, 0.6, 20, 90, 0, 0, 0, 0, 0);
    cylinder(-4, 0, 1.2, 0.6, 0.6, 20, 90, 0, 0, 0, 0, 0);

    // === COCKPIT ===
    // Slightly darker body color for cockpit (more matte finish)
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
void drawF1Garage(double x, double y, double z, double scale, unsigned int texture[], float colors[][3])
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
    glTexCoord2f(8, 0);
    glVertex3f(8, 0, -6);
    glTexCoord2f(8, 6);
    glVertex3f(8, 0, 6);
    glTexCoord2f(0, 6);
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
    glTexCoord2f(8, 0);
    glVertex3f(8, 0, -6);
    glTexCoord2f(8, 3);
    glVertex3f(8, 6, -6);
    glTexCoord2f(0, 3);
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
    glTexCoord2f(6, 0);
    glVertex3f(-8, 0, 6);
    glTexCoord2f(6, 3);
    glVertex3f(-8, 6, 6);
    glTexCoord2f(0, 3);
    glVertex3f(-8, 6, -6);
    glEnd();
    // Right wall (X = +8)
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

    // === TOOL CABINETS (Left side, back) ===
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

    // Cabinet handles
    SetMaterial(0.5, 0.5, 0.55, 0.8, 0.8, 0.85, 1.0, 1.0, 1.0, 100);
    for (int i = 0; i < 5; i++)
    {
        cylinder(-4.7, 0.4 + i * 0.45, -4.5, 0.02, 0.25, 6, 90, 0, 0, 0, 0, 0);
    }

    // === WORKBENCH (Right side, back) ===
    SetMaterial(0.4, 0.35, 0.3, 0.6, 0.5, 0.45, 0.1, 0.1, 0.1, 10);
    noTexCube(6, 1.0, -4.5, 1.2, 0.08, 1.5, 0);

    // Workbench side panels
    SetMaterial(0.35, 0.3, 0.25, 0.55, 0.45, 0.4, 0.1, 0.1, 0.1, 10);
    noTexCube(4.9, 0.55, -4.5, 0.08, 0.55, 1.4, 0);
    noTexCube(7.1, 0.55, -4.5, 0.08, 0.55, 1.4, 0);

    // Workbench back panel
    noTexCube(6, 0.55, -5.6, 1.1, 0.55, 0.08, 0);

    // Workbench legs
    SetMaterial(0.2, 0.2, 0.22, 0.4, 0.4, 0.42, 0.3, 0.3, 0.3, 30);
    cylinder(5.0, 0.5, -5.3, 0.05, 1.0, 6, 0, 0, 0, 0, 0, 0);
    cylinder(7.0, 0.5, -5.3, 0.05, 1.0, 6, 0, 0, 0, 0, 0, 0);
    cylinder(5.0, 0.5, -3.7, 0.05, 1.0, 6, 0, 0, 0, 0, 0, 0);
    cylinder(7.0, 0.5, -3.7, 0.05, 1.0, 6, 0, 0, 0, 0, 0, 0);

    // === TIRE RACK (Right side, middle) ===
    SetMaterial(0.2, 0.2, 0.22, 0.4, 0.4, 0.42, 0.3, 0.3, 0.3, 30);
    // Rack frame (vertical posts)
    cylinder(6.5, 2, 0.5, 0.05, 4, 8, 0, 0, 0, 0, 0, 0);
    cylinder(7.5, 2, 0.5, 0.05, 4, 8, 0, 0, 0, 0, 0, 0);
    // Horizontal bars
    cylinder(7, 0.8, 0.5, 0.05, 1, 8, 90, 0, 0, 0, 0, 0);
    cylinder(7, 1.6, 0.5, 0.05, 1, 8, 90, 0, 0, 0, 0, 0);
    cylinder(7, 2.4, 0.5, 0.05, 1, 8, 90, 0, 0, 0, 0, 0);
    cylinder(7, 3.2, 0.5, 0.05, 1, 8, 90, 0, 0, 0, 0, 0);

    // Tires on rack
    SetMaterial(0.01, 0.01, 0.01, 0.05, 0.05, 0.05, 0.1, 0.1, 0.1, 5);
    for (int i = 0; i < 4; i++)
    {
        cylinder(7, 0.8 + i * 0.8, 0.5, 0.35, 0.25, 16, 90, 0, 0, 0, 0, 0);
    }

    // === COMPUTER/TELEMETRY STATION (Left wall, middle) ===
    glPushMatrix();
    glTranslated(-7.2, 0, 2);
    glRotated(-90, 0, 1, 0);

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

    // Screen (blue glow)
    SetMaterial(0.1, 0.3, 0.6, 0.2, 0.5, 1.0, 0.5, 0.7, 1.0, 60);
    noTexCube(0, 1.3, 0.05, 0.5, 0.35, 0.01, 0);

    // Monitor stand
    SetMaterial(0.3, 0.3, 0.32, 0.5, 0.5, 0.52, 0.4, 0.4, 0.4, 50);
    noTexCube(0, 0.9, 0, 0.12, 0.06, 0.12, 0);
    cylinder(0, 0.95, 0, 0.025, 0.28, 6, 0, 0, 0, 0, 0, 0);

    // Keyboard
    SetMaterial(0.1, 0.1, 0.12, 0.2, 0.2, 0.22, 0.1, 0.1, 0.1, 20);
    noTexCube(0, 0.88, 0.45, 0.4, 0.025, 0.18, 0);

    // Mouse
    SetMaterial(0.15, 0.15, 0.17, 0.25, 0.25, 0.27, 0.3, 0.3, 0.3, 40);
    noTexCube(0.7, 0.88, 0.25, 0.07, 0.02, 0.1, 0);

    glPopMatrix();

    // === TEAM LOGO/POSTER (Back wall, centered) ===
    // Poster background (team color)
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

    // === PIT BOARD (Against left wall) ===
    SetMaterial(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.2, 0.2, 0.2, 10);
    glPushMatrix();
    glTranslated(-7.7, 1.0, -1);
    glRotated(15, 0, 0, 1);
    noTexCube(0, 0, 0, 0.03, 0.7, 0.5, 0);
    glPopMatrix();

    // Pit board handle
    SetMaterial(0.3, 0.3, 0.3, 0.5, 0.5, 0.5, 0.2, 0.2, 0.2, 20);
    cylinder(-7.7, 0.3, -1, 0.02, 0.7, 6, 0, 0, 0, 0, 0, 0);

    // === TOOL BOARD/PEGBOARD (Right wall) ===
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

    // === CABLE REELS (Right side, front) ===
    SetMaterial(1.0, 0.6, 0.0, 1.0, 0.7, 0.1, 0.3, 0.3, 0.3, 30);
    cylinder(6.5, 0.15, 4.5, 0.25, 0.1, 12, 90, 0, 0, 0, 0, 0);
    SetMaterial(0.1, 0.1, 0.1, 0.2, 0.2, 0.2, 0.1, 0.1, 0.1, 10);
    cylinder(6.5, 0.15, 4.5, 0.05, 0.12, 10, 90, 0, 0, 0, 0, 0);

    // === AIR COMPRESSOR (Back right corner) ===
    SetMaterial(0.9, 0.5, 0.1, 1.0, 0.6, 0.2, 0.4, 0.4, 0.4, 40);
    noTexCube(5.5, 0.4, -2, 0.4, 0.4, 0.5, 0);
    // Compressor tank
    cylinder(5.5, 0.8, -2, 0.25, 0.6, 12, 0, 0, 0, 0, 0, 0);
    // Pressure gauge
    SetMaterial(0.8, 0.8, 0.8, 1.0, 1.0, 1.0, 0.6, 0.6, 0.6, 70);
    cylinder(5.5, 0.5, -2, 0.06, 0.025, 10, 90, 0, 0, 0, 0, 0);

    // === JACK STANDS (near car position) ===
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

    // === FLOOR MARKINGS (safety lines) ===
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

    // === THE F1 CAR (centered, facing front/opening) ===
    glPushMatrix();
    glTranslated(0, 0.35, 0);
    glRotated(-90, 0, 1, 0); // Rotate to face front opening
    glDisable(GL_COLOR_MATERIAL);
    drawF1Car(1, 1, 1, texture, colors, 0, 0);
    glEnable(GL_COLOR_MATERIAL);
    glPopMatrix();

    glPopMatrix();
} // Draw tire barrier (stack of colored tires)
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
void drawPitComplex(unsigned int texture[], float colors[][3])
{

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
    drawRoad(0, 0.02, 40, 4, 80, 0, texture);

    // === PIT LANE (LAYER 2 - y = 0.02) ===
    drawPitLane(-6, 0.02, 40, 4, 75, texture);

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
        drawF1Garage(0, 0, 0, 0.8, texture, colors);
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
void drawRoad(double x, double y, double z, double width, double length, double rotation, unsigned int texture[])
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

// Draw pit lane with markings
void drawPitLane(double x, double y, double z, double width, double length, unsigned int texture[])
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

// Draw a road block with red and white curbs on edges and barricades
void drawRoadBlockWithCurbs(double x, double y, double z, double width, double length, double rotation, unsigned int texture[], int curbs, unsigned int barricadeTextures[], int numBarricadeTextures)
{
    glPushMatrix();
    glTranslated(x, y, z);
    glRotated(rotation, 0, 1, 0);

    // Main road surface
    SetMaterial(0.4, 0.4, 0.4, 0.7, 0.7, 0.7, 0.2, 0.2, 0.2, 10);
    glColor3f(1.0, 1.0, 1.0); // White color to show texture properly

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
        double barricadeOffset = width / 2 + 0.5; // Position barricades 0.5 units outside the road edge

        for (double zPos = -length / 2 + 0.5; zPos < length / 2; zPos += 1.0)
        {
            int texIndex = (barricadeCount / 5) % numBarricadeTextures;

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
    double angleStep = degreeTurn / segments; // 90 degree turn

    // Main road surface - BRIGHTER material for better texture visibility
    SetMaterial(0.4, 0.4, 0.4, 0.7, 0.7, 0.7, 0.2, 0.2, 0.2, 10);
    glColor3f(1.0, 1.0, 1.0); // White color to show texture properly

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

void drawRoadBlockLeftTurn(double x, double y, double z, double innerRadius, double width, double rotation, double degreeTurn, unsigned int texture[], int curbs)
{
    glPushMatrix();
    glTranslated(x, y, z);
    glRotated(rotation, 0, 1, 0);

    double outerRadius = innerRadius + width;
    int segments = 16; // Number of segments for smooth curve
    double angleStep = degreeTurn / segments;

    // Main road surface - BRIGHTER material for better texture visibility
    SetMaterial(0.4, 0.4, 0.4, 0.7, 0.7, 0.7, 0.2, 0.2, 0.2, 10);
    glColor3f(1.0, 1.0, 1.0); // White color to show texture properly

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[0]);

    for (int i = 0; i < segments; i++)
    {
        double angle1 = i * angleStep * M_PI / 180.0;
        double angle2 = (i + 1) * angleStep * M_PI / 180.0;

        glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);
        glTexCoord2f(0, (float)i / segments);
        glVertex3f(innerRadius * cos(angle1), 0, -innerRadius * sin(angle1)); // Negated Z
        glTexCoord2f(1, (float)i / segments);
        glVertex3f(outerRadius * cos(angle1), 0, -outerRadius * sin(angle1)); // Negated Z
        glTexCoord2f(1, (float)(i + 1) / segments);
        glVertex3f(outerRadius * cos(angle2), 0, -outerRadius * sin(angle2)); // Negated Z
        glTexCoord2f(0, (float)(i + 1) / segments);
        glVertex3f(innerRadius * cos(angle2), 0, -innerRadius * sin(angle2)); // Negated Z
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

void drawCircuit(unsigned int roadTexture[], unsigned int barricadeTextures[], int numBarricadeTextures)
{
    // Draw light grey ground rectangle
    SetMaterial(0.5, 0.5, 0.5, 0.6, 0.6, 0.6, 0.2, 0.2, 0.2, 10);
    glColor3f(0.6, 0.6, 0.6);

    glPushMatrix();
    glTranslated(0, -0.01, 0); // Slightly below road level
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glVertex3f(-100, 0, -100);
    glVertex3f(100, 0, -100);
    glVertex3f(100, 0, 100);
    glVertex3f(-100, 0, 100);
    glEnd();
    glPopMatrix();

    glPushMatrix();
    glTranslated(20, 0, 0);
    // Race start/finish straight
    drawRoadBlockWithCurbs(0, 0, 0, 4, 35, 90, roadTexture, 1, barricadeTextures, 3);

    // 1st right turn
    drawRoadBlockRightTurn(17.5, 0, 7, 5, 4, 90, 90, roadTexture, 1);

    // Right straight
    drawRoadBlockWithCurbs(24.5, 0, 24, 4, 35, 0, roadTexture, 1, barricadeTextures, 3);

    // 2nd right turn, 130 degrees
    glPushMatrix();
    glTranslated(17.5, 0, 41.5);
    glRotated(-60, 0, 1, 0);
    drawRoadBlockRightTurn(0, 0, 0, 5, 4, 60, 130, roadTexture, 1);
    glPopMatrix();

    // Angled straight
    drawRoadBlockWithCurbs(0, 0, 35.9, 4, 35, 50, roadTexture, 1, barricadeTextures, 3);

    glPopMatrix();

    // angled end
    drawRoadBlockWithCurbs(4.5, 0, 23, 4, 18, 50, roadTexture, 0, barricadeTextures, 3);

    glPushMatrix();
    glTranslated(2.5, 0, 7);
    glRotated(180, 0, 1, 0);
    drawRoadBlockRightTurn(0, 0, 0, 5, 4, 0, 90, roadTexture, 1);
    glPopMatrix();

    drawRoadBlockWithCurbs(-4.5, 0, 9.8, 4, 6, 0, roadTexture, 1, barricadeTextures, 3);

    glPushMatrix();
    glTranslated(2.5, 0, 12.2);
    drawRoadBlockLeftTurn(0, 0, 0, 5, 4, 180, 50, roadTexture, 0);
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
    // Light grey shiny material for posts
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

    // Sign/board with texture - higher diffuse
    SetMaterial(0.5, 0.5, 0.5, 0.9, 0.9, 0.9, 0.3, 0.3, 0.3, 20);
    glPushMatrix();
    glTranslated(0, -0.75, 1.5);
    rectangleTex(0, 0, 0, 3, 1.5, 0, 90, 0, texture, 1);
    glPopMatrix();

    // Wires between posts - light grey shiny material
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