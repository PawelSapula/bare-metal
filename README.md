# Guide on my low-level workspace
Important tools and setups to be able to reproduce my low-level programming setup on any MacOS machine.

# Projects in this repo 
**asm/128-bit-register**
    - Attempt on making a uint128_t. Utilizing two 64-bit registers to perform hexidecimal to decimal conversion using division of whole numbers. <br> <br>
    Showcase: <br>
    - Note: Aarch64 XNU MacOS system calls. Therefore MacOS exclusive.
    - Things to improve: Code structure for clarity, let the user convert an arbitrary amount of hexes (Program malfunctions when < 32)
    <img width="520" height="58" alt="image" src="https://github.com/user-attachments/assets/2e4114cf-9905-40d7-872f-b1bae0598464" />



# Files:
```
├── asm
│   ├── 128-bit-register.s
│   ├── ARMv7-M Architecture Reference Manual.pdf               - V7 (Microcontrollers etc.)
│   ├── ARMv8 for ARMv8-A Architecture Reference Manual.pdf     - V8 (Normal processors etc.)
│   ├── asm_guide.md                                            - Notes for learning Aarch64 
│   ├── asm.s               - Random code
│   ├── c_to_asm.c          - Tests with deassembler
│   ├── c_to_asm.s          
│   ├── fibonacci_long.s    - Fibonacci project
│   ├── syscalls.txt                                            - Aarch64 XNU (MacOS Syscalls)
├── c
│   ├── dynamic_array.c                                         - Test of a dynamic array (heap testing)
|   └── vulkan                                                  - Vulkan project
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
