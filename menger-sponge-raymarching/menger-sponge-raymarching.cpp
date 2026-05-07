#define _CRT_SECURE_NO_WARNINGS
#define GLM_ENABLE_EXPERIMENTAL
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <Windows.h>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <glm/gtc/type_ptr.hpp>

#include "ParameterConsole.h"
#include "ParameterRegistry.h"
#include "RenderConfig.h"
#include "ShaderProgram.h"
#include "Camera.h"
#include "sponge.h"

constexpr int DEFAULT_WINDOW_WIDTH = 3200;
constexpr int DEFAULT_WINDOW_HEIGHT = 1800;
constexpr double DEFAULT_WINDOW_ASPECT_RATIO = static_cast<double>(DEFAULT_WINDOW_WIDTH) / DEFAULT_WINDOW_HEIGHT;
constexpr double INITIAL_WINDOW_SCALE = 0.8;
constexpr const char* DEFAULT_STARTUP_CONFIG_PATH = "config.txt";

struct WindowSize {
    int width;
    int height;
};

struct RaymarchProgram {
    GLuint program_id = 0;
    GLint blocks_loc = -1;
    GLint view_mat_inv_loc = -1;
    GLint aspect_ratio_loc = -1;
};

GLFWwindow* initOpenGL();

void initFullscreenQuad(GLuint* vao);
WindowSize getInitialWindowSize();
void syncRuntimeConfig(int width, int height);
std::unordered_map<std::string, std::string> getShaderSourceReplacements(const AppConfig& app_config);
bool buildRaymarchProgram(const AppConfig& app_config, RaymarchProgram* raymarch_program, std::string& error_message);
bool reloadRaymarchProgram(const AppConfig& app_config, RaymarchProgram* raymarch_program, std::string& error_message);
void destroyRaymarchProgram(RaymarchProgram* raymarch_program);
bool isUnsignedIntegerString(const char* text);
bool loadStartupConfigScript(
    const char* config_path,
    bool config_path_is_required,
    RaymarchProgram* raymarch_program,
    std::string& error_message
);
void clearKeyStates();
void runConsoleSession(GLFWwindow* window, RaymarchProgram* raymarch_program);
HWND getGlfwWindowHandle(GLFWwindow* window);
void focusConsoleWindow();
void sendRenderWindowToBack(GLFWwindow* window);
void bringRenderWindowToFront(GLFWwindow* window);
void setMouseCaptureState(GLFWwindow* window, bool should_capture);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(double delta_time);
void recenterCursor(GLFWwindow* window);

Camera camera;
const ParameterRegistry parameter_registry;
ConfigState config_state = makeDefaultConfigState(static_cast<float>(DEFAULT_WINDOW_ASPECT_RATIO));

// 全局变量存储按键状态
bool keys[1024];
float lastX = DEFAULT_WINDOW_WIDTH * 0.5f, lastY = DEFAULT_WINDOW_HEIGHT * 0.5f;
bool firstMouse = true;
bool mouseCaptured = true; // 鼠标是否被捕获
bool console_requested = false;

