# Soundmind - Absolute Pitch Test Game

[English (Current)](README.md) | [中文](README_CN.md)

Soundmind is an absolute pitch testing game developed on DE1-SoC FPGA. Players are challenged to identify musical notes by ear, progressing through different difficulty levels to test and improve their absolute pitch ability.

## Features

- 🎵 Support for 5 musical notes (C, D, E, F, G) playback and recognition
- 🎮 Graphical user interface display
- 🎯 4 difficulty levels
- 📚 Practice mode for individual note training
- 🎨 Beautiful game interface and animation effects

## Hardware Requirements

- DE1-SoC FPGA development board
- PS/2 keyboard
- VGA monitor
- Audio output device

## Game States

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

## Difficulty Levels

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

## Project Structure

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

## Compilation and Execution

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

## Controls

- **Spacebar**: Start game/confirm selection
- **Up/Down arrows**: Adjust difficulty level
- **C, D, E, F, G keys**: Input corresponding notes
- **Enter key**: Access settings screen from practice mode

## Technical Details

### Hardware Interfaces

- Display control: `0xFF203020`
- PS/2 keyboard: `0xFF200100`
- Audio control: `0xFF203040`
- Timer: `0xFF202000`

### Display System

- Resolution: 320x240
- Uses double buffering technique
- RGB565 color format

### Audio System

- Supports real-time playback of 5 notes
- Uses audio FIFO buffer
- Adjustable volume and playback speed

## Contributing

Issues and Pull Requests to improve the project are welcome.

## License

This project is licensed under the Apache License 2.0. See the [LICENSE](LICENSE) file for details.