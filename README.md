PACMAN (made with C and raylib)

Hey! This is a Pacman game I built in C using raylib 6.0. This file explains how to get it running on your computer. It works on Windows, Linux and Mac.


WHAT'S IN THE FOLDER

  main.c, ghost.c, collision.c, menu.c   the game's code
  raylib/                                the raylib library files
  .vscode/                               VS Code settings (build and run setup)

Inside the raylib folder you need one subfolder for your system, named exactly like this:

  Windows:  raylib-6.0_win64_mingw-w64
  Linux:    raylib-6.0_linux_amd64
  Mac:      raylib-6.0_macos

Each of those should have an "include" folder and a "lib" folder with libraylib.a inside. You can download the right one from the raylib releases page on GitHub (https://github.com/raysan5/raylib/releases) and just unzip it into the raylib folder.


WHAT YOU NEED INSTALLED

Windows: MSYS2 (https://www.msys2.org/) with the UCRT64 tools, so you have gcc and gdb. Make sure C:\msys64\ucrt64\bin is in your PATH.

Linux: gcc, gdb and the OpenGL/X11 development files. On Ubuntu or Debian you can get everything with:
  sudo apt install build-essential gdb libgl1-mesa-dev libx11-dev

Mac: the Xcode Command Line Tools. Install them by running:
  xcode-select --install

If you want to use VS Code (recommended), also install the "C/C++" extension by Microsoft.


THE EASY WAY: VS CODE

1. Open the file raylib_template.code-workspace in VS Code.
2. Press F5 and pick "Debug Raylib App".

That's it. It builds the game first and then starts it. If you only want to build without running, press Ctrl+Shift+B (Cmd+Shift+B on Mac).


THE TERMINAL WAY

Open a terminal in the project folder and run the command for your system.

Linux:

  gcc -g main.c ghost.c collision.c menu.c -Iraylib/raylib-6.0_linux_amd64/include raylib/raylib-6.0_linux_amd64/lib/libraylib.a -lGL -lm -lpthread -ldl -lrt -lX11 -o Pacman

  ./Pacman

Windows:

  gcc -g main.c ghost.c collision.c menu.c -Iraylib/raylib-6.0_win64_mingw-w64/include raylib/raylib-6.0_win64_mingw-w64/lib/libraylib.a -lopengl32 -lgdi32 -lwinmm -Wl,--defsym,_stat64=_stat64i32 -o Pacman

  Pacman.exe

Mac:

  gcc -g main.c ghost.c collision.c menu.c -Iraylib/raylib-6.0_macos/include raylib/raylib-6.0_macos/lib/libraylib.a -framework OpenGL -framework Cocoa -framework IOKit -framework CoreAudio -framework CoreVideo -o Pacman

  ./Pacman

Tip: always run the game from the main project folder, so it can find any images, sounds or fonts it needs.


IF SOMETHING GOES WRONG

"raylib.h: No such file or directory"
  The raylib folder is missing or named wrong. Double-check the folder name against the list above.

"cannot find -lraylib" or "libraylib.a: No such file"
  Make sure libraylib.a is inside the lib folder of your raylib folder.

Linux: "cannot find -lGL" or "-lX11"
  Install the OpenGL and X11 packages from the Linux step above.

Windows: "gcc is not recognized"
  Add C:\msys64\ucrt64\bin to your PATH, then restart VS Code or your terminal.

Windows: the debugger won't start
  Check that gdb.exe exists at C:\msys64\ucrt64\bin\gdb.exe. If not, open the MSYS2 terminal and run:
  pacman -S mingw-w64-ucrt-x86_64-gdb

Mac: "lldb-mi not found"
  Just build and run from the terminal instead.

Red squiggly lines in VS Code
  Press Ctrl+Shift+P, choose "C/C++: Select a Configuration...", and pick Win32, Linux or Mac.



Have fun!