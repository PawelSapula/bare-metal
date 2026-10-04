# Bare Metal - Low level Workspace
Welcome to my low-level workspace! Below is a little introduction about my projects and preferred setups.

# Projects in this repo 

## 💡 blinky_h755
**Have you ever wondered how much you can optimize the typical embedded "Hello World" project?**<br> <br>
I rewrote blinky in **Aarch64 Thumb instruction set** specialised for efficiency and low power consumption, perfectly suited for microcontrollers. <br>
This project is based on the manuals for **STM32 H755x** and other variants of the microcontrollers in the same family, having an **Arm Cortex-M7 CPU**. <br>
This MCU comes with many in-built peripherals including three user diodes which I used to complete Blinky without any external wiring over a specific GPIO port. <br>

<br>
Despite reducing the size of the Blinky from typical C code or using a fully fledged RTOS, there were many things that incremented the program size, due to the advanced structure and components of the board. <br>
This icncludes for example enabling different peripheral clocks and managing an entry point for the processor's boot process. <br> <br>

### This experiment ended up with an impressive.... **192 bytes**! <br>
<img width="656" height="395" alt="image" src="https://github.com/user-attachments/assets/5c2981ca-5a89-4c74-bbef-ba0f9f7087bc" />

<br><br>
..... I must admit I'm lying a little here, this is due to two data variables existing on the memory instead of the flash. <br>
It is fully possible to change this, where we tell the linker to initially store our ram memory on the flash, and then manually load it on the stack. <br>
I was too excited and decided that the experiment could end here before 10 more hours of potential debugging and documentation reading if something went wrong. <br>
Both variables are of word type, so we can additionally add 8 bytes to the final sum. <br>
Copying the memory over to the stack would also consume some bytes itself, giving my final estimate of **~220 bytes**.

<br><br>

**Note for the interested:**
- An arm-wabi compiler is needed to compile the code.
- Can flash using STM32CubeProgrammer (first thing I came up with)
- Use the .elf file to flash it if you want to try out a compiled version. (Includes linker info that STM32Cube handles)
- Compiled binary doesn't work (comment in section over) and is only for showcase.

<br><br>

## 💻 asm/128-bit-register
- Attempt on making a uint128_t. Utilizing two 64-bit registers to perform hexidecimal to decimal conversion using division of whole numbers. 
- Note: Aarch64 XNU MacOS system calls. Therefore MacOS exclusive.
- Things to improve: Code structure for clarity, let the user convert an arbitrary amount of hexes (Program malfunctions when < 32)<br> <br>
    Showcase: <br>
    <img width="520" height="58" alt="image" src="https://github.com/user-attachments/assets/2e4114cf-9905-40d7-872f-b1bae0598464" />


# Files:
```
├── asm
│   ├── 128-bit-register.s                                      - Subproj. 128-bit register
│   ├── asm_guide.md                                            - Notes for learning Aarch64 
│   ├── asm.s               - Test code
|   ├── blinky_h755/                                            Subproject: Blinky in assembly
│   ├── c_to_asm.c          - Tests with deassembler
│   ├── c_to_asm.s          
│   ├── fibonacci_long.s    - Fibonacci project
│   ├── syscalls.txt                                            - Aarch64 XNU (MacOS Syscalls)
├── c
|   └── vulkan                                                  - Subproj: Electrical circuit simulator
       └── mystd                                       - Subproj: Code collection of useful programming concepts remade in C.
```

# Setup
 - `LLVM` - Compiler infrastructure providing toolchains and framework. Similar to the OG GCC, and often a modern alternative.
    - `Clang` - A compilator frontend for C, C++ & Objective- versions. Works also with assembly.
    - `LLDB` - Default debugger for MacOS and IOS systems.

Initial tools: `xcode-select --install`

# Guide to compiling & debugging
 Compile: clang `<source-file.*>` -o `<output-exec>` <br>
 Compile down to asm: clang -S `<source-file.*>` -O`<level>`

 <br>

 Debugger LLDB:
  - Start a session: `lldb (can specify target here)`
  - Initialize a target: `$ target create <executable>`
  - Breakpoints `$ b <func-name>`
  - GUI view: `$ gui`
  - Commands: Continue/Next/Step/Print -> `c/n/s/p`

<br>

# Optional: Voltron Python TUI debugger for productivity
This section will cover installation of Voltron, a standalone and my personal installation.
Standalone, Voltron isn't the most effective tool. Follow "Full installation" for full setup.

### Standalone installation:
  - Install voltron via. installation script.
  - Repo: `https://github.com/snare/voltron`
  - Pip: `python3 -m pip install voltron`
  - Implement installation entry point to `.lldbinit` if not done automatically:
    -  `command script import /path/to/voltron/entry.py`

Start LLDB session, if `Voltron loaded.` not present, try `$ voltron init`. <br>
New terminal -> Start voltron instance: `(python3 -m) voltron view <mode>` <br>

### Full installation:
  - Install voltron via. installation script.
  - Repo: `https://github.com/snare/voltron`
  - Create a python virtual environment in workspace: `python3 -m venv .venv`
  - Use following command for installation `./install.sh -v /path/to/venv -b lldb` (Encourage to do this from home dir!)
  - Create a local `.lldbinit` file and move contents written from home dir's `.llbdinit`.
  - Allow initialization from working directories: `settings set target.load-cwd-lldbinit true`


# Optional: Tmux - A terminal multiplexer
- Allows for different operations for your terminal.
- Installed via. Brew <br>

Command basics:
 - `tmux` - Start an unnamed session
 - `tmux new -s <name>` - Start a named session
 - `tmux ls`- List sessions
 - `tmux a -t <session>` - Attach to a session 
 - `tmux kill-session -t <session`> - Kill a session

Session control:
 - Leader key: `Ctrl+b` (default)
 - Detach from session: `leader + d`

TMUX Provides basic functionalities for panes and windows. Focusing on panes here. <br>

Panes:
 - Split vertical: `leader + %`
 - Split horizontal: `leader + "`
 - Move between panes: `leader + arrow keys`
 - Zoom in/out pane: `leader + z`
 - Close pane: `leader + x`

### Utility: Tmuxinator - Scripting tool for Tmux.
 - Installed via Brew.

<br>

 - Good to know: 
    - Create project: `tmuxinator new <name>`
        - `$EDITOR` might not be set, edit in `.bashrc/zshrc`
    - Create local project: `tmux new --local <name>` (For actual repo)
    - Tmux setup to config: `tmux list-windows`

<br>

Voltron creator setup which i also use is provided in the repo. <br>
Important! Change your root directory path in the `.tmuxinator.yml` file. <br>
Start it with `tmuxinator start voltron`. <br>
