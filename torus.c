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
