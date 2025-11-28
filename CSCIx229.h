#ifndef CSCIx229
#define CSCIx229

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

// GLEW _MUST_ be included first
#ifdef USEGLEW
#include <GL/glew.h>
#endif
//  Get all GL prototypes
#define GL_GLEXT_PROTOTYPES
//  Select SDL, SDL2, GLFW or GLUT
#if defined(SDL2)
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#elif defined(SDL)
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#elif defined(GLFW)
#include <GLFW/glfw3.h>
#elif defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
//  Make sure GLU and GL are included
#ifdef __APPLE__
#include <OpenGL/glu.h>
#include <OpenGL/gl.h>
// Tell Xcode IDE to not gripe about OpenGL deprecation
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#else
#include <GL/glu.h>
#include <GL/gl.h>
#endif

//  cos and sin in degrees
#define Cos(th) cos(3.14159265 / 180 * (th))
#define Sin(th) sin(3.14159265 / 180 * (th))

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __GNUC__
    void Print(const char *format, ...) __attribute__((format(printf, 1, 2)));
    void Fatal(const char *format, ...) __attribute__((format(printf, 1, 2))) __attribute__((noreturn));
#else
void Print(const char *format, ...);
void Fatal(const char *format, ...);
#endif
    unsigned int LoadTexBMP(const char *file);

    void Project(int perspective, double fov, double asp, double dim);

    void ErrCheck(const char *where);

    int LoadOBJ(const char *file);

    void SetMaterial(float ambient_r, float ambient_g, float ambient_b,
                     float diffuse_r, float diffuse_g, float diffuse_b,
                     float specular_r, float specular_g, float specular_b,
                     float shininess);
    // Shapes

    void cube(double x, double y, double z,
              double dx, double dy, double dz,
              double th,
              int useTexture,
              double texRepeatU,
              double texRepeatV);

    void drawTorus(double centerX, double centerY, double centerZ,
                   double majorRadius, double minorRadius,
                   int numMajor, int numMinor,
                   double startAngle, double endAngle);

    void noTexCube(double x, double y, double z,
                   double dx, double dy, double dz,
                   double th);

    void trapezoid(double x, double y, double z,
                   double dx, double dy, double dz,    // scale
                   double thX, double thY, double thZ, // rotations
                   double topWidthX, double topWidthZ,
                   double bottomWidthX, double bottomWidthZ,
                   double height);

    void cylinder(double x, double y, double z,
                  double radius, double height,
                  int slices,
                  double thX, double thY, double thZ,
                  int useTexture,
                  double texRepeatU, double texRepeatV);

    void rectangle(double x, double y, double z,     // center position
                   double w, double h,               // width and height
                   double rx, double ry, double rz); // rotations (deg) about x,y,z

    void rectangleTex(double x, double y, double z, double width, double height, double rx, double ry, double rz, unsigned int texture, int useTexture);

    void prism(double base, double height, double depth);

    void drawBarricade(double x, double y, double z, double rotation, unsigned int texture);

    // Complex Objs
    void drawF1Car(float length, float width, float breadth, unsigned int texture[], float colors[][3], float steeringAngle, int isBraking);

    void drawTireBarrierRow(double startX, double y, double z, int count, double spacing);

    void drawF1Garage(double x, double y, double z, double scale, unsigned int texture[], float colors[][3]);

    void drawTree(double x, double y, double z, double scale, unsigned int texture[]);

    void drawStand(double x, double y, double z, double width, double height, double depth, unsigned int texture[]);

    void ball(double x, double y, double z, double r);

    void sphere(double x, double y, double z, double r);

    void drawRoadBlockWithCurbs(double x, double y, double z, double width, double length, double rotation, unsigned int texture[], int curbs, unsigned int barricadeTextures[], int numBarricadeTextures);

    void drawRoadBlockRightTurn(double x, double y, double z, double innerRadius, double width, double rotation, double degreeTurn, unsigned int texture[], int curbs);

    void drawRoadBlockLeftTurn(double x, double y, double z, double innerRadius, double width, double rotation, double degreeTurn, unsigned int texture[], int curbs);

    void drawCircuit(unsigned int texture[], unsigned int barricadeTextures[], int numBarricadeTextures, float colors[][3]);

    void drawFrameBox();

    void drawSupportBanner(float height, float width, float boxUnit, int type);

    void squareBracketMarking();

    void drawTrafficLight();

    void drawCamera();

    // Shader functions
    void PrintShaderLog(int obj, char *file);
    void PrintProgramLog(int obj);
    char *ReadText(char *file);
    int CreateShader(GLenum type, char *file);
    int CreateShaderProg(char *VertFile, char *FragFile);

#ifdef __cplusplus
}
#endif

#endif
