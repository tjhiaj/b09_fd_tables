# b09_fd_tables

## Overview

This program processes file descriptor (FD) data for system processes, and can display or export the data in various formats based on user-specified flags. The program can be customized to display FD information in different formats, such as pre-processing output, system-wide data, vnode info, and more.

The program processes system data by iterating over process directories, extracting file descriptor information, and displaying it based on the selected flags.

## Disclaimers

- **Default Behavior**: If no argument is passed, the program will display the composite table, which is equivalent to using the --composite flag.
- **Threshold Behavior**: When the --threshold flag is used, the program assumes the default action of displaying the composite table along with threshold data.

## How I Built It

### First Iteration
In my first attempt, I focused on meeting the core requirement: listing the file descriptors for processes in the /proc directory. I began by navigating through each process directory (found under /proc/[pid]), looking for the fd directory, which contains links to open files. I also implemented a basic function to identify numeric directories (i.e., process IDs) to ensure I was only examining directories that represent actual processes.

My code used opendir and readdir to iterate through directories and check for symbolic links in the fd directories. Then, using readlink, I was able to retrieve the target of each link, which pointed to the file the file descriptor represented. Additionally, I used stat to get the inode of the target file, which was another piece of information I was tasked with displaying.

### Final Result
The final program iterated through the /proc directories, checked each process's fd directory for symbolic links, and output a nicely formatted list of file descriptors. The program also ensured that any errors during directory access or symbolic link reading were properly logged.

Looking back, the first iteration served as a good foundation, but it was the process of identifying and addressing inefficiencies, improving the user experience, and tightening error handling that led to the final, more polished version of the program.

## Modules and Functions

### isNumber(char* input)
- **Description**: Checks if a string consists solely of digits.
- **Parameters**: input (string) — the string to check.
- **Returns**: 1 if all characters are digits, `0` otherwise.

### printHeader(int flag, FILE* file)
- **Description**: Prints the appropriate header based on the flag provided.
- **Parameters**:
  - flag (int): Flag that determines the format of the header.
  - file (FILE*): Optional file output stream for writing headers.
- **Flags**:
  - FLAG_PRE_PROCESS: Pre-processing header.
  - FLAG_SYSTEM_WIDE: System-wide header.
  - FLAG_VNODES: Vnode-related header.
  - FLAG_OUTPUT_TXT: Text output header.
  - FLAG_OUTPUT_BINARY: Binary output header.
  - FLAG_COMPOSITE: Default or composite header.

### printSummary(int pidCount, PIDEntry * pidTable)
- **Description**: Prints a summary table showing the number of file descriptors for each process.
- **Parameters**:
  - pidCount (int): Number of processes to display.
  - pidTabl (PIDEntry*): Array of PID entries.

### printThreshold(int pidCount, PIDEntry * pidTable, int threshold_val)
- **Description**: Prints processes whose file descriptor count exceeds a specified threshold.
- **Parameters**:
  - pidCount (int): Number of processes.
  - pidTable (PIDEntry*): Table of process IDs and their file descriptor counts.
  - threshold_val (int): Threshold value for file descriptor count.

### printData(int flag, int * row, struct dirent * entry, struct dirent * fd_entry, char * target_path, struct stat statbuf, FILE* file)
- **Description**: Prints file descriptor data based on the specified flag.
- **Parameters**:
  - flag (int): Format flag to determine the output type.
  - row (int*): Row counter for display.
  - entry (struct dirent*): Process directory entry.
  - fd_entry (struct dirent*): File descriptor entry.
  - target_path (char*): Path of the target file.
  - statbuf (struct stat): File stat data.
  - file (FILE*): Optional output file stream.

### updatePIDTable(int * pidCount, PIDEntry** pidTable, struct dirent *entry)
- **Description**: Updates the PID table with the count of file descriptors for each process.
- **Parameters**:
  - pidCount (int*): Current count of processes.
  - pidTable (PIDEntry**): Table of PID entries.
  - entry (struct dirent*): Process directory entry.

