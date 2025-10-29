#define _CRT_SECURE_NO_WARNINGS
#define GLM_ENABLE_EXPERIMENTAL

#include <iostream>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

#include <glm/gtc/type_ptr.hpp>

#include "ShaderProgram.h"
#include "Camera.h"
#include "sponge.h"

constexpr int WINDOW_WIDTH = 3200;
constexpr int WINDOW_HEIGHT = 1800;
constexpr double WINDOW_ASPECT_RATIO = static_cast<double>(WINDOW_WIDTH) / WINDOW_HEIGHT;

GLFWwindow* initOpenGL();

GLuint initTriangle(GLuint* VAO);

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(float deltaTime);

Camera camera;

// 全局变量存储按键状态
bool keys[1024];
float lastX = 500, lastY = 500; // 初始化为窗口中心
bool firstMouse = true;
bool mouseCaptured = true; // 鼠标是否被捕获

int main() {
    GLFWwindow* window = initOpenGL();

    GLuint VAO;
    GLuint shader_program = initTriangle(&VAO);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);

    glUseProgram(shader_program);

    // 初始化按键状态数组
    for (int i = 0; i < 1024; i++)
        keys[i] = false;

    // 设置初始鼠标位置到中心
    glfwSetCursorPos(window, lastX, lastY);

    initFractal();

    // 获取blocks uniform的位置
    GLint blocksLoc = glGetUniformLocation(shader_program, "blocks");

    unsigned char frame_count = 0;
    for (double last_t = glfwGetTime(); !glfwWindowShouldClose(window);) {
        double t = glfwGetTime();
        GLint viewMatInv = glGetUniformLocation(shader_program, "view_inv");
        processInput(t - last_t);
        frame_count++;
        if (frame_count == 0)
			std::cout << "FPS: " << 1.0 / (t - last_t) << std::endl;
        last_t = t;
        // 每帧将鼠标移动到屏幕中心
        if (mouseCaptured) {
            glfwSetCursorPos(window, 500, 500);
            lastX = 500;
            lastY = 500;
        }

        glClear(GL_COLOR_BUFFER_BIT);

        glUniformMatrix4fv(viewMatInv, 1, GL_FALSE, glm::value_ptr(camera.getInvViewMatrix()));

        // 传递blocks数组到GPU
        ivec3 blockArray[BLOCK_AMOUNT];
        auto it = --blocks.end();
        for (int i = 0; i < 16; i++) {
            blockArray[i] = ivec3(*it);
            --it;
        }
        glUniform3iv(blocksLoc, BLOCK_AMOUNT_GPU, glm::value_ptr(blockArray[0]));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glfwPollEvents();
        glfwSwapBuffers(window);

        updateFractal(camera.getPos());
    }

    glfwTerminate();

    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    // ESC键退出
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // Alt键切换鼠标捕获状态
    if ((key == GLFW_KEY_LEFT_ALT || key == GLFW_KEY_RIGHT_ALT)) {
        if (action == GLFW_PRESS)
            mouseCaptured = false;
		else if (action == GLFW_RELEASE)
			mouseCaptured = true;
        if (mouseCaptured) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            // 重置鼠标位置到中心
            glfwSetCursorPos(window, 500, 500);
            lastX = 500;
            lastY = 500;
            firstMouse = true;
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

    // 记录按键状态（按下/释放）
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}

// 鼠标回调函数
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!mouseCaptured) return; // 如果鼠标未被捕获，不处理鼠标移动

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
        return;
    }

    float xoffset = lastX - xpos; // 反转X坐标
    float yoffset = lastY - ypos; // 反转Y坐标

    // 重置鼠标位置到中心后，偏移量可能会很大，需要限制
    if (abs(xoffset) > 100 || abs(yoffset) > 100) {
        lastX = xpos;
        lastY = ypos;
        return;
    }

    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    camera.rotateYaw(glm::radians(xoffset));
    camera.rotatePitch(glm::radians(yoffset));
}

// 处理连续按键输入的函数（在渲染循环中调用）
void processInput(float deltaTime) {
    float moveSpeed = 1.0f * deltaTime * sdMenger(camera.getPos()); // 调整移动速度

    // 地平面移动（WASD）
    if (keys[GLFW_KEY_W])
        camera.moveForward(moveSpeed);
    if (keys[GLFW_KEY_S])
        camera.moveForward(-moveSpeed);
    if (keys[GLFW_KEY_A])
        camera.moveRight(-moveSpeed);
    if (keys[GLFW_KEY_D])
        camera.moveRight(moveSpeed);

    // 垂直移动（SPACE向上，LCTRL向下）
    if (keys[GLFW_KEY_SPACE])
        camera.moveUp(moveSpeed);
    if (keys[GLFW_KEY_LEFT_CONTROL])
        camera.moveUp(-moveSpeed);
}

GLFWwindow* initOpenGL() {

    //initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // create window
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Menger Sponge", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);

    // initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return nullptr;
    }

    // set viewport
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // set callback functions
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    // 初始时捕获鼠标
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    return window;
}

constexpr GLfloat vertices[] = {
    -1.0,  1.0, // LU
    -1.0, -1.0, // LD
     1.0, -1.0, // RD
     1.0,  1.0  // RU
};

constexpr GLuint indices[] = {
    0, 1, 2,
    2, 3, 0
};

GLuint initTriangle(GLuint* VAO) {
    // init VAO
    glGenVertexArrays(1, VAO);
    glBindVertexArray(*VAO);

    // init VBO
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);

    GLuint EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices, indices, GL_STATIC_DRAW);

    ShaderProgram shader_program;

    Shader vertex_shader(GL_VERTEX_SHADER);
    vertex_shader.compileFile("./shader.vert");
    shader_program.attachShader(vertex_shader);

    Shader fragment_shader(GL_FRAGMENT_SHADER);
    fragment_shader.compileFile("./shader.frag");
    shader_program.attachShader(fragment_shader);

    shader_program.link();

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    return shader_program.getProgramId();
}