# b09_fd_tables

## Overview

This program processes file descriptor (FD) data for system processes, and can display or export the data in various formats based on user-specified flags. The program can be customized to display FD information in different formats, such as pre-processing output, system-wide data, vnode info, and more.

The program processes system data by iterating over process directories, extracting file descriptor information, and displaying it based on the selected flags.

## Disclaimers

- **Default Behavior**: If no argument is passed, the program will display the composite table, which is equivalent to using the --composite flag.
- **Threshold Behavior**: When the --threshold flag is used, the program assumes the default action of displaying the composite table along with threshold data.

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
  - pidTable (PIDEntry*): Array of PID entries.

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

## Comments on Implementation

- **Refactoring**: Various modules were refactored to streamline flag processing, improve maintainability, and prevent function signatures from becoming too large with new flags.
- **Performance**: The program processes directories and file descriptors efficiently, but may be limited by system permissions and the number of processes being monitored.
- **Error Handling**: Error handling has been implemented for file operations such as reading symbolic links and opening directories.