### processFDEntry(int flag, int * row, struct dirent * entry, struct dirent * fd_entry, char * link_path, int* pidCount, PIDEntry** pidTable, FILE* file)
- **Description**: Processes an individual file descriptor entry for a specific process.
- **Parameters**:
  - flag (int): Output format flag.
  - row (int*): Row counter.
  - entry (struct dirent*): Process directory entry.
  - fd_entry (struct dirent*): File descriptor entry.
  - link_path (char*): Path to the file descriptor link.
  - pidCount (int*): Process count.
  - pidTable (PIDEntry**): Table of PID entries.
  - file (FILE*): Optional output file.

### processFD(int flag, int * row, int* pidCount, PIDEntry** pidTable, int target_pid, FILE* file, char * fd_path, struct dirent * entry, DIR *dir)
- **Description**: Processes all file descriptors for a specific process.
- **Parameters**:
  - flag (int): Output format flag.
  - row (int*): Row counter.
  - pidCount (int*): Process count.
  - pidTable (PIDEntry**): Table of PID entries.
  - target_pid (int): Target process ID.
  - file (FILE*): Optional output file.
  - fd_path (char*): Path to the file descriptors of the process.
  - entry (struct dirent*): Process directory entry.
  - dir (DIR*): Directory pointer for file descriptor data.

### processDirectory(int flag, int* pidCount, PIDEntry** pidTable, int target_pid, FILE* file)
- **Description**: Iterates over all process directories and processes file descriptors.
- **Parameters**:
  - flag (int): Output format flag.
  - pidCount (int*): Process count.
  - pidTable (PIDEntry**): Table of PID entries.
  - target_pid (int): Target process ID.
  - file (FILE*): Optional output file.

### parseArguments(int argc, char ** argv, int * flags, int * threshold_val, pid_t * target_pid, int * flag_detected)
- **Description**: Parses command-line arguments and sets the corresponding flags.
- **Parameters**:
  - argc (int): Number of arguments.
  - argv (char**): Array of argument strings.
  - flags (int*): Array of flags for processing.
  - threshold_val (int*): Threshold value for file descriptor count.
  - target_pid (pid_t*): Target process ID.
  - flag_detected (int*): Flag indicating whether a flag was detected.

### processFlags(int * flags, int threshold_val, pid_t target_pid, int flag_detected, PIDEntry ** pidTable, int * pidCount)
- **Description**: Processes flags and executes the corresponding actions.
- **Parameters**:
  - flags (int*): Array of flags.
  - threshold_val (int): Threshold value for file descriptor count.
  - target_pid (pid_t): Target process ID.
  - flag_detected (int): Flag indicating whether a flag was detected.
  - pidTable (PIDEntry**): Table of PID entries.
  - pidCount (int*): Number of processes.

## How to Run the Program

1. **Compile the Program**: Use the included Makefile to compile the program.

   make
3. **Run the Program**: Execute the program with the desired flags. Some example usage:

   ./showFDtables --summary --threshold=100
   
   ./showFDtables --composite
   
   ./showFDtables --systemWide --output_TXT
   
   ./showFDtables 1234
   
   If no arguments are passed, the program will display the composite table by default.

## Bonus

#### **All PIDs**:
| Run | Real_t (s) | User_t (s) | Sys_t (s) | Real_b (s) | User_b (s) | Sys_b (s) | Size_t (bytes) | Size_b (bytes) |
|-----|------------|------------|-----------|------------|------------|-----------|----------------|----------------|
| 1   | 0.024      | 0.001      | 0.010     | 0.020      | 0.000      | 0.009     | 13383          | 8606           |
| 2   | 0.019      | 0.000      | 0.009     | 0.021      | 0.002      | 0.008     | 13383          | 8606           |
| 3   | 0.022      | 0.002      | 0.010     | 0.021      | 0.001      | 0.009     | 13383          | 8606           |
| 4   | 0.023      | 0.001      | 0.010     | 0.024      | 0.000      | 0.011     | 13383          | 8606           |
| 5   | 0.021      | 0.002      | 0.009     | 0.023      | 0.000      | 0.011     | 13383          | 8606           |

