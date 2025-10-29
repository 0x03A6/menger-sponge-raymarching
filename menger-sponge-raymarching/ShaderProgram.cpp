#include "ShaderProgram.h"

ShaderProgram::ShaderProgram() {
    shader_program = glCreateProgram();
}

GLuint ShaderProgram::getProgramId() {
    return shader_program;
}

void ShaderProgram::attachShader(Shader shader) {
    shader.attach(shader_program);
}

void ShaderProgram::link() {
    glLinkProgram(shader_program);
    GLint success;
    GLchar info_log[512];
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program, sizeof info_log, NULL, info_log);
        std::cout << "shader program linking failed.\n" << info_log << std::endl;
    }
}
