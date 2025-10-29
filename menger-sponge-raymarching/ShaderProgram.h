#pragma once

#include "Shader.h"

class ShaderProgram {
	GLuint shader_program;

public:

	ShaderProgram();

	GLuint getProgramId();

	void attachShader(Shader shader);

	void link();
};