#### **One PID (3535897)**:
| Run | Real_t (s) | User_t (s) | Sys_t (s) | Real_b (s) | User_b (s) | Sys_b (s) | Size_t (bytes) | Size_b (bytes) |
|-----|------------|------------|-----------|------------|------------|-----------|----------------|----------------|
| 1   | 0.016      | 0.002      | 0.003     | 0.015      | 0.000      | 0.004     | 3662           | 2354           |
| 2   | 0.014      | 0.000      | 0.004     | 0.015      | 0.000      | 0.005     | 3662           | 2354           |
| 3   | 0.015      | 0.001      | 0.004     | 0.014      | 0.001      | 0.004     | 3662           | 2354           |
| 4   | 0.015      | 0.001      | 0.004     | 0.015      | 0.000      | 0.005     | 3662           | 2354           |
| 5   | 0.014      | 0.001      | 0.004     | 0.009      | 0.001      | 0.000     | 3662           | 2354           |

#### **All PIDs**:
- **output_TXT**:
  - Real: (0.024 s + 0.019 s + 0.022 s + 0.023 s + 0.021 s) / 5 = 0.0218 s
  - User: (0.001 s + 0.000 s + 0.002 s + 0.001 s + 0.002 s) / 5  = 0.0012 s
  - Sys: (0.010s  + 0.009 s + 0.010 s + 0.010 s + 0.009 s) / 5 = 0.0096 s
  - Size: 13383 bytes

- **output_binary**:
  - Real: (0.020 s + 0.021 s + 0.021 s + 0.024 s + 0.023 s) / 5 = 0.0218 s
  - User: (0.000 s + 0.002 s + 0.001 s + 0.000 s + 0.000 s) / 5 = 0.0006 s
  - Sys: (0.009 s + 0.008 s + 0.009 s + 0.011 s + 0.011 s) / 5 = 0.0096 s
  - Size: 8606 bytes

#### **One PID (3535897)**:
- **output_TXT**:
  - Real: (0.016 + 0.014 + 0.015 + 0.015 + 0.014) / 5 = 0.0148 s
  - User: (0.002 + 0.000 + 0.001 + 0.001 + 0.001) / 5 = 0.0010 s
  - Sys: (0.003 + 0.004 + 0.004 + 0.004 + 0.004) / 5 = 0.0038 s
  - Size: 3662 bytes

- **output_binary**:
  - Real: (0.015 s + 0.015 s + 0.014 s + 0.015 s + 0.0095 s) / 5 = 0.0136 s
  - User: (0.000 + 0.000 + 0.001 + 0.000 + 0.001) / 5 = 0.0004 s
  - Sys: (0.004 + 0.005 + 0.004 + 0.005 + 0.000) / 5 = 0.0036 s
  - Size: 2354 bytes

#### **All PIDs**:
- **output_TXT**:
  - Real: sqrt(((0.024-0.0218)^2 + (0.019-0.0218)^2 + (0.022-0.0218)^2 + (0.023-0.0218)^2 + (0.021-0.0218)^2) / 5) = 0.0017 s
  - User: sqrt(((0.001-0.0012)^2 + (0.000-0.0012)^2 + (0.002-0.0012)^2 + (0.001-0.0012)^2 + (0.002-0.0012)^2) / 5) = 0.0008 s
  - Sys: sqrt(((0.010-0.0096)^2 + (0.009-0.0096)^2 + (0.010-0.0096)^2 + (0.010-0.0096)^2 + (0.009-0.0096)^2) / 5) = 0.0005 s

