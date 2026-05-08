# Menger Sponge Raymarching

An OpenGL ray-marching renderer for an infinite Menger sponge. The camera can fly through the fractal, while the program records the sub-block path needed to keep the scene continuous across levels.

This is v1.2. Bilibili video demo for v1.0: <https://www.bilibili.com/video/BV1N6yUBxEJx>.

## Usage

For normal use, download the release package from the **Releases** section on the right side of the GitHub page. The package contains the executable, shaders, and config files. Extract it, then run the executable from the extracted directory.

You can also build and run the Visual Studio project from the project directory so that `shaders/shader.vert`, `shaders/shader.frag`, and optional config files can be found with relative paths.

```txt
menger-sponge-raymarching.exe
menger-sponge-raymarching.exe 12345
menger-sponge-raymarching.exe config.txt
menger-sponge-raymarching.exe config.txt 12345
```

The optional numeric argument is the random seed. If the first argument is not numeric, it is treated as a startup config script path, and the optional seed moves to the second argument. Without a custom config path, the program tries to load `config.txt` if it exists.

## Controls

### Movements

- **W/A/S/D**: Move forward/left/backward/right.
- **Space**: Move up.
- **Left Ctrl**: Move down.
- **Mouse movement**: Look around.
- **Alt**: Release mouse control while held.
- **P**: Open the parameter console.
- **Esc**: Exit.

## Parameter Console

Press **P** while the renderer is running. The render window is moved behind the console window and the mouse cursor is released. After leaving the parameter console, the render window is brought back to the front and mouse capture is restored.

Commands:

```txt
help
show
<name> <value>
apply
cancel
```

Parameter updates are staged first. Type `apply` to rebuild the shader with the staged values, or `cancel` to discard them. If shader compilation fails, the old shader remains active and the console stays open so the value can be adjusted.

Shader define parameters are injected as raw strings. Numeric parameters receive a small literal check; GLSL expression parameters are validated mainly by shader compilation.

## Startup Config Script

A config script uses the same line format as the parameter console:

```txt
max_raymarching_iter 450
epsilon 0.00002
base_color_formula abs(normal) * 0.6 + 0.4 // This comment is ignored.
```

At startup, the script is applied automatically at the end of the file, so an explicit `apply` line is optional. In startup config scripts only, `//` starts a line comment and the rest of that line is ignored. The interactive parameter console does not strip `//` comments.

## Useful Parameters

- `max_iter_rough`: Rough ray-marching iteration count.
- `max_iter_relative`: Relative iteration baseline used for accurate ray marching.
- `max_iter_relative_cutoff`: Maximum relative iteration count used by the accurate pass.
- `max_raymarching_iter`: Maximum ray-marching steps for each pass.
- `cutoff_dist_far`: Maximum ray distance before a ray is treated as a miss.
- `cutoff_dist_near_rough`: Hit threshold scale for the rough pass.
- `cutoff_dist_near_relative`: Hit threshold scale for the accurate pass.
- `epsilon`: Base finite-difference epsilon for normal calculation.
- `base_color_formula`: GLSL `vec3` expression used to calculate `base_color`.

Example color formulas:

```txt
base_color_formula transpose(mat3(view_inv)) * normal * 0.5 + 0.5
base_color_formula abs(normal) * 0.6 + 0.4
base_color_formula vec3(0.9, 0.55, 0.25)
base_color_formula abs(rd_world)
base_color_formula mix(vec3(0.1, 0.2, 0.8), vec3(1.0, 0.8, 0.3), pow(1.0 - max(dot(normal, -rd_world), 0.0), 2.0))
```

Available variables around `base_color_formula` include `normal`, `rd_world`, `view_inv`, `p`, `t`, and `dist`.

---

# 门格海绵光线行进渲染器

这是一个基于 OpenGL ray marching 的无限层级门格海绵渲染器。相机可以在分形内部自由飞行，程序会记录当前进入过的子块路径，以保持跨层级移动时的场景连续性。

