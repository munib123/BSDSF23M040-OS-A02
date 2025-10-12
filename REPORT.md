# Operating Systems Lab Assignment - ls Command Implementation
## BSDSF23M040-OS-A02

**Student Name:** Muneeb  
**Course:** Operating Systems  
**Instructor:** Muhammad Arif Butt, PhD  
**Date:** October 13, 2025

---

## Table of Contents
1. [Feature-2: ls-v1.1.0 - Long Listing Format](#feature-2)
2. [Feature-3: ls-v1.2.0 - Column Display](#feature-3)
3. [Feature-4: ls-v1.3.0 - Horizontal Display](#feature-4)
4. [Feature-5: ls-v1.4.0 - Alphabetical Sort](#feature-5)
5. [Feature-6: ls-v1.5.0 - Colorized Output](#feature-6)
6. [Feature-7: ls-v1.6.0 - Recursive Listing](#feature-7)
7. [Project Summary](#summary)

---

## Feature-2: ls-v1.1.0 - Complete Long Listing Format {#feature-2}

### Report Questions:

#### Q1: What is the difference between the stat() and lstat() system calls?

**Answer:**

Both `stat()` and `lstat()` are used to retrieve file metadata, but they differ in how they handle symbolic links:

- **`stat()`**: Follows symbolic links. When you call `stat()` on a symbolic link, it returns information about the file or directory the link points to, not the link itself.

- **`lstat()`**: Does NOT follow symbolic links. When you call `lstat()` on a symbolic link, it returns information about the symbolic link itself (such as its size, permissions, and modification time).

**Example:**
```c
struct stat st;
lstat("mylink", &st);  // Returns info about the link itself
stat("mylink", &st);   // Returns info about the target file
```

**Why this matters for ls:**
The real `ls` command needs to show symbolic links as separate entries with their own properties. Using `lstat()` allows us to detect that a file is a symbolic link using `S_ISLNK(st.st_mode)` and display it appropriately (often in a different color).

---

#### Q2: The st_mode field contains both file type and permission bits. How can you extract just the permission bits?

**Answer:**

The `st_mode` field is a bit field that contains:
- **File type bits** (upper bits): Indicate if the file is a regular file, directory, symbolic link, etc.
- **Permission bits** (lower 9 bits): The read, write, and execute permissions for owner, group, and others.

To extract just the permission bits, we use a bitwise AND operation with the mask `0777` (octal), which preserves only the lower 9 bits:

```c
mode_t permissions = st.st_mode & 0777;
```

**Breakdown of the 9 permission bits:**
- Bits 0-2 (001, 002, 004): Others' execute, write, read
- Bits 3-5 (010, 020, 040): Group's execute, write, read  
- Bits 6-8 (0100, 0200, 0400): Owner's execute, write, read

**In my implementation:**
```c
void mode_to_str(mode_t mode, char *str) {
    strcpy(str, "----------");
    if (S_ISDIR(mode)) str[0] = 'd';
    else if (S_ISLNK(mode)) str[0] = 'l';
    // ... other file types ...
    
    // Extract permission bits using bitwise AND
    if (mode & S_IRUSR) str[1] = 'r';  // Owner read (0400)
    if (mode & S_IWUSR) str[2] = 'w';  // Owner write (0200)
    if (mode & S_IXUSR) str[3] = 'x';  // Owner execute (0100)
    // ... group and others ...
}
```

---

#### Q3: Explain the role of the getpwuid() and getgrgid() functions.

**Answer:**

These functions convert numeric user and group IDs to human-readable names:

**`getpwuid(uid_t uid)`:**
- Takes a user ID (UID) as input
- Returns a pointer to a `struct passwd` containing user account information
- The `pw_name` field contains the username string
- Used to display the owner's name instead of just their numeric UID

**`getgrgid(gid_t gid)`:**
- Takes a group ID (GID) as input
- Returns a pointer to a `struct group` containing group information
- The `gr_name` field contains the group name string
- Used to display the group name instead of just their numeric GID

**Example from my code:**
```c
struct stat st;
lstat(path, &st);

struct passwd *pwd = getpwuid(st.st_uid);
struct group *grp = getgrgid(st.st_gid);

printf("%s %s ", 
    pwd ? pwd->pw_name : "unknown",
    grp ? grp->gr_name : "unknown");
```

**Why this is important:**
- Makes the output human-readable (e.g., "muneeb" instead of "1000")
- Matches the behavior of the standard `ls -l` command
- Handles cases where the UID/GID might not exist (returns NULL), so we check for NULL and display "unknown" as a fallback

---

## Feature-3: ls-v1.2.0 - Column Display (Down Then Across) {#feature-3}

### Report Questions:

#### Q1: Explain the general logic for printing items in a "down then across" columnar format. Why is a simple single loop through the list of filenames insufficient?

**Answer:**

**The "Down Then Across" Logic:**

The "down then across" format (also called column-major order) organizes items into columns where you read downward in each column before moving to the next column. This is the default behavior of the real `ls` command.

**Why a simple loop is insufficient:**

A simple loop would give us "across then down" (row-major) order:
```
file1  file2  file3  file4
file5  file6  file7  file8
```

But we need "down then across" (column-major) order:
```
file1  file3  file5  file7
file2  file4  file6  file8
```

**The Implementation Logic:**

1. **Calculate layout parameters:**
   ```c
   int num_cols = term_width / col_width;
   int num_rows = (count + num_cols - 1) / num_cols;  // Ceiling division
   ```

2. **Iterate row by row:**
   ```c
   for (int row = 0; row < num_rows; row++) {
       for (int col = 0; col < num_cols; col++) {
           int idx = col * num_rows + row;  // KEY FORMULA
           if (idx < count) {
               printf("%-*s", col_width, files[idx]);
           }
       }
       printf("\n");
   }
   ```

3. **The key formula: `idx = col * num_rows + row`**
   - This formula maps 2D (row, col) coordinates to 1D array indices
   - It ensures we access items in column-major order
   - For column 0: indices 0, 1, 2, ... (num_rows-1)
   - For column 1: indices num_rows, num_rows+1, ... (2*num_rows-1)
   - And so on...

**Example with 8 files and 4 columns (2 rows):**
```
Row 0: files[0*2+0]=files[0], files[1*2+0]=files[2], files[2*2+0]=files[4], files[3*2+0]=files[6]
Row 1: files[0*2+1]=files[1], files[1*2+1]=files[3], files[2*2+1]=files[5], files[3*2+1]=files[7]
```

This produces the correct "down then across" layout.

---

#### Q2: What is the purpose of the ioctl system call in this context? What would be the limitations if you only used a fixed-width fallback?

**Answer:**

**Purpose of `ioctl()` for terminal width detection:**

The `ioctl()` system call with `TIOCGWINSZ` command retrieves the current terminal window size:

```c
struct winsize w;
ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
int term_width = w.ws_col;  // Number of columns
```

**Why we need dynamic terminal width:**

1. **Adaptability:** Users can resize their terminal windows, and `ls` should adapt to display the optimal number of columns
2. **User Experience:** Maximizes screen real estate usage regardless of terminal size
3. **Correctness:** Prevents line wrapping issues that would break the columnar layout

**Limitations of a fixed-width fallback (e.g., 80 columns):**

1. **Wasted Space on Large Terminals:**
   - Modern terminals can be 200+ columns wide
   - Using only 80 columns leaves most of the screen blank
   - Fewer items visible at once, requiring more scrolling

2. **Broken Layout on Small Terminals:**
   - Mobile terminals or split-screen setups might be only 40-60 columns
   - Lines would wrap, destroying the columnar alignment
   - Output becomes unreadable and confusing

3. **Poor User Experience:**
   - Users expect programs to respect their terminal settings
   - Fixed width feels outdated and inflexible
   - Doesn't match the behavior of standard Unix tools

**My implementation:**
```c
struct winsize w;
ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
int term_width = w.ws_col;
if (term_width <= 0) term_width = 80;  // Fallback only if ioctl fails
```

This provides the best of both worlds: dynamic sizing when possible, with a reasonable fallback for edge cases.

---

## Feature-4: ls-v1.3.0 - Horizontal Column Display (-x Option) {#feature-4}

### Report Questions:

#### Q1: Compare the implementation complexity of "down then across" versus "across" (horizontal) printing logic. Which one requires more pre-calculation and why?

**Answer:**

**"Down Then Across" (Vertical/Column-Major) - More Complex:**

This method requires significant pre-calculation:

1. **Calculate number of rows and columns:**
   ```c
   int num_cols = term_width / col_width;
   int num_rows = (count + num_cols - 1) / num_cols;
   ```

2. **Complex index mapping:**
   ```c
   int idx = col * num_rows + row;  // 2D to 1D conversion
   ```

3. **Must complete layout calculation before printing:**
   - Need to know total number of rows
   - Must iterate in a specific order (row by row, but accessing column by column)
   - Requires understanding the entire layout structure upfront

4. **More complex loop structure:**
   ```c
   for (int row = 0; row < num_rows; row++) {
       for (int col = 0; col < num_cols; col++) {
           int idx = col * num_rows + row;  // Complex calculation
           if (idx < count) print_file(idx);
       }
   }
   ```

**"Across" (Horizontal/Row-Major) - Simpler:**

This method requires minimal pre-calculation:

1. **Only need column width:**
   ```c
   int col_width = max_filename_length + 2;
   ```

2. **Simple sequential access:**
   ```c
   int idx = 0;  // Just iterate sequentially
   ```

3. **Can print as you go:**
   - Don't need to know total rows beforehand
   - Just track current position on line
   - Add newline when line would overflow

4. **Simple loop structure:**
   ```c
   int current_position = 0;
   for (int i = 0; i < count; i++) {
       if (current_position + col_width > term_width && current_position > 0) {
           printf("\n");
           current_position = 0;
       }
       print_file(i);
       current_position += col_width;
   }
   ```

**Summary:**

- **"Down Then Across" requires more pre-calculation** because you need to:
  - Determine the complete grid structure (rows × columns)
  - Use complex indexing to access the linear array in column-major order
  - Plan the entire layout before printing anything

- **"Across" is simpler** because you:
  - Just iterate through the array sequentially
  - Track horizontal position and wrap when needed
  - No need to pre-calculate the grid structure

The vertical layout is more complex but provides better visual organization for users scanning a list.

---

#### Q2: Describe the strategy you used in your code to manage different display modes (-l, -x, and default). How did your program decide which function to call?

**Answer:**

**Strategy: Flag-Based Conditional Dispatch**

I used simple integer flag variables to track which display mode was selected:

```c
int long_format = 0;   // Set to 1 when -l is present
int horizontal = 0;    // Set to 1 when -x is present
int recursive = 0;     // Set to 1 when -R is present (v1.6.0)
```

**Argument Parsing Phase:**
```c
while ((opt = getopt(argc, argv, "lxR")) != -1) {
    switch (opt) {
        case 'l':
            long_format = 1;
            break;
        case 'x':
            horizontal = 1;
            break;
        case 'R':
            recursive = 1;
            break;
    }
}
```

**Display Mode Selection Logic:**

The `do_ls()` function uses nested conditionals to decide which display function to call:

```c
void do_ls(char *dirname, int long_format, int horizontal, int recursive) {
    // ... read and sort files ...
    
    if (long_format) {
        // Long listing format (-l)
        for (int i = 0; i < count; i++) {
            // Print detailed file information
            print_long_format(dirname, files[i]);
        }
    } else {
        if (horizontal) {
            // Horizontal display (-x)
            print_horizontal(dirname, files, count);
        } else {
            // Default: "down then across" vertical columns
            print_vertical_columns(dirname, files, count);
        }
    }
    
    // Handle recursion if -R flag is set
    if (recursive) {
        for (int i = 0; i < count; i++) {
            if (is_directory(files[i])) {
                do_ls(subdirectory_path, long_format, horizontal, recursive);
            }
        }
    }
}
```

**Decision Priority:**

1. **Long format takes precedence:** If `-l` is specified, it overrides the column layout choice
2. **Then check horizontal vs vertical:** If not long format, check for `-x`
3. **Default to vertical:** If no display flags, use "down then across" columns

**Advantages of this approach:**

1. **Clear and maintainable:** Easy to understand and modify
2. **Extensible:** Easy to add new display modes (just add another flag)
3. **No complex state machine:** Simple boolean logic
4. **Flags are composable:** Can combine `-l` with `-R`, `-x` with `-R`, etc.

**Example combinations:**
- `ls` → vertical columns
- `ls -l` → long format
- `ls -x` → horizontal columns
- `ls -lR` → recursive long format
- `ls -xR` → recursive horizontal columns

This design makes the code modular and easy to extend with additional features.

---

## Feature-5: ls-v1.4.0 - Alphabetical Sort {#feature-5}

### Report Questions:

#### Q1: Why is it necessary to read all directory entries into memory before you can sort them? What are the potential drawbacks for directories with millions of files?

**Answer:**

**Why we must read all entries into memory:**

1. **Sorting requires complete data:**
   - To determine the correct alphabetical order, we need to compare ALL entries
   - You can't sort what you haven't seen yet
   - Sorting algorithms (like quicksort) need random access to all elements

2. **`readdir()` is sequential:**
   - The `readdir()` function returns directory entries in an unpredictable order (filesystem-dependent)
   - There's no way to "seek" to a specific entry or read them in sorted order
   - We must buffer all entries before sorting

3. **Comparison requirements:**
   - To place an item in its correct sorted position, we must compare it with other items
   - This requires having all items accessible in memory simultaneously

**Potential drawbacks for large directories (millions of files):**

1. **Memory Consumption:**
   ```c
   // For 1 million files with average filename length of 30 chars:
   // Array of pointers: 1M × 8 bytes = 8 MB
   // String data: 1M × 30 bytes = 30 MB
   // Total: ~38 MB just for filenames
   ```
   - Can exhaust available RAM on constrained systems
   - May cause swapping to disk, degrading performance
   - Could fail with out-of-memory errors

2. **Performance Impact:**
   - **Reading phase:** Must read all entries before displaying anything (long delay)
   - **Sorting phase:** O(n log n) comparisons - for 1M files: ~20M string comparisons
   - **String comparisons are expensive:** Each strcmp() must compare potentially long strings
   - Total time could be several seconds or minutes

3. **User Experience:**
   - Long delay before any output appears (no streaming output)
   - No feedback during processing (appears hung)
   - Unusable for very large directories

4. **Resource Starvation:**
   - Could impact system performance for other processes
   - Risk of being killed by OOM (Out-Of-Memory) killer on Linux

**Better alternatives for huge directories:**

1. **Unsorted output:** Just display entries as read (what we did in v1.0.0)
2. **External sorting:** Sort in batches, merge sorted chunks
3. **Streaming with partial sorting:** Display in chunks, each sorted
4. **Database-backed:** Use a proper filesystem database with indices

**My implementation:**
```c
char **files = malloc(capacity * sizeof(char*));
while ((entry = readdir(dir)) != NULL) {
    if (count >= capacity) {
        capacity *= 2;
        files = realloc(files, capacity * sizeof(char*));
    }
    files[count] = strdup(entry->d_name);  // Allocate memory for each name
    count++;
}
qsort(files, count, sizeof(char*), compare);
```

This works well for typical directories (up to thousands of files) but would struggle with extreme cases.

---

#### Q2: Explain the purpose and signature of the comparison function required by qsort(). How does it work, and why must it take const void * arguments?

**Answer:**

**Purpose of the comparison function:**

The comparison function tells `qsort()` how to order two elements. It must return:
- **Negative value** if first element should come before second (a < b)
- **Zero** if elements are equal (a == b)
- **Positive value** if first element should come after second (a > b)

**Required signature:**
```c
int compare(const void *a, const void *b);
```

**Why `const void *` arguments?**

1. **Generic sorting algorithm:**
   - `qsort()` is designed to sort ANY data type (integers, strings, structs, etc.)
   - It doesn't know at compile-time what type it's sorting
   - `void *` is a generic pointer type that can point to anything

2. **Type safety through casting:**
   - Inside the comparison function, we cast the `void *` to the actual type
   - This allows the same `qsort()` function to work with any data type

3. **const for safety:**
   - The `const` qualifier promises we won't modify the data during comparison
   - Only reading/comparing, never modifying
   - Prevents accidental corruption of data during sorting

**My implementation for sorting filenames:**

```c
int compare(const void *a, const void *b) {
    // Cast void* to char** (pointer to pointer to char)
    // because our array is: char *files[]
    // So a and b point to elements of files[], which are char*
    const char **str_a = (const char **)a;
    const char **str_b = (const char **)b;
    
    // Dereference to get the actual strings and compare
    return strcmp(*str_a, *str_b);
}
```

**Step-by-step explanation:**

1. **Input:** `qsort()` passes pointers to array elements
   - Our array is `char *files[]` (array of string pointers)
   - So `qsort` passes pointers to those string pointers: `char **`

2. **Casting:** We cast `const void *` → `const char **`
   ```c
   const char **str_a = (const char **)a;
   ```

3. **Dereferencing:** We dereference to get the actual string pointers
   ```c
   *str_a  // This gives us the actual string (char *)
   ```

4. **Comparison:** `strcmp()` compares the strings lexicographically
   ```c
   strcmp(*str_a, *str_b);  // Returns <0, 0, or >0
   ```

**Visual representation:**
```
files array:        ["zebra", "apple", "mango"]
                       ↑        ↑        ↑
                      str1     str2     str3
                       
qsort passes:      &files[0], &files[1] (addresses of pointers)
                       ↓          ↓
compare receives:  void *a,   void *b
                       ↓          ↓
Cast to:           char **str_a, char **str_b
                       ↓          ↓
Dereference:       *str_a = "zebra", *str_b = "apple"
                       ↓          ↓
Compare:           strcmp("zebra", "apple") = positive (z > a)
```

**Why this design is brilliant:**

- **One `qsort()` implementation** works for all data types
- **Type-safe** through explicit casting in the comparison function
- **Efficient** - no data copying, just pointer manipulation
- **Flexible** - can sort by any criteria (reverse order, case-insensitive, etc.)

---

## Feature-6: ls-v1.5.0 - Colorized Output Based on File Type {#feature-6}

### Report Questions:

#### Q1: How do ANSI escape codes work to produce color in a standard Linux terminal? Show the specific code sequence for printing text in green.

**Answer:**

**How ANSI Escape Codes Work:**

ANSI escape codes are special character sequences that terminals interpret as commands rather than text to display. They begin with the escape character (`\033` in octal or `\x1b` in hex) followed by control sequences.

**General format:**
```
\033[<attribute>m<text>\033[0m
```
- `\033[` - Escape sequence introducer (ESC + '[')
- `<attribute>` - Color or style code
- `m` - End of escape sequence
- `<text>` - The actual text to display
- `\033[0m` - Reset to default colors

**Common ANSI Color Codes:**

| Code | Color | Type |
|------|-------|------|
| 30 | Black | Foreground |
| 31 | Red | Foreground |
| 32 | Green | Foreground |
| 33 | Yellow | Foreground |
| 34 | Blue | Foreground |
| 35 | Magenta/Pink | Foreground |
| 36 | Cyan | Foreground |
| 37 | White | Foreground |
| 0 | Reset | All attributes |
| 7 | Reverse | Swap fg/bg |

**Specific code sequence for green text:**

```c
printf("\033[0;32m%s\033[0m", "This text is green");
```

Breaking it down:
- `\033` - ESC character (ASCII 27)
- `[` - Begin control sequence
- `0` - Normal intensity (vs. 1 for bold/bright)
- `;` - Separator
- `32` - Green foreground color
- `m` - End of escape sequence
- `%s` - The text to print in green
- `\033[0m` - Reset to default (IMPORTANT!)

**My implementation:**

```c
// Define color macros for readability
#define COLOR_RESET "\033[0m"
#define COLOR_BLUE "\033[0;34m"      // Directories
#define COLOR_GREEN "\033[0;32m"     // Executables
#define COLOR_RED "\033[0;31m"       // Archives
#define COLOR_PINK "\033[0;35m"      // Symbolic links
#define COLOR_REVERSE "\033[7m"      // Special files

// Usage example:
void print_colored(char *dirname, char *name) {
    char *color = COLOR_RESET;
    
    // Determine file type and set color
    if (S_ISDIR(st.st_mode)) 
        color = COLOR_BLUE;
    else if (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) 
        color = COLOR_GREEN;  // Executable
    
    // Print with color
    printf("%s%s%s", color, name, COLOR_RESET);
}
```

**Why the reset is crucial:**

Without `\033[0m` at the end, the color would "bleed" into subsequent text:
```c
// BAD - no reset:
printf("\033[0;32m%s", "file1");
printf(" file2");  // This will ALSO be green!

// GOOD - with reset:
printf("\033[0;32m%s\033[0m", "file1");
printf(" file2");  // This will be default color
```

**Advanced features I could add:**

- Bold colors: `\033[1;32m` (bright green)
- Background colors: `\033[42m` (green background)
- Combined: `\033[1;32;44m` (bold green text on blue background)

**Terminal compatibility:**

- Most modern terminals support ANSI colors (xterm, gnome-terminal, konsole, etc.)
- Some very old terminals or non-ANSI terminals won't display colors correctly
- The `ls` command checks if output is a terminal (`isatty()`) and disables colors for pipes/redirects

---

#### Q2: To color an executable file, you need to check its permission bits. Explain which bits in the st_mode field you need to check to determine if a file is executable.

**Answer:**

**The Execute Permission Bits in st_mode:**

The `st_mode` field contains permission bits for three categories of users:
- **Owner (User):** The file's owner
- **Group:** Users in the file's group
- **Others:** Everyone else

Each category has three permission bits:
- **Read (r):** Can read file contents
- **Write (w):** Can modify file contents  
- **Execute (x):** Can run file as a program

**Execute bit positions and masks:**

```c
#define S_IXUSR 0100  // Owner execute (octal 0100 = binary 001 000 000)
#define S_IXGRP 0010  // Group execute (octal 0010 = binary 000 001 000)
#define S_IXOTH 0001  // Other execute (octal 0001 = binary 000 000 001)
```

**Bit layout of st_mode (simplified):**
```
Bit position: 9 8 7 6 5 4 3 2 1 0
              r w x r w x r w x
              └─┬─┘ └─┬─┘ └─┬─┘
              Owner Group Other
```

**Three approaches to checking executability:**

**1. Check any execute bit (most common for ls):**
```c
if (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
    // File is executable by SOMEONE
    color = COLOR_GREEN;
}
```
This is what I use in my implementation. It colors the file green if ANY user can execute it.

**2. Check only owner execute bit:**
```c
if (st.st_mode & S_IXUSR) {
    // File is executable by owner
    color = COLOR_GREEN;
}
```
More restrictive - only colors if the owner can execute.

**3. Check if current user can execute:**
```c
if (access(path, X_OK) == 0) {
    // Current user can execute this file
    color = COLOR_GREEN;
}
```
Most accurate for the current user, but slower (requires system call).

**My implementation:**

```c
void print_colored(char *dirname, char *name) {
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", dirname, name);
    
    struct stat st;
    lstat(path, &st);
    
    char *color = COLOR_RESET;
    
    // Check file type first
    if (S_ISDIR(st.st_mode)) {
        color = COLOR_BLUE;  // Directories
    }
    // Then check for any execute permission
    else if (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
        color = COLOR_GREEN;  // Executables
    }
    // Check for archives
    else if (strstr(name, ".tar") || strstr(name, ".gz") || 
             strstr(name, ".zip")) {
        color = COLOR_RED;    // Archives
    }
    
    printf("%s%s%s", color, name, COLOR_RESET);
}
```

**Why check all three execute bits?**

The standard `ls` command colors a file green if it's executable by ANY user. This makes sense because:
1. It indicates the file is a program or script, regardless of who can run it
2. Consistent with the `-F` flag behavior (adds `*` to executables)
3. More useful for visual identification than checking only your own permissions

**Important distinction:**
- **`S_IXUSR | S_IXGRP | S_IXOTH`** creates a bitmask: `0111` (all execute bits)
- **Bitwise OR (`|`)** combines the masks
- **Bitwise AND (`&`)** checks if any bit is set
- Returns true if file is executable by owner, group, OR others

**Example permission analysis:**
```c
// File permissions: rwxr-xr-- (0754)
// Binary: 111 101 100
//          rwx r-x r--

st.st_mode & S_IXUSR  // True  (owner can execute)
st.st_mode & S_IXGRP  // True  (group can execute)
st.st_mode & S_IXOTH  // False (others cannot execute)

st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)  // True (at least one can execute)
```

This accurately identifies the file as executable and colors it green.

---

## Feature-7: ls-v1.6.0 - Recursive Listing (-R Option) {#feature-7}

### Report Questions:

#### Q1: In a recursive function, what is a "base case"? In the context of your recursive ls, what is the base case that stops recursion from continuing forever?

**Answer:**

**What is a Base Case?**

A **base case** is a condition in a recursive function that stops the recursion by returning without making another recursive call. Without a base case, the function would call itself infinitely, leading to a stack overflow.

Every recursive function needs:
1. **Base case(s):** Conditions that stop recursion
2. **Recursive case(s):** Conditions that make recursive calls with modified parameters

**Classic example - Factorial:**
```c
int factorial(int n) {
    if (n <= 1) return 1;         // BASE CASE - stops recursion
    return n * factorial(n - 1);  // RECURSIVE CASE - calls itself
}
```

**Base Cases in Recursive ls:**

My recursive `ls` implementation has multiple implicit and explicit base cases:

**1. Implicit Base Case - No more subdirectories:**
```c
if (recursive) {
    for (int i = 0; i < count; i++) {
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dirname, files[i]);
        struct stat st;
        lstat(path, &st);
        
        // Recursive call ONLY if entry is a directory
        if (S_ISDIR(st.st_mode) && 
            strcmp(files[i], ".") != 0 && 
            strcmp(files[i], "..") != 0) {
            do_ls(path, long_format, horizontal, recursive);
        }
        // If not a directory, no recursive call - implicit base case
    }
}
```

When `do_ls()` encounters a directory with no subdirectories (only files), the loop completes without making any recursive calls. This is the **implicit base case**.

**2. Explicit Base Case - Skip special directories:**
```c
if (strcmp(files[i], ".") != 0 && strcmp(files[i], "..") != 0)
```

This prevents infinite recursion by:
- **Skipping "."** - Current directory (would recurse into same directory forever)
- **Skipping ".."** - Parent directory (would bounce between parent and child)

Without this check:
```
ls -R /home
  → ls -R /home/.
    → ls -R /home/./.
      → ls -R /home/./././.
        → ... INFINITE RECURSION!
```

**3. Explicit Base Case - Check if path is a directory:**
```c
if (S_ISDIR(st.st_mode))
```

Only directories trigger recursion. When we encounter a regular file, symbolic link, or other non-directory, we don't recurse. This naturally terminates at leaf nodes (files).

**Complete recursion flow:**

```
do_ls("/home/user") {
    Print: /home/user:
    List files: file1.txt, dir1, dir2
    
    if (recursive) {
        // file1.txt - not a directory → BASE CASE (no recursion)
        
        // dir1 - is a directory → RECURSIVE CASE
        do_ls("/home/user/dir1") {
            Print: /home/user/dir1:
            List files: file2.txt, subdir1
            
            // file2.txt - not a directory → BASE CASE
            
            // subdir1 - is a directory → RECURSIVE CASE
            do_ls("/home/user/dir1/subdir1") {
                Print: /home/user/dir1/subdir1:
                List files: file3.txt
                
                // file3.txt - not a directory → BASE CASE
                // No subdirectories → BASE CASE (loop ends)
            }
        }
        
        // dir2 - is a directory → RECURSIVE CASE
        do_ls("/home/user/dir2") {
            // ... similar process ...
        }
    }
}
```

**Why these base cases prevent infinite recursion:**

1. **Finite directory tree:** Every real filesystem has a finite depth
2. **No cycles in proper filesystems:** (excluding symlinks, which we handle with `lstat`)
3. **Skip "." and ".."** prevents cycling back
4. **Only recurse on directories:** Eventually reach files (leaf nodes)
5. **No subdirectories:** When a directory is empty or contains only files, recursion stops

**Stack depth considerations:**

Each recursive call uses stack memory. For a very deep directory tree (e.g., 1000 levels), we could theoretically overflow the stack. However:
- Real filesystems rarely exceed 20-30 levels of nesting
- Linux has generous default stack sizes (8MB)
- Each `do_ls()` call uses relatively little stack space

My implementation is safe for all practical directory structures.

---

#### Q2: Explain why it is essential to construct a full path before making a recursive call. What would happen if you simply called do_ls("subdir") from within do_ls("parent_dir")?

**Answer:**

**Why Full Path Construction is Essential:**

When making a recursive call to `do_ls()`, we must pass the complete absolute or relative path from the current working directory, not just the subdirectory name.

**The Problem with Partial Paths:**

If we only pass the subdirectory name without the full path:

```c
// WRONG - This will fail!
do_ls("parent_dir") {
    // Read entries, find "subdir"
    do_ls("subdir");  // ERROR: "subdir" doesn't exist from cwd!
}
```

**What happens:**
1. We call `do_ls("parent_dir")` from current working directory (e.g., `/home/user`)
2. Inside `do_ls`, we read directory entries and find `"subdir"`
3. We try to call `do_ls("subdir")`
4. **`opendir("subdir")` fails!** Because:
   - Current working directory is still `/home/user`
   - There is no directory `/home/user/subdir`
   - The subdirectory is actually at `/home/user/parent_dir/subdir`

**Error that would occur:**
```c
DIR *dir = opendir("subdir");
if (dir == NULL) { 
    perror("opendir");  // Would print: "No such file or directory"
    exit(1); 
}
```

**The Correct Approach - Full Path Construction:**

```c
do_ls("parent_dir") {
    for (int i = 0; i < count; i++) {
        // Construct FULL path by concatenating parent path + filename
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dirname, files[i]);
        //                            └─┬──┘   └──┬──┘
        //                          parent    subdir name
        //                          path
        
        struct stat st;
        lstat(path, &st);  // Use full path to stat the file
        
        if (S_ISDIR(st.st_mode) && strcmp(files[i], ".") != 0 
            && strcmp(files[i], "..") != 0) {
            do_ls(path, ...);  // Pass full path to recursive call
        }
    }
}
```

**Concrete Example:**

```
Directory structure:
/home/user/
  ├── test/
  │   ├── dir1/
  │   │   └── file1.txt
  │   └── dir2/
  │       └── file2.txt
  └── other_stuff/

Execution trace:
-----------------
Call: do_ls("test")
  dirname = "test"
  Read entries: ["dir1", "dir2"]
  
  For "dir1":
    path = "test" + "/" + "dir1" = "test/dir1"  ✓ Correct!
    do_ls("test/dir1")  // This will work
  
  For "dir2":
    path = "test" + "/" + "dir2" = "test/dir2"  ✓ Correct!
    do_ls("test/dir2")  // This will work

If we did it WRONG:
-------------------
Call: do_ls("test")
  dirname = "test"
  Read entries: ["dir1", "dir2"]
  
  For "dir1":
    do_ls("dir1")  // ✗ FAIL! No "dir1" in current directory
    opendir("dir1") → Error: No such file or directory
```

**My implementation:**

```c
if (recursive) {
    for (int i = 0; i < count; i++) {
        // Build full path from parent directory path + filename
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dirname, files[i]);
        
        struct stat st;
        lstat(path, &st);  // Need full path to stat
        
        if (S_ISDIR(st.st_mode) && 
            strcmp(files[i], ".") != 0 && 
            strcmp(files[i], "..") != 0) {
            printf("\n");
            do_ls(path, long_format, horizontal, recursive);  // Pass full path
        }
    }
}
```

**Additional considerations:**

**1. Path handling edge cases:**
```c
// Handle root directory properly
if (strcmp(dirname, "/") == 0) {
    snprintf(path, sizeof(path), "/%s", files[i]);
} else {
    snprintf(path, sizeof(path), "%s/%s", dirname, files[i]);
}
// This prevents double slashes: "//subdir"
```

**2. Buffer overflow protection:**
```c
char path[1024];  // Fixed size buffer
snprintf(path, sizeof(path), "%s/%s", dirname, files[i]);
// snprintf() prevents buffer overflow by limiting output to 1024 chars
// Very deep paths might get truncated, but won't crash
```

**3. Memory efficiency:**
```c
// Path is a local variable (stack-allocated)
// Automatically freed when function returns
// No need for malloc/free
```

**Why this design is correct:**

1. **Relative paths work:** Starting from any directory, paths are always relative to cwd
2. **Absolute paths work:** If initial path is absolute (e.g., `/home/user`), all recursive paths are absolute
3. **No chdir() needed:** We don't change the current working directory, avoiding state corruption
4. **Thread-safe:** No global state affected

**Performance note:**

Building path strings repeatedly might seem inefficient, but:
- String concatenation is very fast (a few CPU cycles)
- Alternative (chdir into each directory) would require system calls (much slower)
- Our approach is cleaner and more maintainable

This is a fundamental principle in recursive directory traversal: **always pass complete path information to recursive calls**.

---

## Project Summary {#summary}

### Version History and Features

| Version | Feature | Status |
|---------|---------|--------|
| v1.0.0 | Basic directory listing | ✅ Complete |
| v1.1.0 | Long listing format (-l) | ✅ Complete |
| v1.2.0 | Column display (down-then-across) | ✅ Complete |
| v1.3.0 | Horizontal display (-x) | ✅ Complete |
| v1.4.0 | Alphabetical sorting | ✅ Complete |
| v1.5.0 | Colorized output | ✅ Complete |
| v1.6.0 | Recursive listing (-R) | ✅ Complete |

### Git Repository Structure

**Branch History:**
```
main
├── feature-long-listing-v1.1.0 (merged)
├── feature-column-display-v1.2.0 (merged)
├── feature-horizontal-display-v1.3.0 (merged)
├── feature-alphabetical-sort-v1.4.0 (merged)
├── feature-colorized-output-v1.5.0 (merged)
└── feature-recursive-listing-v1.6.0 (merged)
```

**All tags created and pushed:**
- v1.1.0, v1.2.0, v1.3.0, v1.4.0, v1.5.0, v1.6.0

### Key Technical Achievements

1. **System Programming:**
   - Mastered Linux system calls: `opendir()`, `readdir()`, `stat()`, `lstat()`
   - File metadata handling and permission bit manipulation
   - Terminal control with `ioctl()` for dynamic width detection

2. **C Programming:**
   - Dynamic memory management with `malloc()`, `realloc()`, `free()`
   - String manipulation and path construction
   - Function pointers with `qsort()`
   - Proper error handling and edge cases

3. **Algorithms:**
   - Column layout calculations (row-major vs column-major)
   - Sorting with comparison functions
   - Recursive directory traversal

4. **Software Engineering:**
   - Feature-based branching workflow
   - Semantic versioning and tagging
   - Incremental development
   - Code modularity and maintainability

### Final Implementation Statistics

- **Total lines of code:** ~250 lines (v1.6.0)
- **Functions implemented:** 6 major functions
- **Command-line options:** 3 flags (-l, -x, -R)
- **Color schemes:** 5 file types colorized
- **Git commits:** 15+ across all features
- **Test coverage:** All features thoroughly tested

### Learning Outcomes

Through this project, I gained deep understanding of:
- Linux filesystem and directory structure
- POSIX system calls and APIs
- Terminal capabilities and ANSI escape codes
- Memory management in C
- Recursive algorithms
- Professional git workflow
- Software documentation practices

---

## Conclusion

This assignment provided comprehensive hands-on experience in systems programming, demonstrating how complex utilities like `ls` are built from fundamental system calls and algorithms. The incremental development approach taught proper software engineering practices, including version control, testing, and documentation.

The final implementation successfully replicates core functionality of the standard Unix `ls` command, handling edge cases, providing colorized output, and supporting recursive directory traversal - all while maintaining clean, readable, and maintainable code.

---

**End of Report**

*Submitted by: Muneeb*  
*Date: October 13, 2025*  
*Course: Operating Systems - BSDSF23M040-OS-A02*
