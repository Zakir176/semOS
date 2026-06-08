# Building SERC OS

This guide covers compiling the SERC OS mini-OS simulator on Linux and Windows.

## Linux / macOS

### Prerequisites

```bash
sudo apt-get update
sudo apt-get install build-essential pkg-config libgtk-3-dev  # Ubuntu/Debian
# or
brew install gtk+3                                             # macOS
```

### Compile

```bash
# Full build (CLI + GUI)
make all

# CLI-only build (no GTK dependency)
make cli

# Clean build artifacts
make clean
```

Binaries are produced in `bin/`:
- `bin/serc-os` — Full application (CLI + GUI)
- `bin/serc-os-cli` — CLI-only variant

### Run

```bash
./bin/serc-os        # Interactive launcher (choose CLI or GUI)
./bin/serc-os-cli    # CLI directly
```

---

## Windows (MinGW / MSYS2)

### Option 1: MSYS2 (Recommended)

**Install MSYS2** from https://www.msys2.org/

Open MSYS2 terminal and install tools:

```bash
pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-gtk3
```

Navigate to the repository and build:

```bash
cd /c/path/to/semOS
make all
make cli
```

Run:
```bash
./bin/serc-os
./bin/serc-os-cli
```

### Option 2: MinGW (GCC 12+)

**Download** MinGW-w64 from https://sourceforge.net/projects/mingw-w64/

- Choose: x86_64, native Windows threads, POSIX exception handling
- Extract and add `\bin` to your system `PATH`

**Install GTK3 development files:**
- Download from https://www.gtk.org/ or use pre-built binaries
- Add GTK include/lib paths to `PATH` and compiler flags

Then from PowerShell or Command Prompt:

```bash
cd C:\path\to\semOS
make all
make cli
```

Run:
```bash
bin\serc-os.exe
bin\serc-os-cli.exe
```

### Option 3: Windows Subsystem for Linux (WSL2)

Install WSL2 and a Linux distribution, then follow the Linux instructions.

```bash
wsl --install
# In WSL terminal:
sudo apt-get install build-essential pkg-config libgtk-3-dev
make all
./bin/serc-os
```

---

## Troubleshooting

### GTK not found (Windows)

If compilation fails with GTK errors:
1. Verify GTK3 is installed and paths are correct
2. Use `-DSERC_CLI_ONLY` flag when building CLI-only:
   ```bash
   make cli
   ```
3. Or use MSYS2, which manages dependencies automatically

### Missing gcc

Ensure your compiler is in `PATH`:
```bash
gcc --version
```

If not found, add MinGW/MSYS2 bin directory to your `PATH` environment variable.

### Build succeeds but binaries won't run

- Verify `bin/serc-os` and `bin/serc-os-cli` exist
- On Windows, ensure DLLs (libgtk, libglib, etc.) are accessible
  - Either in `bin/` directory or in system `PATH`
  - Or use CLI-only mode: `make cli`

---

## CI/CD

GitHub Actions workflow defined in `.github/workflows/build.yml`:
- Builds on every push and pull request to `main`
- Installs dependencies, compiles, and runs smoke tests
- Verifies both `bin/serc-os` and `bin/serc-os-cli` executables

---

## Project Structure

```
.
├── src/              # C source files
├── include/          # Header files
├── Makefile          # Build configuration
├── bin/              # Compiled binaries (generated)
├── obj/              # Object files (generated)
├── logs/             # Runtime logs (generated)
└── .github/workflows/# CI/CD configuration
```
