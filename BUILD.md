# Building SERC OS

This guide covers compiling the SERC OS mini-OS simulator on Linux, macOS, and Windows.

---

## Quick Start

| Platform | Command | Output |
|----------|---------|--------|
| Linux / macOS | `make all` | `bin/serc-os` (CLI + GUI) |
| Linux / macOS | `make cli` | `bin/serc-os-cli` (CLI only) |
| Windows (MSYS2) | `make all` | `bin/serc-os.exe` |
| Windows (WSL2) | `make all` | `bin/serc-os` |

---

## Linux / macOS

### 1. Install Prerequisites

**Ubuntu / Debian:**
```bash
sudo apt-get update
sudo apt-get install build-essential pkg-config libgtk-3-dev
```

**macOS (Homebrew):**
```bash
brew install gtk+3
```

### 2. Build

```bash
# Full build (CLI + GUI) — requires GTK3
make all

# CLI-only build — no GTK dependency needed
make cli

# Remove build artifacts
make clean
```

### 3. Run

```bash
./bin/serc-os        # Launcher — choose CLI or GUI
./bin/serc-os-cli    # CLI directly (no launcher)
```

---

## Windows

### Option 1: MSYS2 (Recommended)

**Step 1 — Install MSYS2**
Download from https://www.msys2.org/ and follow the installer.

**Step 2 — Open MSYS2 terminal and install build tools:**
```bash
pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-gtk3
```

**Step 3 — Navigate to the project and build:**
```bash
cd /c/path/to/semOS
make all
make cli
```

**Step 4 — Run:**
```bash
./bin/serc-os.exe
./bin/serc-os-cli.exe
```

---

### Option 2: MinGW (GCC 12+)

**Step 1 — Download MinGW-w64**
Get it from https://sourceforge.net/projects/mingw-w64/
- Architecture: x86_64
- Threads: native Windows threads
- Exception handling: POSIX

Extract and add the `\bin` folder to your system `PATH`.

**Step 2 — Install GTK3 development files**
Download from https://www.gtk.org/ or use pre-built binaries.
Add GTK include/lib paths to `PATH` and compiler flags.

**Step 3 — Build (PowerShell or Command Prompt):**
```cmd
cd C:\path\to\semOS
make all
make cli
```

**Step 4 — Run:**
```cmd
bin\serc-os.exe
bin\serc-os-cli.exe
```

---

### Option 3: Windows Subsystem for Linux (WSL2)

**Step 1 — Install WSL2:**
```cmd
wsl --install
```

**Step 2 — In the WSL terminal, follow the Linux instructions:**
```bash
sudo apt-get update
sudo apt-get install build-essential pkg-config libgtk-3-dev
make all
./bin/serc-os
```

---

## Troubleshooting

### GTK not found (Windows)
- Verify GTK3 is installed and paths are correct.
- Build CLI-only mode (no GTK needed): `make cli`
- Use MSYS2 — it manages dependencies automatically.

### Missing gcc
Check that your compiler is on the `PATH`:
```bash
gcc --version
```
If not found, add the MinGW/MSYS2 `bin` directory to your `PATH`.

### Build succeeds but binaries won't run
- Confirm `bin/serc-os` (or `bin/serc-os.exe`) exists.
- On Windows, ensure GTK DLLs (libgtk, libglib, etc.) are accessible:
  - Copy them into the `bin/` directory, **or**
  - Add their location to your system `PATH`, **or**
  - Use CLI-only mode: `make cli`

---

## CI/CD

A GitHub Actions workflow (`.github/workflows/build.yml`) runs on every push and pull request to `main`:
- Installs dependencies
- Compiles the project
- Runs smoke tests
- Verifies both `bin/serc-os` and `bin/serc-os-cli` executables

---

## Project Structure

```
.
├── src/               # C source files
├── include/           # Header files
├── Makefile           # Build configuration
├── bin/               # Compiled binaries (generated)
├── obj/               # Object files (generated)
├── logs/              # Runtime logs (generated)
└── .github/workflows/ # CI/CD configuration