int main(int argc, char *argv[]) {
    GLFWwindow* window = initOpenGL();
    if (window == nullptr)
        return 1;

    GLuint vao;
    initFullscreenQuad(&vao);

    RaymarchProgram raymarch_program;
    std::string error_message;
    if (!reloadRaymarchProgram(config_state.active_config, &raymarch_program, error_message)) {
        std::cout << error_message << std::endl;
        glfwTerminate();
        return 1;
    }

    const char* startup_config_path = DEFAULT_STARTUP_CONFIG_PATH;
    bool startup_config_path_is_required = false;
    int seed_argument_index = 1;
    if (argc >= 2) {
        if (isUnsignedIntegerString(argv[1])) {
            seed_argument_index = 1;
        }
        else {
            startup_config_path = argv[1];
            startup_config_path_is_required = true;
            seed_argument_index = 2;
        }
    }

    if (!loadStartupConfigScript(
        startup_config_path,
        startup_config_path_is_required,
        &raymarch_program,
        error_message
    )) {
        std::cout << error_message << std::endl;
        destroyRaymarchProgram(&raymarch_program);
        glDeleteVertexArrays(1, &vao);
        glfwTerminate();
        return 1;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);

    // 初始化按键状态数组
    for (int i = 0; i < 1024; i++)
        keys[i] = false;

    // 设置初始鼠标位置到中心
    recenterCursor(window);

    unsigned int seed;
    if (argc > seed_argument_index) {
        sscanf(argv[seed_argument_index], "%u", &seed);
    }
    else {
		seed = static_cast<unsigned int>(time(nullptr));
    }
    std::cout << "Random seed: " << seed << std::endl;
    initFractal(seed);

    unsigned char frame_count = 0;
    for (double last_t = glfwGetTime(); !glfwWindowShouldClose(window);) {
        double t = glfwGetTime();
        processInput(t - last_t);
        frame_count++;
        if (frame_count == 0)
			std::cout << "FPS: " << 1.0 / (t - last_t) << std::endl;
        last_t = t;
        // 每帧将鼠标移动到屏幕中心
        if (mouseCaptured) {
            recenterCursor(window);
        }

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(raymarch_program.program_id);
        glUniformMatrix4fv(raymarch_program.view_mat_inv_loc, 1, GL_FALSE, glm::value_ptr(camera.getInvViewMatrix()));
        glUniform1f(raymarch_program.aspect_ratio_loc, config_state.active_config.runtime.aspect_ratio);

        // 传递blocks数组到GPU
        ivec3 blockArray[BLOCK_AMOUNT];
        auto it = --blocks.end();
        for (int i = 0; i < BLOCK_AMOUNT; i++) {
            blockArray[i] = ivec3(*it);
            --it;
        }
        glUniform3iv(raymarch_program.blocks_loc, BLOCK_AMOUNT_GPU, glm::value_ptr(blockArray[0]));

        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glfwPollEvents();
        glfwSwapBuffers(window);

        if (console_requested)
            runConsoleSession(window, &raymarch_program);

        updateFractal(camera.getPos());
    }

    destroyRaymarchProgram(&raymarch_program);
    glDeleteVertexArrays(1, &vao);
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
            recenterCursor(window);
            firstMouse = true;
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
    else if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        console_requested = true;
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
void processInput(double delta_time) {
    const double move_speed = delta_time * sdMenger(camera.getPos()); // 调整移动速度
    const float move_delta = static_cast<float>(move_speed);

    // 地平面移动（WASD）
    if (keys[GLFW_KEY_W])
        camera.moveForward(move_delta);
    if (keys[GLFW_KEY_S])
        camera.moveForward(-move_delta);
    if (keys[GLFW_KEY_A])
        camera.moveRight(-move_delta);
    if (keys[GLFW_KEY_D])
        camera.moveRight(move_delta);

    // 垂直移动（SPACE向上，LCTRL向下）
    if (keys[GLFW_KEY_SPACE])
        camera.moveUp(move_delta);
    if (keys[GLFW_KEY_LEFT_CONTROL])
        camera.moveUp(-move_delta);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    syncRuntimeConfig(width, height);
}

void recenterCursor(GLFWwindow* window) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    const double centerX = width * 0.5;
    const double centerY = height * 0.5;
    glfwSetCursorPos(window, centerX, centerY);
    lastX = static_cast<float>(centerX);
    lastY = static_cast<float>(centerY);
}

WindowSize getInitialWindowSize() {
    const int screen_width = GetSystemMetrics(SM_CXSCREEN);
    const int screen_height = GetSystemMetrics(SM_CYSCREEN);

    WindowSize window_size = {
        static_cast<int>(screen_width * INITIAL_WINDOW_SCALE),
        static_cast<int>(screen_height * INITIAL_WINDOW_SCALE),
    };

    if (window_size.width <= 0)
        window_size.width = DEFAULT_WINDOW_WIDTH;
    if (window_size.height <= 0)
        window_size.height = DEFAULT_WINDOW_HEIGHT;

    return window_size;
}

void syncRuntimeConfig(int width, int height) {
    if (height <= 0)
        return;

    const float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
    config_state.active_config.runtime.aspect_ratio = aspect_ratio;
    config_state.staged_config.runtime.aspect_ratio = aspect_ratio;
}

std::unordered_map<std::string, std::string> getShaderSourceReplacements(const AppConfig& app_config) {
    return {
        {"__CONFIG_DEFINES__", buildShaderDefineBlock(app_config, parameter_registry)},
    };
}

bool buildRaymarchProgram(const AppConfig& app_config, RaymarchProgram* raymarch_program, std::string& error_message) {
    RaymarchProgram next_program;
    ShaderProgram shader_program;

    Shader vertex_shader(GL_VERTEX_SHADER);
    if (!vertex_shader.compileFile("./shader.vert")) {
        error_message = "vertex shader compilation failed:\n" + vertex_shader.getLastError();
        glDeleteProgram(shader_program.getProgramId());
        return false;
    }
    shader_program.attachShader(vertex_shader);

    Shader fragment_shader(GL_FRAGMENT_SHADER);
    if (!fragment_shader.compileFile("./shader.frag", getShaderSourceReplacements(app_config))) {
        error_message = "fragment shader compilation failed:\n" + fragment_shader.getLastError();
        glDeleteProgram(shader_program.getProgramId());
        return false;
    }
    shader_program.attachShader(fragment_shader);

    if (!shader_program.link()) {
        error_message = "shader program linking failed:\n" + shader_program.getLastError();
        glDeleteProgram(shader_program.getProgramId());
        return false;
    }

    next_program.program_id = shader_program.getProgramId();
    next_program.blocks_loc = glGetUniformLocation(next_program.program_id, "blocks");
    next_program.view_mat_inv_loc = glGetUniformLocation(next_program.program_id, "view_inv");
    next_program.aspect_ratio_loc = glGetUniformLocation(next_program.program_id, "aspect_ratio");

    *raymarch_program = next_program;
    error_message.clear();
    return true;
}

