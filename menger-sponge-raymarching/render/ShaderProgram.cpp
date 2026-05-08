#include "ShaderProgram.h"

ShaderProgram::ShaderProgram() {
    shader_program = glCreateProgram();
}

GLuint ShaderProgram::getProgramId() const {
    return shader_program;
}

void ShaderProgram::attachShader(const Shader& shader) {
    shader.attach(shader_program);
}

bool ShaderProgram::link() {
    glLinkProgram(shader_program);

    GLint success;
    GLchar info_log[1024];
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program, sizeof info_log, nullptr, info_log);
        last_error = info_log;
        return false;
    }

    last_error.clear();
    return true;
}

const std::string& ShaderProgram::getLastError() const {
    return last_error;
}
