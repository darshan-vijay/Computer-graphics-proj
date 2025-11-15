
#include "CSCIx229.h"

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