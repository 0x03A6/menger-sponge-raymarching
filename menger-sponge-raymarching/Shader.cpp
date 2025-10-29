#define _CRT_SECURE_NO_WARNINGS

#include "Shader.h"

// create and compile
Shader::Shader(const char* const src, const GLenum shader_type_) :shader_type(shader_type_) {
    shader = glCreateShader(shader_type_);
    compile(src);
}

Shader::Shader(const GLenum shader_type_) :shader_type(shader_type_) {
    shader = glCreateShader(shader_type_);
}

void Shader::compileFile(const char* path) {
    FILE* src_file = fopen(path, "r");

    // 获取文件大小（字节）
    fseek(src_file, 0, SEEK_END);
    const unsigned int src_size = ftell(src_file);
    rewind(src_file);

    char* src = new char[(size_t)src_size + 1]; // plus one for \0
    fread(src, 1, src_size, src_file);
    src[src_size] = '\0';
    compile(src);

    fclose(src_file);
}

void Shader::compile(const char* const src) {
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    GLint success;
    GLchar info_log[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, sizeof info_log, NULL, info_log);
        std::cout << "shader complication failed.\n" << info_log << std::endl;
    }
}

// attach to program
void Shader::attach(const GLuint shader_program) {
    glAttachShader(shader_program, shader);
}

GLuint Shader::getShaderId() {
    return shader;
}

// delete
Shader::~Shader() {
    glDeleteShader(shader);
}