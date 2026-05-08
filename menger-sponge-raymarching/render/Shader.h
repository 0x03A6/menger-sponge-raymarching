#pragma once

#include <string>
#include <unordered_map>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

class Shader {
    GLuint shader;
    GLenum shader_type;
    std::string last_error;

public:
    Shader(const char* const src, const GLenum shader_type_);
    Shader(const GLenum shader_type);

    bool compileFile(const char* path);
    bool compileFile(const char* path, const std::unordered_map<std::string, std::string>& replacements);
    bool compile(const char* const src);
    bool compile(const std::string& src);

    void attach(const GLuint shader_program) const;

    GLuint getShaderId() const;
    const std::string& getLastError() const;

    ~Shader();

private:
    static bool readFileText(const char* path, std::string& source, std::string& error_message);
    static void applyReplacements(std::string& source, const std::unordered_map<std::string, std::string>& replacements);
};
