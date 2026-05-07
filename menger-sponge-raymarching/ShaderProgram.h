#pragma once

#include <string>

#include "Shader.h"

class ShaderProgram {
    GLuint shader_program;
    std::string last_error;

public:
    ShaderProgram();

    GLuint getProgramId() const;
    void attachShader(const Shader& shader);
    bool link();
    const std::string& getLastError() const;
};

