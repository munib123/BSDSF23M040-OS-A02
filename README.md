# BSDSF23M040-OS-A02
# Operating Systems Lab Assignment - ls Command Implementation

[![Version](https://img.shields.io/badge/version-1.6.0-blue.svg)](https://github.com/munib123/BSDSF23M040-OS-A02)
[![Language](https://img.shields.io/badge/language-C-brightgreen.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![License](https://img.shields.io/badge/license-MIT-orange.svg)](LICENSE)

## 📋 Project Overview

A comprehensive implementation of the Unix `ls` command utility, developed incrementally through 7 versions, each adding new features while maintaining backward compatibility.

**Course:** Operating Systems  
**Instructor:** Muhammad Arif Butt, PhD  
**Student:** Muneeb  
**Repository:** [github.com/munib123/BSDSF23M040-OS-A02](https://github.com/munib123/BSDSF23M040-OS-A02)

---

## 🚀 Features Implemented

### Version History

| Version | Feature | Description | Status |
|---------|---------|-------------|--------|
| v1.0.0 | Basic Listing | Simple directory listing | ✅ |
| v1.1.0 | Long Format (-l) | Detailed file information with permissions, owner, size, timestamp | ✅ |
| v1.2.0 | Column Display | "Down then across" columnar layout with terminal width detection | ✅ |
| v1.3.0 | Horizontal (-x) | Horizontal "across then down" display mode | ✅ |
| v1.4.0 | Alphabetical Sort | Sorted output using qsort() with custom comparator | ✅ |
| v1.5.0 | Colorized Output | ANSI color codes for different file types | ✅ |
| v1.6.0 | Recursive (-R) | Recursive directory traversal | ✅ |

---

## 🛠️ Technical Implementation

### System Calls Used
- `opendir()`, `readdir()`, `closedir()` - Directory operations
- `stat()`, `lstat()` - File metadata retrieval
- `getpwuid()`, `getgrgid()` - User/group name resolution
- `ioctl(TIOCGWINSZ)` - Terminal size detection
- `getopt()` - Command-line argument parsing

### Key Algorithms
- **Column Layout:** O(n) time complexity for "down then across" formatting
- **Sorting:** O(n log n) using quicksort via `qsort()`
- **Recursion:** Depth-first directory tree traversal

### Data Structures
- Dynamic arrays with `malloc()`/`realloc()`
- String arrays for filename storage
- `struct stat` for file metadata
- `struct dirent` for directory entries

---

## 📦 Installation and Usage

### Prerequisites
```bash
gcc (GNU Compiler Collection)
make
Linux/Unix operating system
```

### Building the Project
```bash
# Clone the repository
git clone https://github.com/munib123/BSDSF23M040-OS-A02.git
cd BSDSF23M040-OS-A02

# Build using make
make

# The binary will be created at: bin/ls
```

### Usage Examples

```bash
# Basic listing (colorized, sorted, columnar)
./bin/ls

# Long format with detailed information
./bin/ls -l

# Horizontal layout
./bin/ls -x

# Recursive directory traversal
./bin/ls -R

# Combine flags
./bin/ls -lR /path/to/directory
./bin/ls -xR .
```

### Available Options
- `-l` : Long listing format (permissions, owner, size, date)
- `-x` : Horizontal column display (across then down)
- `-R` : Recursive listing of subdirectories

---

## 🎨 Colorized Output

Files are automatically color-coded based on type:

- 🔵 **Blue** - Directories
- 🟢 **Green** - Executable files
- 🔴 **Red** - Archive files (.tar, .gz, .zip)
- 🟣 **Pink** - Symbolic links
- ⚫ **Reverse** - Special files (devices, sockets, etc.)

---

## 📊 Project Structure

```
BSDSF23M040-OS-A02/
├── bin/                    # Compiled binaries
│   └── ls                  # Final executable
├── src/                    # Source code
│   ├── ls-v1.0.0.c        # Initial version
│   ├── ls-v1.1.0.c        # Long listing
│   ├── ls-v1.2.0.c        # Column display
│   ├── ls-v1.3.0.c        # Horizontal display
│   ├── ls-v1.4.0.c        # Alphabetical sort
│   ├── ls-v1.5.0.c        # Colorized output
│   └── ls-v1.6.0.c        # Recursive listing
├── obj/                    # Object files (if needed)
├── man/                    # Manual pages
├── Makefile               # Build configuration
├── README.md              # This file
├── REPORT.md              # Detailed technical report
├── VIVA_PREPARATION.md    # Viva-voce preparation guide
└── lab.txt                # Original assignment specifications
```

---

## 🔀 Git Workflow

### Branching Strategy
Each feature was developed in its own branch:
- `feature-long-listing-v1.1.0`
- `feature-column-display-v1.2.0`
- `feature-horizontal-display-v1.3.0`
- `feature-alphabetical-sort-v1.4.0`
- `feature-colorized-output-v1.5.0`
- `feature-recursive-listing-v1.6.0`

### Version Tags
All versions are tagged for easy access:
```bash
git tag
# Output: v1.1.0, v1.2.0, v1.3.0, v1.4.0, v1.5.0, v1.6.0
```

### Viewing Specific Version
```bash
# Checkout a specific version
git checkout v1.3.0

# View differences between versions
git diff v1.2.0 v1.3.0
```

---

## 📚 Documentation

### Available Documents

1. **[REPORT.md](REPORT.md)** - Comprehensive technical report
   - Answers to all assignment questions
   - Detailed explanations of algorithms
   - Code examples and diagrams
   - Learning outcomes

2. **[VIVA_PREPARATION.md](VIVA_PREPARATION.md)** - Viva-voce guide
   - Quick reference for all features
   - Common interview questions
   - Code snippets
   - Demo commands

3. **[lab.txt](lab.txt)** - Original assignment specifications

---

## 🧪 Testing

### Test Scenarios Covered
- Empty directories
- Single file
- Multiple files (sorted/unsorted)
- Nested directory structures
- Symbolic links
- Executable files
- Archive files
- Mixed file types
- Large directories
- Deep directory trees

### Sample Test Output
```bash
$ ./bin/ls -lR test_directory
test_directory:
drwxrwxr-x 2 muneeb muneeb 4096 Oct 13 10:30:00 2025 subdir1
-rwxr-xr-x 1 muneeb muneeb  124 Oct 13 10:30:00 2025 script.sh
-rw-rw-r-- 1 muneeb muneeb 1024 Oct 13 10:30:00 2025 data.txt

test_directory/subdir1:
-rw-rw-r-- 1 muneeb muneeb  512 Oct 13 10:30:00 2025 file.txt
```

---

## 🎯 Learning Outcomes

### Technical Skills Gained
- ✅ POSIX system programming
- ✅ Dynamic memory management in C
- ✅ File system operations
- ✅ Terminal control and ANSI codes
- ✅ Algorithm implementation (sorting, recursion)
- ✅ Command-line interface design

### Software Engineering Practices
- ✅ Version control with Git
- ✅ Feature-based development
- ✅ Incremental implementation
- ✅ Testing and debugging
- ✅ Documentation and reporting
- ✅ Code maintainability

---

## 🏆 Achievements

- **7 Complete Features:** All requirements implemented
- **Professional Git History:** Clean branching and merging
- **Comprehensive Documentation:** 1500+ lines of technical documentation
- **Production-Quality Code:** Well-structured, commented, and maintainable
- **Full Test Coverage:** All features tested with various scenarios

---

## 🔧 Build System

### Makefile Targets

```bash
make          # Build the project (default target)
make clean    # Remove compiled binaries and object files
make all      # Same as make
```

### Compiler Flags
```makefile
CC = gcc
CFLAGS = -Wall -g
```
- `-Wall`: Enable all warnings
- `-g`: Include debugging information

---

## 📝 Code Quality

### Standards Compliance
- POSIX C (200809L)
- GNU C Library extensions (_DEFAULT_SOURCE)

### Best Practices
- Consistent naming conventions
- Proper error handling
- Memory leak prevention
- Buffer overflow protection
- Clear function separation
- Comprehensive comments

---

## 🐛 Known Limitations

1. **Hidden Files:** `-a` flag not implemented (skip files starting with '.')
2. **Human-Readable Sizes:** `-h` flag not implemented
3. **Time Sorting:** `-t` flag not implemented
4. **Case-Insensitive Sort:** Not implemented
5. **Inode Display:** `-i` flag not implemented

These are intentional scope limitations and could be added in future versions.

---

## 🚀 Future Enhancements

Potential features for future versions:
- [ ] Add `-a` flag for hidden files
- [ ] Add `-h` flag for human-readable sizes (KB, MB, GB)
- [ ] Add `-t` flag for time-based sorting
- [ ] Add `-r` flag for reverse sorting
- [ ] Add `-S` flag for size-based sorting
- [ ] Implement `--color` option control
- [ ] Add `-1` flag for single-column output
- [ ] Support for `-F` flag (append type indicators)

---

## 👨‍💻 Author

**Muneeb**  
BSDSF23M040  
Operating Systems Course  

---

## 📄 License

This project is part of an academic assignment for educational purposes.

---

## 🙏 Acknowledgments

- **Instructor:** Dr. Muhammad Arif Butt for comprehensive assignment design
- **POSIX Documentation:** For system call references
- **GNU C Library:** For standard library functions
- **Unix/Linux Community:** For inspiration from the standard `ls` utility

---

## 📞 Contact

For questions or discussions about this project:
- GitHub: [@munib123](https://github.com/munib123)
- Repository: [BSDSF23M040-OS-A02](https://github.com/munib123/BSDSF23M040-OS-A02)

---

**Project Status:** ✅ Complete and Ready for Submission

*Last Updated: October 13, 2025*
