# Soundmind - Absolute Pitch Test Game | 绝对音感测试游戏

[English](#english) | [中文](#chinese)

<a name="english"></a>

## English

Soundmind is an absolute pitch testing game developed on DE1-SoC FPGA. Players are challenged to identify musical notes by ear, progressing through different difficulty levels to test and improve their absolute pitch ability.

### Features

- 🎵 Support for 5 musical notes (C, D, E, F, G) playback and recognition
- 🎮 Graphical user interface display
- 🎯 4 difficulty levels
- 📚 Practice mode for individual note training
- 🎨 Beautiful game interface and animation effects

### Hardware Requirements

- DE1-SoC FPGA development board
- PS/2 keyboard
- VGA monitor
- Audio output device

### Game States

The game includes the following states:

1. **Start Screen** (`GAME_START`)
   - Displays game title and start options
   - Press spacebar to enter practice mode

2. **Practice Mode** (`GAME_STUDY`)
   - Practice each note individually
   - Press C, D, E, F, G keys to listen to corresponding notes
   - Press Enter to access the settings screen

3. **Settings Screen** (`GAME_SETUP`)
   - Select game difficulty (0-3)
   - Use up/down arrow keys to adjust difficulty
   - Press spacebar to start the game

4. **Gameplay** (`GAME_PLAYING`)
   - System randomly plays notes
   - Player must identify the notes correctly
   - Use C, D, E, F, G keys to input notes

5. **Success Screen** (`GAME_SUCCESS`)
   - Shows success message and score
   - Press spacebar to return to start screen

6. **Game Over Screen** (`GAME_OVER`)
   - Shows failure message
   - Press spacebar to return to start screen

### Difficulty Levels

- **Level 0**: Beginner level
  - Longest pause time between notes
  - Suitable for beginners

- **Level 1**: Elementary level
  - Moderate pause time between notes
  - Suitable for players familiar with game mechanics

- **Level 2**: Intermediate level
  - Short pause time between notes
  - Requires faster note identification

- **Level 3**: Advanced level
  - Minimal pause time between notes
  - Requires rapid absolute pitch recognition

### Project Structure

```
Soundmind/
├── state/              # Main source code
│   ├── display.c       # Main program file
│   ├── *.h             # Audio and image resource files
│   └── Makefile        # Compilation configuration file
├── png/                # Image resources
├── music_file/         # Audio resources
└── old_version/        # Old version code
```

### Compilation and Execution

1. Ensure necessary development tools are installed:
   - ARM cross-compilation toolchain
   - DE1-SoC development environment

2. Compile the project:
   ```bash
   cd state
   make
   ```

3. Download the compiled program to DE1-SoC:
   ```bash
   make download
   ```

### Controls

- **Spacebar**: Start game/confirm selection
- **Up/Down arrows**: Adjust difficulty level
- **C, D, E, F, G keys**: Input corresponding notes
- **Enter key**: Access settings screen from practice mode

### Technical Details

#### Hardware Interfaces

- Display control: `0xFF203020`
- PS/2 keyboard: `0xFF200100`
- Audio control: `0xFF203040`
- Timer: `0xFF202000`

#### Display System

- Resolution: 320x240
- Uses double buffering technique
- RGB565 color format

#### Audio System

- Supports real-time playback of 5 notes
- Uses audio FIFO buffer
- Adjustable volume and playback speed

### Contributing

Issues and Pull Requests to improve the project are welcome.

### License

This project is licensed under the Apache License 2.0. See the [LICENSE](LICENSE) file for details.

---

<a name="chinese"></a>

## 中文

Soundmind 是一个基于 DE1-SoC FPGA 开发的绝对音感测试游戏。玩家需要通过听辨识别音符，通过不同难度级别来测试和提高自己的绝对音感能力。

### 功能特点

- 🎵 支持 5 个音符（C、D、E、F、G）的播放和识别
- 🎮 图形化界面显示
- 🎯 4 个难度级别
- 📚 学习模式支持单独练习
- 🎨 精美的游戏界面和动画效果

### 硬件要求

- DE1-SoC FPGA 开发板
- PS/2 键盘
- VGA 显示器
- 音频输出设备

### 游戏状态

游戏包含以下状态：

1. **开始界面** (`GAME_START`)
   - 显示游戏标题和开始选项
   - 按空格键进入学习模式

2. **学习模式** (`GAME_STUDY`)
   - 可以单独练习每个音符
   - 按 C、D、E、F、G 键试听对应音符
   - 按回车键进入设置界面

3. **设置界面** (`GAME_SETUP`)
   - 选择游戏难度（0-3）
   - 使用上下箭头键调整难度
   - 按空格键开始游戏

4. **游戏进行中** (`GAME_PLAYING`)
   - 系统会随机播放音符
   - 玩家需要正确识别音符
   - 使用 C、D、E、F、G 键输入音符

5. **成功界面** (`GAME_SUCCESS`)
   - 显示成功信息和得分
   - 按空格键返回开始界面

6. **失败界面** (`GAME_OVER`)
   - 显示失败信息
   - 按空格键返回开始界面

### 难度级别

- **难度 0**：最基础难度
  - 音符间暂停时间最长
  - 适合初学者

- **难度 1**：初级难度
  - 音符间暂停时间适中
  - 适合熟悉游戏机制的玩家

- **难度 2**：中级难度
  - 音符间暂停时间较短
  - 需要更快的音符识别能力

- **难度 3**：高级难度
  - 音符间暂停时间最短
  - 需要快速的绝对音感反应

### 项目结构

```
Soundmind/
├── state/              # 主要源代码
│   ├── display.c       # 主程序文件
│   ├── *.h             # 音频和图像资源文件
│   └── Makefile        # 编译配置文件
├── png/                # 图像资源
├── music_file/         # 音频资源
└── old_version/        # 旧版本代码
```

### 编译和运行

1. 确保已安装必要的开发工具：
   - ARM 交叉编译工具链
   - DE1-SoC 开发环境

2. 编译项目：
   ```bash
   cd state
   make
   ```

3. 将编译好的程序下载到 DE1-SoC：
   ```bash
   make download
   ```

### 控制说明

- **空格键**：开始游戏/确认选择
- **上下箭头**：调整难度级别
- **C、D、E、F、G 键**：输入对应音符
- **回车键**：在学习模式中进入设置界面

### 技术细节

#### 硬件接口

- 显示控制：`0xFF203020`
- PS/2 键盘：`0xFF200100`
- 音频控制：`0xFF203040`
- 计时器：`0xFF202000`

#### 显示系统

- 分辨率：320x240
- 使用双缓冲技术
- RGB565 颜色格式

#### 音频系统

- 支持 5 个音符的实时播放
- 使用音频 FIFO 缓冲
- 可调节的音量和播放速度

### 贡献

欢迎提交 Issue 和 Pull Request 来改进项目。

### 许可证

本项目采用 Apache License 2.0 许可证。详见 [LICENSE](LICENSE) 文件。