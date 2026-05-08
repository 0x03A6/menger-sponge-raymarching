#include "Shader.h"

#include <fstream>
#include <sstream>

Shader::Shader(const char* const src, const GLenum shader_type_) : shader_type(shader_type_) {
    shader = glCreateShader(shader_type_);
    compile(src);
}

Shader::Shader(const GLenum shader_type_) : shader_type(shader_type_) {
    shader = glCreateShader(shader_type_);
}

bool Shader::compileFile(const char* path) {
    const std::unordered_map<std::string, std::string> replacements;
    return compileFile(path, replacements);
}

bool Shader::compileFile(const char* path, const std::unordered_map<std::string, std::string>& replacements) {
    std::string source;
    if (!readFileText(path, source, last_error))
        return false;

    applyReplacements(source, replacements);
    return compile(source);
}

bool Shader::compile(const char* const src) {
    return compile(std::string(src));
}

bool Shader::compile(const std::string& src) {
    const char* source = src.c_str();
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    GLchar info_log[1024];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, sizeof info_log, nullptr, info_log);
        last_error = info_log;
        return false;
    }

    last_error.clear();
    return true;
}

void Shader::attach(const GLuint shader_program) const {
    glAttachShader(shader_program, shader);
}

GLuint Shader::getShaderId() const {
    return shader;
}

const std::string& Shader::getLastError() const {
    return last_error;
}

Shader::~Shader() {
    glDeleteShader(shader);
}

bool Shader::readFileText(const char* path, std::string& source, std::string& error_message) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        error_message = std::string("failed to open shader file: ") + path;
        return false;
    }

    std::ostringstream stream;
    stream << file.rdbuf();
    source = stream.str();
    error_message.clear();
    return true;
}

void Shader::applyReplacements(std::string& source, const std::unordered_map<std::string, std::string>& replacements) {
    for (const auto& replacement : replacements) {
        std::size_t position = 0;
        while ((position = source.find(replacement.first, position)) != std::string::npos) {
            source.replace(position, replacement.first.size(), replacement.second);
            position += replacement.second.size();
        }
    }
}
