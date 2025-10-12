# Viva-Voce Preparation Guide
## Operating Systems Lab Assignment - ls Command Implementation

---

## Quick Reference for Viva

### Project Overview
- **Total Versions:** 7 (v1.0.0 through v1.6.0)
- **Total Lines of Code:** ~250 lines in final version
- **Programming Language:** C (POSIX-compliant)
- **Version Control:** Git with feature branch workflow
- **Repository:** github.com/munib123/BSDSF23M040-OS-A02

---

## Feature Summary (Be Ready to Explain Each)

### v1.0.0 - Basic Listing
- **Purpose:** Simple directory listing (unsorted, no formatting)
- **Key Functions:** `opendir()`, `readdir()`, `closedir()`
- **What You Learned:** Basic directory operations

### v1.1.0 - Long Listing Format (-l)
- **New Flag:** `-l` for detailed file information
- **Key System Calls:** `stat()`, `lstat()`, `getpwuid()`, `getgrgid()`
- **Displays:** Permissions, links, owner, group, size, timestamp
- **Key Insight:** Difference between `stat()` and `lstat()` (symbolic links)

### v1.2.0 - Column Display
- **Feature:** "Down then across" columnar layout
- **Key Technique:** Terminal width detection with `ioctl(TIOCGWINSZ)`
- **Algorithm:** Column-major order indexing: `idx = col * num_rows + row`
- **Challenge:** Complex index calculation for proper layout

### v1.3.0 - Horizontal Display (-x)
- **New Flag:** `-x` for horizontal (row-major) layout
- **Key Difference:** Simpler than vertical - just track horizontal position
- **Implementation:** Sequential iteration with line wrapping
- **Design Pattern:** Flag-based conditional dispatch

### v1.4.0 - Alphabetical Sort
- **Feature:** Sort all entries alphabetically
- **Key Function:** `qsort()` with custom comparison function
- **Algorithm:** O(n log n) sorting
- **Key Concept:** Function pointers, `void *` casting
- **Trade-off:** Must load all entries into memory before sorting

### v1.5.0 - Colorized Output
- **Feature:** Color-coded file types using ANSI escape codes
- **Colors Implemented:**
  - Blue: Directories
  - Green: Executables (check execute permission bits)
  - Red: Archives (.tar, .gz, .zip)
  - Pink: Symbolic links
  - Reverse video: Special files
- **Key Technique:** ANSI escape codes (`\033[0;32m` for green)
- **Permission Check:** `st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)`

### v1.6.0 - Recursive Listing (-R)
- **New Flag:** `-R` for recursive directory traversal
- **Key Concept:** Recursion with proper base cases
- **Implementation:** 
  - Print directory header
  - List contents
  - Recurse into subdirectories
- **Base Cases:** 
  - Skip "." and ".."
  - Only recurse on directories
  - Stop when no subdirectories found
- **Critical Detail:** Must construct full path before recursive call

---

## Common Viva Questions and Answers

### Technical Questions

#### Q: What's the difference between stat() and lstat()?
**Answer:** `stat()` follows symbolic links and returns info about the target. `lstat()` does NOT follow symlinks and returns info about the link itself. We use `lstat()` to properly detect and display symbolic links.

#### Q: How do you detect if a file is executable?
**Answer:** Check the execute permission bits in `st_mode`:
```c
if (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
```
This checks if ANY user (owner, group, or others) can execute the file.

#### Q: Why must you read all files into memory before sorting?
**Answer:** `readdir()` returns entries in unpredictable order. To sort, we need to compare ALL items, requiring random access. We can't sort what we haven't seen yet.

#### Q: Explain the qsort() comparison function.
**Answer:** 
```c
int compare(const void *a, const void *b) {
    const char **str_a = (const char **)a;
    const char **str_b = (const char **)b;
    return strcmp(*str_a, *str_b);
}
```
- Takes `void *` for generic sorting
- We cast to `char **` (pointer to string pointer)
- Dereference to get actual strings
- Return negative if a<b, 0 if equal, positive if a>b

#### Q: How do ANSI color codes work?
**Answer:** They're escape sequences that terminals interpret:
- Format: `\033[<code>m<text>\033[0m`
- Example: `\033[0;32m` = green, `\033[0m` = reset
- Must always reset after colored text

#### Q: What's the "down then across" algorithm?
**Answer:** Column-major ordering where you read down each column:
```c
for (row = 0; row < num_rows; row++) {
    for (col = 0; col < num_cols; col++) {
        idx = col * num_rows + row;  // Key formula
        print(files[idx]);
    }
}
```

#### Q: What are the base cases in recursive ls?
**Answer:**
1. Skip "." and ".." to avoid infinite loops
2. Only recurse on directories (files are leaf nodes)
3. When no subdirectories exist, loop ends naturally

#### Q: Why construct full paths in recursion?
**Answer:** `opendir()` needs path from current working directory. If we just pass "subdir", it won't find it because cwd hasn't changed. We must build: `parent_path/subdir`.

---

### Design Questions

#### Q: Why use feature branches?
**Answer:** 
- Isolate development of each feature
- Easy to review individual features
- Can merge when feature is complete and tested
- Maintains clean history of project evolution

