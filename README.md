# Super Mario OS

![Super Mario Bros](https://images-wixmp-ed30a86b8c4ca887773594c2.wixmp.com/f/6e64b8f7-82f5-47e5-9319-e2e69ca6f56d/d9f6x59-83bc7697-99b5-4ab1-b56c-48286f982b2b.gif?token=eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJzdWIiOiJ1cm46YXBwOjdlMGQxODg5ODIyNjQzNzNhNWYwZDQxNWVhMGQyNmUwIiwiaXNzIjoidXJuOmFwcDo3ZTBkMTg4OTgyMjY0MzczYTVmMGQ0MTVlYTBkMjZlMCIsIm9iaiI6W1t7InBhdGgiOiIvZi82ZTY0YjhmNy04MmY1LTQ3ZTUtOTMxOS1lMmU2OWNhNmY1NmQvZDlmNng1OS04M2JjNzY5Ny05OWI1LTRhYjEtYjU2Yy00ODI4NmY5ODJiMmIuZ2lmIn1dXSwiYXVkIjpbInVybjpzZXJ2aWNlOmZpbGUuZG93bmxvYWQiXX0.mnTmFaDVNoVFe89N6uVgroG2RosfYFWHRAjuRqImtII)

## Overview

Super Mario OS is a 32-bit educational operating system built from scratch. It is a **unikernel**, meaning the application (a Super Mario Bros. game) is compiled directly with the kernel into a single executable. The core of the OS is named "Sextant".

The primary goal of this project is to demonstrate fundamental OS concepts, such as:
- Preemptive multitasking
- Memory management
- Hardware drivers
- Synchronization primitives (Mutex, Semaphores)
- Concurrency

## Project Structure

The codebase is organized into the following directories:

-   `sextant/`: Contains the core kernel "Sextant".
    -   `ordonnancements/`: The preemptive Round-Robin scheduler.
    -   `memoire/`: A basic memory manager.
    -   `interruptions/`: Interrupt handling (IDT, IRQ).
    -   `Synchronisation/`: Mutex, Semaphore, and Spinlock implementations.
-   `drivers/`: Hardware drivers for the keyboard, timer, serial port, VGA, and Bochs VBE for higher-resolution graphics.
-   `hal/`: The Hardware Abstraction Layer, which includes low-level boot code (`multiboot.S`), port I/O functions, and PCI bus scanning.
-   `Applications/`: The Super Mario Bros. game logic. As a unikernel, this is the only application.
-   `sprites/`: Contains the game's graphical assets (sprites, tilesets).
-   `build/`: Stores the build artifacts, including the final `grub.iso` boot image.

## How it Works

### Booting
The OS is booted using GRUB, following the Multiboot standard.

### Kernel: Sextant
The "Sextant" kernel provides essential OS services. It uses a preemptive Round-Robin algorithm to schedule threads, allowing multiple tasks to run concurrently.

### The Super Mario Bros. Game

The project culminates in a playable, albeit simplified, version of the classic Super Mario Bros. The game features:

-   **Player Movement**: Mario can run left and right and jump. The physics simulation includes gravity and collision detection against the level map.
-   **Enemies**: The iconic Goombas are included as the primary enemy. They can be defeated by stomping on them, which is detected by checking the player's vertical velocity and position upon collision.
-   **Level Design**: The game loads a representation of the famous World 1-1. A Python script (`Applications/MarioBros/level_generator.py`) parses a PNG image of the original level and analyzes its tiles. It then generates a C++ header file (`Applications/Level/LevelCollision.h`) containing a `collision_map` array. This array is used by the game logic for collision detection.
-   **Scrolling**: The camera smoothly follows Mario's horizontal movement through the level.
-   **Win/Loss Conditions**: The game is won by reaching the flagpole at the end of the level. A game-over state is triggered if Mario loses all his lives, either by falling into a pit or taking damage from an enemy.
-   **Sprites**: All graphics for Mario, Goombas, and level tiles are stored as static C++ arrays within header files (e.g., `sprites/MarioSprites.h`, `sprites/GoombaSprite.h`).

### Game Architecture: A Multithreaded Approach
Instead of a traditional separation between kernel space and user space, Super Mario OS runs the game as a collection of threads directly within the kernel. This simplifies the design and is a classic feature of a unikernel.

The game's logic is divided into four main threads that work together:
1.  **`KeyboardThread`**: Manages keyboard input from the player.
2.  **`LogicThread`**: Handles Mario's game logic, including movement, physics, and actions. This thread is the heart of the game, checking for collisions with the level and enemies, and updating the game state.
3.  **`MobLogic`**: Controls the behavior and state of enemies (mobs). It's responsible for moving the Goombas and checking their state.
4.  **`GameDisplay`**: Responsible for rendering the game world, including the level, characters, and HUD, to the screen.

These threads communicate and stay synchronized through a shared data structure defined in `Applications/GameData.h`. This structure holds all the critical game state information, such as Mario's position, the score, number of lives, and the state of all enemies. Semaphores are used to prevent race conditions when accessing this shared data.

### Graphics and Rendering

The visual presentation of Super Mario OS is handled by a combination of a dedicated graphics driver, a rendering thread, and a palette-based color system, designed for efficiency.

*   **`EcranBochs` Driver:** The core of the graphics system is the `EcranBochs` driver (`drivers/EcranBochs.cpp`). It uses the [Bochs VBE Extensions](https://wiki.osdev.org/Bochs_VBE_Extensions) to set up a 720x240 graphical mode with 8-bit color depth. The driver configures a large virtual screen in video memory (VRAM) that is wide enough to hold the entire game level.

*   **Palette Color System:** The game operates on an 8-bit (256 colors) palette, a classic technique for retro graphics. This palette is defined in `sprites/palette.h` and loaded into the graphics hardware by the `EcranBochs` driver at startup. All sprites in the game are stored as arrays of bytes, where each byte is an index corresponding to a color in this global palette. The color `255` is treated as transparent.

*   **`GameDisplay` Rendering Thread:** This thread is the game's painter. It runs in a continuous loop, drawing all visual elements to the screen based on the state found in the shared `GameData` structure.
    *   **Optimized Drawing:** To avoid flickering and the cost of redrawing the entire screen each frame, the `GameDisplay` thread uses an optimized approach. When a sprite (like Mario or a Goomba) moves, the thread first redraws the small portion of the background where the sprite *used* to be, effectively erasing it. It then draws the sprite at its new location. This targeted update is managed by the `plot_moving_sprite` function.
    *   **Hardware Scrolling:** The entire level background is rendered once to a large off-screen portion of the VRAM at the start of the game. The illusion of a scrolling camera is achieved by instructing the `EcranBochs` driver to change the display's starting offset (`set_offset`). This effectively pans the visible 720x240 window across the larger level map in VRAM, which is a very efficient hardware-accelerated operation.
    *   **Text Rendering:** There is no traditional font rendering system. Instead, each alphanumeric character is a small, pre-drawn sprite. To display text like the HUD ("MARIO", score, lives), the `GameDisplay` thread dynamically combines these individual character sprites into a larger temporary sprite representing the full string, which is then drawn to the screen.



## How to Build and Run

### Requirements
- A 32-bit GCC-compatible toolchain (e.g., `i686-elf-gcc`).
- `make`
- `qemu`

### Build
To compile the kernel and create the bootable ISO image, run:
```sh
make
```

### Run
To launch the operating system in the QEMU emulator, use the following command:
```sh
make run_gui
```
This will start QEMU and boot the Super Mario OS.
