# ViewPort

By Varak Tanashian & Ethan Brook.

This is our videoplayer project.

The PDF of the report can be found in [report/report.pdf](report/report.pdf).

# TODO / Roadmap

## Data Storage (Database)

Implement a database layer for storing video and user metadata.

### Videos Table

| Field       | Type     | Description                         |
|------------|----------|-------------------------------------|
| `vid`      | pointer  | Internal video identifier           |
| `title`    | string   | Video title                         |
| `description` | string | Video description                  |
| `date`     | datetime | Upload / creation date              |
| `transcript` | string | Transcript text for searching       |
| `thumbnail` | png      | Generated video thumbnail           |
| `user`     | pointer  | Reference to the uploader           |

### Users Table

| Field      | Type   | Description               |
|------------|--------|---------------------------|
| `userid`   | int    | Unique user identifier    |
| `username` | string | User name                 |

**Future work:**
- Store metadata in the database instead of scanning the filesystem
- Use transcript and description fields for improved search

---

## Search Feature Improvements

Current search works by filtering video titles in memory.

**Planned improvements:**

- Integrate search with the database
- Support searching across:
  - Title
  - Description
  - Transcript
- Improve UI layout so results **do not stretch vertically** when there are few results
- Optionally add ranking / relevance ordering

---

## Project Learning Goals Implementation

These features are required to satisfy **operating systems / concurrency learning objectives**.

### Category 1 — Multi-Process Architecture (Pipes)

Implement a worker-based processing architecture:

- Central **Controller process**
- At least **3 worker processes running concurrently**
- Workers communicate with controller via **pipes**
- Shared data between processes as needed
- Implement a **worker pool**
- Fork workers dynamically as new tasks arrive

**Example worker tasks:**
- Video decoding
- Thumbnail generation
- Search processing
- Recommendation generation

### Category 2 — Multi-User Streaming (Sockets)

Support multiple clients accessing the application concurrently.

**Requirements:**

- Handle client connections using **sockets**
- Multiple users can connect simultaneously
- System remains stable if a client disconnects
- Controller process manages:
  - Search results
  - Recommendation generation
- Worker processes handle streaming video data to each client
- Allow **concurrent streaming of the same video**
  - File reads should not block other streams

---

## Future Features

- Graph-based **video recommendation system**
- Improved playback UI
- Fullscreen / small-screen playback mode
- Performance improvements for streaming and decoding

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

Run `make` to build the project.
Run `make run` to build and immediately run the project.
Run `make clean` to clean the project.

After you finish the `make` command, it will generate a file called `vp`.

Run `./vp` to start up the project.
