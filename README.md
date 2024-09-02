## Overview

This project is a simple ELF (Executable and Linkable Format) loader and runner implemented in C. It loads a 32-bit ELF executable file into memory and transfers control to the entry point of the executable, effectively running it. The loader reads the ELF headers, locates the appropriate loadable segment, maps it into memory, and then jumps to the entry point to execute the program.

## Files

- fib.c: The input file which is converted into an executable file and then loaded.
- loader.c: Contains the main logic for loading and running the ELF file.
- loader.h: (Assumed) Contains necessary headers and possibly declarations for functions used in `loader.c`.
- Makefile: Contains the necessary commands to run these files.

## Requirements

- A 32-bit ELF executable file to load and run.
- A Linux environment that supports ELF executables.
- A C compiler (e.g., GCC) to compile the loader.

## Compilation

To compile the loader, you can use the following command:

```sh
gcc -o loader loader.c -ldl
```

This command compiles `loader.c` and produces an executable named `loader`.

## Usage

Once compiled, you can use the loader to execute a 32-bit ELF executable as follows:

```sh
./loader <path_to_ELF_executable>
```

- `<path_to_ELF_executable>`: The path to the ELF file you want to load and run.

## Code Explanation

### Main Functions

- **loader_cleanup()**: 
  - This function handles cleanup operations by closing any open file descriptors and freeing dynamically allocated memory.

- **load_and_run_elf(char** exe)**:
  - This function takes the path to an ELF executable, loads it into memory, and executes it.
  - It performs the following steps:
    1. **Open the ELF file**: The file is opened for reading.
    2. **Read the ELF Header**: The ELF header is read to get information about the executable.
    3. **Read Program Headers**: The program headers are read to determine the segments to be loaded.
    4. **Locate Loadable Segment**: The appropriate segment containing the entry point is identified.
    5. **Memory Mapping**: The segment is mapped into memory using `mmap`.
    6. **Jump to Entry Point**: The program control is transferred to the entry point of the executable.
    7. **Execute the Program**: The program is executed, and the return value is printed.

- **main(int argc, char** argv)**:
  - The main function checks if the correct number of arguments is provided. If so, it calls `load_and_run_elf` with the provided ELF file path and performs cleanup after execution.

### Error Handling

The loader performs basic error handling:
- If the ELF file cannot be opened or if any memory allocation fails, appropriate error messages are displayed, and the program exits.
- If the correct segment containing the entry point cannot be found, the program exits with an error.

### Important Notes

- **Security**: This loader is a simple educational example. In a real-world scenario, additional security checks and validations are required before executing arbitrary code.
- **Compatibility**: This code assumes that the system is capable of executing 32-bit ELF files. Make sure your system's architecture supports 32-bit executables.

## License

This project is provided as-is for educational purposes. Use it at your own risk.