- **output_binary**:
  - Real: sqrt(((0.020-0.0218)^2 + (0.021-0.0218)^2 + (0.021-0.0218)^2 + (0.024-0.0218)^2 + (0.023-0.0218)^2) / 5) = 0.0017 s
  - User: sqrt(((0.000-0.0006)^2 + (0.002-0.0006)^2 + (0.001-0.0006)^2 + (0.000-0.0006)^2 + (0.000-0.0006)^2) / 5) = 0.0008 s
  - Sys: sqrt(((0.009-0.0096)^2 + (0.008-0.0096)^2 + (0.009-0.0096)^2 + (0.011-0.0096)^2 + (0.011-0.0096)^2) / 5) = 0.0011 s

#### **One PID (3535897)**:
- **output_TXT**:
  - Real: sqrt(((0.016-0.0148)^2 + (0.014-0.0148)^2 + (0.015-0.0148)^2 + (0.015-0.0148)^2 + (0.014-0.0148)^2) / 5) = 0.0007 s
  - User: sqrt(((0.002-0.0010)^2 + (0.000-0.0010)^2 + (0.001-0.0010)^2 + (0.001-0.0010)^2 + (0.001-0.0010)^2) / 5) = 0.0007 s
  - Sys: sqrt(((0.003-0.0038)^2 + (0.004-0.0038)^2 + (0.004-0.0038)^2 + (0.004-0.0038)^2 + (0.004-0.0038)^2) / 5) = 0.0004 s

- **output_binary**:
  - Real: sqrt(((0.015-0.0136)^2 + (0.015-0.0136)^2 + (0.014-0.0136)^2 + (0.015-0.0136)^2 + (0.009-0.0136)^2) / 5) = 0.0022 s
  - User: sqrt(((0.000-0.0004)^2 + (0.000-0.0004)^2 + (0.001-0.0004)^2 + (0.000-0.0004)^2 + (0.001-0.0004)^2) / 5) = 0.0005 s
  - Sys: sqrt(((0.004-0.0036)^2 + (0.005-0.0036)^2 + (0.004-0.0036)^2 + (0.005-0.0036)^2 + (0.000-0.0036)^2) / 5 ) = 0.0018 s

#### **All PIDs**:
| Metric               | output_TXT (Avg ± Std Dev)   | output_binary (Avg ± Std Dev)   |
|----------------------|------------------------------|---------------------------------|
| Real Time (s)        | 0.0218 ± 0.0017              | 0.0218 ± 0.0017                 |
| User Time (s)        | 0.0012 ± 0.0008              | 0.0006 ± 0.0008                 |
| Sys Time (s)         | 0.0096 ± 0.0005              | 0.0096 ± 0.0011                 |
| File Size (bytes)    | 13383                        | 8606                            |

#### **One PID (3535897)**:
| Metric               | output_TXT (Avg ± Std Dev)   | output_binary (Avg ± Std Dev)   |
|----------------------|------------------------------|---------------------------------|
| Real Time (s)        | 0.0148 ± 0.0007              | 0.0136 ± 0.0022                 |
| User Time (s)        | 0.0010 ± 0.0007              | 0.0004 ± 0.0005                 |
| Sys Time (s)         | 0.0038 ± 0.0004              | 0.0036 ± 0.0018                 |
| File Size (bytes)    | 3662                         | 2354                            |

1. **Execution Time**:
   - For **all PIDs**, both output_TXT and output_binary have similar real times (~0.0218s), but output_binary has slightly lower user time.
   - For **one PID**, output_binary is slightly faster in real time (~0.0136s vs. ~0.0148s).

2. **File Sizes**:
   - Binary files are significantly smaller than text files:
     - All PIDs: 8606 bytes (binary) vs. 13383 bytes (text).
     - One PID: 2354 bytes (binary) vs. 3662 bytes (text).

3. **Efficiency**:
   - Binary output is more efficient in terms of file size and slightly faster in execution time.

4. **Trade-offs**:
   - Use text output for human readability.
   - Use binary output for efficiency and smaller file sizes.