当前版本是 v1.2。v1.0 的 Bilibili 演示视频：<https://www.bilibili.com/video/BV1N6yUBxEJx>。

## 使用方式

普通使用可以直接在 GitHub 页面右侧的 **Releases** 里下载发布包。发布包里包含可执行文件、shader 和配置文件；解压后在解压目录中运行可执行文件即可。

也可以用 Visual Studio 构建并运行项目。运行目录需要能通过相对路径找到 `shaders/shader.vert`、`shaders/shader.frag` 和可选配置文件。

```txt
menger-sponge-raymarching.exe
menger-sponge-raymarching.exe 12345
menger-sponge-raymarching.exe config.txt
menger-sponge-raymarching.exe config.txt 12345
```

可选的数字参数是随机种子。如果第一个参数不是数字，它会被当作启动配置脚本路径；此时随机种子可以作为第二个参数传入。如果没有指定配置脚本，程序会尝试读取当前目录下的 `config.txt`，不存在则跳过。

## 操作

- **W/A/S/D**：前后左右移动。
- **Space**：向上移动。
- **Left Ctrl**：向下移动。
- **鼠标移动**：转动视角。
- **Alt**：按住时释放鼠标控制。
- **P**：打开参数控制台。
- **Esc**：退出程序。

## 参数控制台

渲染运行时按 **P** 进入参数控制台。渲染窗口会被放到后面，鼠标会释放并显示；退出参数控制台后，渲染窗口会回到前台并重新捕获鼠标。

命令：

```txt
help
show
<参数名> <参数值>
apply
cancel
```

参数修改会先进入暂存状态。输入 `apply` 后程序会用暂存值重新编译 shader；输入 `cancel` 会放弃暂存修改。如果 shader 编译失败，旧 shader 会继续使用，控制台不会退出，可以继续修改参数。

shader define 参数会按原始字符串注入。数字参数只做很轻量的字面量检查；GLSL 表达式参数主要交给 shader 编译器验证。

## 启动配置脚本

配置脚本和参数控制台使用相同的逐行格式：

```txt
max_raymarching_iter 450
epsilon 0.00002
base_color_formula abs(normal) * 0.6 + 0.4 // 这里后面的注释会被忽略
```

启动脚本读到文件结尾时会自动应用，所以文件里可以不写 `apply`。只有启动配置脚本支持 `//` 行内注释，`//` 之后的内容会被忽略；交互式参数控制台不会移除 `//` 注释。

## 常用参数

- `max_iter_rough`：粗略光线行进使用的距离估计迭代次数。
- `max_iter_relative`：精确阶段的相对迭代基准。
- `max_iter_relative_cutoff`：精确阶段允许的最大相对迭代次数。
- `max_raymarching_iter`：每个光线行进阶段的最大步数。
- `cutoff_dist_far`：超过该距离后认为光线未命中。
- `cutoff_dist_near_rough`：粗略阶段的命中阈值比例。
- `cutoff_dist_near_relative`：精确阶段的命中阈值比例。
- `epsilon`：法线有限差分计算的基础 epsilon。
- `base_color_formula`：用于计算 `base_color` 的 GLSL `vec3` 表达式。

颜色公式示例：

```txt
base_color_formula transpose(mat3(view_inv)) * normal * 0.5 + 0.5
base_color_formula abs(normal) * 0.6 + 0.4
base_color_formula vec3(0.9, 0.55, 0.25)
base_color_formula abs(rd_world)
base_color_formula mix(vec3(0.1, 0.2, 0.8), vec3(1.0, 0.8, 0.3), pow(1.0 - max(dot(normal, -rd_world), 0.0), 2.0))
```

`base_color_formula` 所在位置可以使用的变量包括 `normal`、`rd_world`、`view_inv`、`p`、`t` 和 `dist`。

---

Author: DeepSeek, Me, GPT.
