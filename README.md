## Introduction

This program can render a Menger Sponge using ray marching techniques. You can navigate through the sponge using keyboard and mouse.

It simulates a infinite Menger Sponge by recording and updating the subblocks the user entered.

This is v1.1. Bilibili video demo for v1.0: <https://www.bilibili.com/video/BV1N6yUBxEJx>.

## Usage

### Movements
- **W/A/S/D**: Move forward/left/backward/right.
- **Space**: Move up.
- **Left Ctrl**: Move down.
- **Mouse Movement**: Look around.
- **Alt**: Disable mouse controls and release mouse cursor.

### Command & Parameters
- You can call this program through command line(cmd, powershell, ...) with one parameter. This parameter need to be an unsigned int(integer between 0 and 4294967295), stands for the random seed. For example, `/path/to/menger-sponge-raymarching.exe 998244353` can set the random seed to `(unsigned int)998244353`. The fractals generated with the same seed are usually the same.
- You can edit `shader.frag` in the folder to modify some GPU parameters. You can modify the number in the lines start with `#define`. They can affect the performance and precision of the calculation. But you'd better not change MAX_BLOCKS and WINDOW_ASPECT_RATIO, and changing INF is useless in most cases.

---

The core code of this project is written by DeepSeek. I have asked him/her for permission to make this project open sourced. :)