bool reloadRaymarchProgram(const AppConfig& app_config, RaymarchProgram* raymarch_program, std::string& error_message) {
    RaymarchProgram next_program;
    if (!buildRaymarchProgram(app_config, &next_program, error_message))
        return false;

    destroyRaymarchProgram(raymarch_program);
    *raymarch_program = next_program;
    glUseProgram(raymarch_program->program_id);
    return true;
}

void destroyRaymarchProgram(RaymarchProgram* raymarch_program) {
    if (raymarch_program->program_id != 0)
        glDeleteProgram(raymarch_program->program_id);

    raymarch_program->program_id = 0;
    raymarch_program->blocks_loc = -1;
    raymarch_program->view_mat_inv_loc = -1;
    raymarch_program->aspect_ratio_loc = -1;
}

bool isUnsignedIntegerString(const char* text) {
    if (text == nullptr || *text == '\0')
        return false;

    for (const char* ch = text; *ch != '\0'; ch++) {
        if (*ch < '0' || *ch > '9')
            return false;
    }

    return true;
}

bool loadStartupConfigScript(
    const char* config_path,
    bool config_path_is_required,
    RaymarchProgram* raymarch_program,
    std::string& error_message
) {
    std::ifstream config_stream(config_path);
    if (!config_stream.is_open()) {
        if (!config_path_is_required) {
            error_message.clear();
            return true;
        }

        error_message = std::string("failed to open startup config file: ") + config_path;
        return false;
    }

    ParameterConsoleContext parameter_console_context;
    parameter_console_context.config_state = &config_state;
    parameter_console_context.parameter_registry = &parameter_registry;
    parameter_console_context.apply_config =
        [raymarch_program](const AppConfig& app_config, std::string& reload_error_message) {
            return reloadRaymarchProgram(app_config, raymarch_program, reload_error_message);
        };

    if (!applyParameterScript(config_stream, std::cout, parameter_console_context, error_message)) {
        error_message = std::string("failed to process startup config file '") + config_path + "': " + error_message;
        return false;
    }

    return true;
}

void clearKeyStates() {
    for (bool& key_state : keys)
        key_state = false;
}

void runConsoleSession(GLFWwindow* window, RaymarchProgram* raymarch_program) {
    console_requested = false;

    sendRenderWindowToBack(window);
    focusConsoleWindow();
    setMouseCaptureState(window, false);
    clearKeyStates();

    ParameterConsoleContext parameter_console_context;
    parameter_console_context.config_state = &config_state;
    parameter_console_context.parameter_registry = &parameter_registry;
    parameter_console_context.apply_config =
        [raymarch_program](const AppConfig& app_config, std::string& error_message) {
            return reloadRaymarchProgram(app_config, raymarch_program, error_message);
        };
    runParameterConsole(std::cin, std::cout, parameter_console_context);

    if (window != nullptr && !glfwWindowShouldClose(window)) {
        bringRenderWindowToFront(window);
        setMouseCaptureState(window, true);
    }

    clearKeyStates();
}

HWND getGlfwWindowHandle(GLFWwindow* window) {
    if (window == nullptr)
        return nullptr;

    return glfwGetWin32Window(window);
}

void focusConsoleWindow() {
    HWND console_window = GetConsoleWindow();
    if (console_window == nullptr || !IsWindow(console_window))
        return;

    ShowWindow(console_window, SW_RESTORE);
    SetForegroundWindow(console_window);
    SetFocus(console_window);
}

void sendRenderWindowToBack(GLFWwindow* window) {
    HWND render_window = getGlfwWindowHandle(window);
    if (render_window == nullptr || !IsWindow(render_window))
        return;

    SetWindowPos(
        render_window,
        HWND_BOTTOM,
        0,
        0,
        0,
        0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE
    );
}

void bringRenderWindowToFront(GLFWwindow* window) {
    HWND render_window = getGlfwWindowHandle(window);
    if (render_window == nullptr || !IsWindow(render_window))
        return;

    ShowWindow(render_window, SW_RESTORE);
    SetWindowPos(
        render_window,
        HWND_TOPMOST,
        0,
        0,
        0,
        0,
        SWP_NOMOVE | SWP_NOSIZE
    );
    SetWindowPos(
        render_window,
        HWND_NOTOPMOST,
        0,
        0,
        0,
        0,
        SWP_NOMOVE | SWP_NOSIZE
    );
    SetForegroundWindow(render_window);
    SetFocus(render_window);
}

void setMouseCaptureState(GLFWwindow* window, bool should_capture) {
    mouseCaptured = should_capture;

    if (!should_capture) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        return;
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    recenterCursor(window);
    firstMouse = true;
}

GLFWwindow* initOpenGL() {

    //initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    // create window
    const WindowSize initial_window_size = getInitialWindowSize();
    syncRuntimeConfig(initial_window_size.width, initial_window_size.height);
    GLFWwindow* window = glfwCreateWindow(
        initial_window_size.width,
        initial_window_size.height,
        "Menger Sponge",
        nullptr,
        nullptr
    );
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
    framebuffer_size_callback(window, width, height);

    // set callback functions
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
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

void initFullscreenQuad(GLuint* vao) {
    // init VAO
    glGenVertexArrays(1, vao);
    glBindVertexArray(*vao);

    // init VBO
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);

    GLuint EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices, indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}
