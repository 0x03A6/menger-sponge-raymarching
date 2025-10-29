#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>

using namespace glm;

class Camera {
	vec3 pos;
	vec3 sight;
	vec3 up;
	vec3 right;
public:
	Camera();
	void moveForward(float delta);
	void moveRight(float delta);
	void moveUp(float delta);
	void rotateYaw(float angle);
	void rotatePitch(float angle);
	vec3 getPos() const;
	mat4 getInvViewMatrix();
};
