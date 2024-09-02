#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;

void loader_cleanup() {
    if (fd >= 0) {
        close(fd);
    }
    if (ehdr) {
        free(ehdr);
    }
    if (phdr) {
        free(phdr);
    }
}

void load_and_run_elf(char* exe) {
    fd = open(exe, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open ELF file");
        exit(1);
    }

    // Read ELF header
    ehdr = malloc(sizeof(Elf32_Ehdr));
    if (read(fd, ehdr, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
        perror("Failed to read ELF header");
        loader_cleanup();
        exit(1);
    }

    // Read Program Header Table
    lseek(fd, ehdr->e_phoff, SEEK_SET);
    phdr = malloc(ehdr->e_phnum * sizeof(Elf32_Phdr));
    if (read(fd, phdr, ehdr->e_phnum * sizeof(Elf32_Phdr)) != ehdr->e_phnum * sizeof(Elf32_Phdr)) {
        perror("Failed to read Program Header Table");
        loader_cleanup();
        exit(1);
    }

    // Find the PT_LOAD segment
    Elf32_Phdr *phdr_load = NULL;
    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type == PT_LOAD) {
            phdr_load = &phdr[i];
            break;
        }
    }
    if (!phdr_load) {
        fprintf(stderr, "Failed to find loadable segment\n");
        loader_cleanup();
        exit(1);
    }

    // Allocate memory and load the segment
    void *mem = mmap(NULL, phdr_load->p_memsz, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) {
        perror("mmap failed");
        loader_cleanup();
        exit(1);
    }
    lseek(fd, phdr_load->p_offset, SEEK_SET);
    if (read(fd, mem, phdr_load->p_filesz) != phdr_load->p_filesz) {
        perror("Failed to load segment");
        loader_cleanup();
        exit(1);
    }

    // Navigate to the entry point and typecast it to a function pointer
    void *entry_point_addr = mem + (ehdr->e_entry - phdr_load->p_vaddr);
    int (*_start)() = (int (*)())entry_point_addr;

    // Call the entry point function
    int result = _start();
    printf("User _start return value = %d\n", result);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <ELF Executable>\n", argv[0]);
        exit(1);
    }

    // Open and validate the ELF file
    fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("Failed to open file");
        exit(1);
    }
    
    Elf32_Ehdr ehdr;
    if (read(fd, &ehdr, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
        perror("Failed to read ELF header");
        close(fd);
        exit(1);
    }

    // Check the ELF magic number
    if (ehdr.e_ident[EI_MAG0] != ELFMAG0 || 
        ehdr.e_ident[EI_MAG1] != ELFMAG1 || 
        ehdr.e_ident[EI_MAG2] != ELFMAG2 || 
        ehdr.e_ident[EI_MAG3] != ELFMAG3) {
        fprintf(stderr, "Invalid ELF file\n");
        close(fd);
        exit(1);
    }
    close(fd);

    // Load and execute the ELF file
    load_and_run_elf(argv[1]);

    // Clean up resources
    loader_cleanup();

    return 0;
}
