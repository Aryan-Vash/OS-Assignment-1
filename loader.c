#include "loader.h"

Elf32_Ehdr *ehdr = NULL;
Elf32_Phdr *phdr = NULL;
int fd = -1;

void loader_cleanup() {
  if (fd >= 0) {
     close(fd);
  }
  free(ehdr);
  free(phdr);
}

void load_and_run_elf(char** exe) {

  // 1. Load entire binary content into the memory from the ELF file.
  fd = open(exe[1], O_RDONLY);

  if (fd < 0) {
    perror("open");
    exit(1);
  }

  ehdr = (Elf32_Ehdr*)malloc(sizeof(Elf32_Ehdr));
  if (!ehdr) {
    perror("malloc for ehdr");
    loader_cleanup();
    exit(1);
  }

  if (read(fd, ehdr, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
    perror("read ehdr");
    loader_cleanup();
    exit(1);
  }

  phdr = (Elf32_Phdr*)malloc(ehdr->e_phentsize * ehdr->e_phnum);
  if (!phdr) {
    perror("malloc for phdr");
    loader_cleanup();
    exit(1);
  }

  if (read(fd, phdr, sizeof(Elf32_Phdr)) != sizeof(Elf32_Phdr)) {
    perror("read phdr");
    loader_cleanup();
    exit(1);
  }

  // 2. Iterate through the PHDR table and find the section of PT_LOAD 
  //    type that contains the address of the entrypoint method in fib.c
  Elf32_Phdr* phdr_load = NULL;
  for (int i = 0; i < ehdr->e_phnum; i++) {
    if (read(fd, phdr, sizeof(Elf32_Phdr)) != sizeof(Elf32_Phdr)) {
      perror("read phdr in loop");
      loader_cleanup();
      exit(1);
    }

    if (phdr->p_type == 1) {
      if (phdr->p_vaddr <= ehdr->e_entry && ehdr->e_entry <= phdr->p_vaddr + phdr->p_memsz) {
        phdr_load = phdr;
        break;
      }
    }
  }

  if (!phdr_load) {
    fprintf(stderr, "Load Segment not found\n");
    loader_cleanup();
    exit(1);
  }

  // 3. Allocate memory of the size "p_memsz" using mmap function 
  //    and then copy the segment content
  void *mem = mmap(NULL, phdr_load->p_memsz, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (mem == MAP_FAILED) {
    perror("mmap");
    loader_cleanup();
    exit(1);
  }

  if (read(fd, mem, phdr_load->p_memsz) != (ssize_t)phdr_load->p_memsz) {
    perror("read load segment");
    loader_cleanup();
    exit(1);
  }

  // 4. Navigate to the entrypoint address into the segment loaded in the memory in above step
  
  int (*_start)() = (int(*)()) (mem + (ehdr->e_entry - phdr_load->p_vaddr));
  
  // 5. Typecast the address to that of function pointer matching "_start" method in fib.c.
  // 6. Call the "_start" method and print the value returned from the "_start"
  int result = _start();
  printf("User _start return value = %d\n", result);
}

int main(int argc, char** argv) {
  if (argc != 2) {
    printf("Usage: %s <ELF Executable>\n", argv[0]);
    exit(1);
  }

  load_and_run_elf(argv);
  loader_cleanup();
  return 0;
}