#### Q: How did you manage different display modes?
**Answer:** Used flag variables (`long_format`, `horizontal`, `recursive`) set by `getopt()`, then conditional logic to call appropriate display function.

#### Q: What would you improve?
**Answer:**
- Add `-a` flag for hidden files
- Add `-h` for human-readable file sizes
- Implement `-t` for time-based sorting
- Handle very large directories more efficiently
- Add color customization options
- Better error handling for edge cases

---

### Problem-Solving Questions

#### Q: What was the most challenging feature?
**Answer:** [Choose one and explain - probably v1.2.0 column display or v1.6.0 recursion]

**Example:** "The column display (v1.2.0) was challenging because I had to understand the mathematical relationship between row/column coordinates and array indices. The formula `idx = col * num_rows + row` wasn't intuitive at first, but once I drew it out on paper, I understood how it maps 2D positions to 1D array indices in column-major order."

#### Q: How did you debug issues?
**Answer:**
- Used `printf()` debugging to trace execution
- Created test directories with known structures
- Compared output with standard `ls` command
- Used `gcc -Wall` to catch warnings
- Tested edge cases (empty directories, single files, deep nesting)

#### Q: What system calls did you learn?
**Answer:**
- `opendir()`, `readdir()`, `closedir()` - Directory operations
- `stat()`, `lstat()` - File metadata
- `getpwuid()`, `getgrgid()` - User/group name lookup
- `ioctl()` - Terminal control
- `getopt()` - Command-line parsing

---

## Code Snippets to Remember

### Opening and Reading Directory
```c
DIR *dir = opendir(dirname);
if (dir == NULL) { perror("opendir"); exit(1); }

struct dirent *entry;
while ((entry = readdir(dir)) != NULL) {
    if (entry->d_name[0] == '.') continue;  // Skip hidden
    // Process entry
}
closedir(dir);
```

### Getting File Info
```c
struct stat st;
lstat(path, &st);

// Check file type
if (S_ISDIR(st.st_mode))  // Directory
if (S_ISLNK(st.st_mode))  // Symbolic link

// Check permissions
if (st.st_mode & S_IXUSR)  // Owner can execute
```

### Sorting with qsort
```c
char **files = malloc(capacity * sizeof(char*));
// ... read files ...
qsort(files, count, sizeof(char*), compare);

int compare(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}
```

### Colorized Output
```c
#define COLOR_GREEN "\033[0;32m"
#define COLOR_RESET "\033[0m"

printf("%s%s%s ", COLOR_GREEN, filename, COLOR_RESET);
```

### Recursive Directory Traversal
```c
void do_ls(char *dirname, int recursive) {
    printf("%s:\n", dirname);  // Header
    // ... list contents ...
    
    if (recursive) {
        for (int i = 0; i < count; i++) {
            char path[1024];
            snprintf(path, sizeof(path), "%s/%s", dirname, files[i]);
            struct stat st;
            lstat(path, &st);
            if (S_ISDIR(st.st_mode) && 
                strcmp(files[i], ".") != 0 && 
                strcmp(files[i], "..") != 0) {
                do_ls(path, recursive);
            }
        }
    }
}
```

---

## Git Commands Used

```bash
# Create feature branch
git checkout -b feature-name-v1.x.0

# Commit changes
git add .
git commit -m "feat: Description"

# Merge to main
git checkout main
git merge --no-ff feature-name-v1.x.0

# Create tag
git tag -a v1.x.0 -m "Release v1.x.0: Description"

# Push to remote
git push origin main --tags
git push origin feature-name-v1.x.0
```

---

## Project Statistics

- **Total Commits:** 15+
- **Total Tags:** 6 (v1.1.0 through v1.6.0)
- **Feature Branches:** 6
- **Lines in REPORT.md:** 1193
- **Programming Concepts:** 15+
- **System Calls Used:** 10+

---

## Final Checklist ✓

- [x] All 7 features implemented
- [x] All features tested and working
- [x] All report questions answered
- [x] All feature branches pushed to GitHub
- [x] All tags created and pushed
- [x] Main branch contains final code
- [x] REPORT.md is complete
- [x] Code is well-commented
- [x] Git history is clean and organized

---

## Viva Tips

1. **Be Confident:** You built this entire project - you know it!
2. **Use Examples:** Refer to specific parts of your code
3. **Explain Trade-offs:** Show you understand pros/cons of design choices
4. **Be Honest:** If you don't know something, say so and explain how you'd find out
5. **Show Understanding:** Don't just recite - explain WHY things work that way
6. **Draw Diagrams:** Visual explanations help (especially for column layout)
7. **Relate to Real ls:** Compare your implementation to standard Unix ls
8. **Discuss Improvements:** Show you're thinking beyond the assignment

---

## Quick Demo Commands

```bash
# Basic listing
./bin/ls

# Long format
./bin/ls -l

# Horizontal format
./bin/ls -x

# Recursive
./bin/ls -R

# Combined
./bin/ls -lR
./bin/ls -xR

# Specific directory
./bin/ls -R /usr/bin
```

---

**Good luck with your viva-voce!** 

You've built a sophisticated implementation of a core Unix utility, demonstrating mastery of systems programming, C, algorithms, and professional development practices.

Remember: You understand this code because you wrote it. Trust your knowledge!
