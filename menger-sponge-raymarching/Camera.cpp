#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

Camera::Camera()
    : pos(0.0f, 0.0f, 3.0f),           // 初始位置：在Z轴上
    sight(0.0f, 0.0f, -1.0f),        // 初始视线：看向-Z方向
    up(0.0f, 1.0f, 0.0f),            // 初始上方向：Y轴正方向
    right(1.0f, 0.0f, 0.0f)          // 初始右方向：X轴正方向
{
    // 确保向量是单位向量且正交
    sight = glm::normalize(sight);
    up = glm::normalize(up);
    right = glm::normalize(right);
}

void Camera::moveForward(float delta) {
    pos += sight * delta;
}

void Camera::moveRight(float delta) {
    pos += right * delta;
}

void Camera::moveUp(float delta) {
    pos += up * delta;
}

void Camera::rotateYaw(float angle) {
    // 绕上向量旋转视线和右向量
    sight = glm::rotate(sight, angle, up);
    right = glm::rotate(right, angle, up);

    // 重新正交化
    sight = glm::normalize(sight);
    right = glm::normalize(glm::cross(sight, up));
}

void Camera::rotatePitch(float angle) {
    // 绕右向量旋转视线和上向量
    sight = glm::rotate(sight, angle, right);
    up = glm::rotate(up, angle, right);

    // 重新正交化
    sight = glm::normalize(sight);
    up = glm::normalize(glm::cross(right, sight));
}

vec3 &Camera::getPos() {
    return pos;
}

glm::mat4 Camera::getInvViewMatrix() {
    // 构建从相机空间到世界空间的变换矩阵
    // 这个矩阵的列向量分别是右、上、前方向和位置

    glm::mat4 invView(1.0f);

    // 设置旋转部分（前三个列向量）
    invView[0] = glm::vec4(right, 0.0f);    // 第一列：右方向
    invView[1] = glm::vec4(up, 0.0f);       // 第二列：上方向  
    invView[2] = glm::vec4(-sight, 0.0f);   // 第三列：前方向（注意取负，因为相机看向-Z）

    // 设置平移部分（第四列）
    invView[3] = glm::vec4(pos, 1.0f);

    return invView;
}