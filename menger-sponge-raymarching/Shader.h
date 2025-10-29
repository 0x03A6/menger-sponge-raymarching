#pragma once
#include <iostream>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

class Shader {
    GLuint shader;
    GLenum shader_type;

public:

    // create and compile
    Shader(const char* const src, const GLenum shader_type_);

    // create
    Shader(const GLenum shader_type);

    void compileFile(const char* path);

    // set source code and compile
    void compile(const char* const src);

    // attach to program
    void attach(const GLuint shader_program);

    GLuint getShaderId();

    // delete
    ~Shader();
};