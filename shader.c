#include "CSCIx229.h"

/*
 *  Print Shader Log
 */
void PrintShaderLog(int obj, char *file)
{
    int len = 0;
    glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &len);
    if (len > 1)
    {
        int n = 0;
        char *buffer = (char *)malloc(len);
        if (!buffer)
            Fatal("Cannot allocate %d bytes of text for shader log\n", len);
        glGetShaderInfoLog(obj, len, &n, buffer);
        fprintf(stderr, "%s:\n%s\n", file, buffer);
        free(buffer);
    }
    glGetShaderiv(obj, GL_COMPILE_STATUS, &len);
    if (!len)
        Fatal("Error compiling %s\n", file);
}

/*
 *  Print Program Log
 */
void PrintProgramLog(int obj)
{
    int len = 0;
    glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &len);
    if (len > 1)
    {
        int n = 0;
        char *buffer = (char *)malloc(len);
        if (!buffer)
            Fatal("Cannot allocate %d bytes of text for program log\n", len);
        glGetProgramInfoLog(obj, len, &n, buffer);
        fprintf(stderr, "%s\n", buffer);
        free(buffer);
    }
    glGetProgramiv(obj, GL_LINK_STATUS, &len);
    if (!len)
        Fatal("Error linking program\n");
}

/*
 *  Read text file
 */
char *ReadText(char *file)
{
    char *buffer;
    int n;
    FILE *f = fopen(file, "rt");
    if (!f)
        Fatal("Cannot open text file %s\n", file);
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    rewind(f);
    buffer = (char *)malloc(n + 1);
    if (!buffer)
        Fatal("Cannot allocate %d bytes for text file %s\n", n, file);
    if ((int)fread(buffer, 1, n, f) != n)
        Fatal("Cannot read %d bytes for text file %s\n", n, file);
    buffer[n] = 0;
    fclose(f);
    return buffer;
}

/*
 *  Create Shader
 */
int CreateShader(GLenum type, char *file)
{
    int shader = glCreateShader(type);
    char *source = ReadText(file);
    glShaderSource(shader, 1, (const char **)&source, NULL);
    free(source);
    glCompileShader(shader);
    PrintShaderLog(shader, file);
    return shader;
}

/*
 *  Create Shader Program
 */
int CreateShaderProg(char *VertFile, char *FragFile)
{
    int prog = glCreateProgram();
    int vert = CreateShader(GL_VERTEX_SHADER, VertFile);
    int frag = CreateShader(GL_FRAGMENT_SHADER, FragFile);

    glAttachShader(prog, vert);
    glAttachShader(prog, frag);

    // Bind vec4 rainData to attribute 0 BEFORE linking
    glBindAttribLocation(prog, 0, "rainData");

    glLinkProgram(prog);

    glDeleteShader(vert);
    glDeleteShader(frag);
    return prog;
}
