# ViewPort

By Varak Tanashian & Ethan Brook.

This is our videoplayer project.

The PDF of the report can be found in [report/report.pdf](report/report.pdf).

# Installing dependencies

This project has the following dependencies:

- FFmpeg (libavcodec, libavformat, libavutil, libswscale, libswresample)
- SDL2 (Simple DirectMedia Layer 2)
- pkg-config (to find library paths)

Although all of them are available on the `teach.cs` machines by default, here's how to install them if running the project on a local machine

Windows:

```bash
# Using MSYS2 (Mingw-w64)
pacman -S mingw-w64-x86_64-ffmpeg mingw-w64-x86_64-SDL2 mingw-w64-x86_64-pkg-config
```

Unix (Linux/WSL):

```bash
# On Ubuntu/Debian
sudo apt-get install libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libswresample-dev libsdl2-dev pkg-config
```

On macOS (Homebrew):

```bash
brew install ffmpeg sdl2 pkg-config
```

## Buidling

Simple run `make` to build the project.
Run `make run` to build and immediately run the project.
Run `make clean` to clean the project.

After you finish the `make` command, it will generate a file called `vp`.
