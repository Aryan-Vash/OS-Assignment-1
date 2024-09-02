#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;

void loader_cleanup() {
  if (fd >= 0) {
     close(fd);
  }
  free(ehdr);
  free(phdr);
}

void load_and_run_elf(char** exe) {

  fd = open(exe[1], O_RDONLY);

  if(fd<0){
    perror("open");
    exit(1);
  }

  ehdr = (Elf32_Ehdr*)malloc(sizeof(Elf32_Ehdr));
  read(fd, ehdr, sizeof(Elf32_Ehdr));

  lseek(fd, ehdr->e_phoff, SEEK_SET);
  phdr = (Elf32_Phdr*)malloc(ehdr->e_phentsize*ehdr->e_phnum);
  read(fd,phdr,sizeof(Elf32_Phdr));
  
  Elf32_Phdr* phdr_load = NULL;
  for(int i=0; i< ehdr->e_phnum; i++){
    read(fd, phdr, sizeof(Elf32_Phdr));
    if(phdr->p_type == 1) {
      if (phdr->p_vaddr<=ehdr->e_entry && ehdr->e_entry<=phdr->p_vaddr+phdr->p_memsz){
        phdr_load = phdr;
        break;
      }
    }
  }

  if(!phdr_load){
    fprintf(stderr, "Load Segment \n");
    exit(1);
  }

  void *mem = mmap(NULL, phdr_load->p_memsz, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, fd, phdr_load->p_offset);
  if (mem == MAP_FAILED) {
      perror("mmap erroe");
      loader_cleanup();
      exit(1);
  }

  lseek(fd, phdr_load->p_offset, SEEK_SET);
  read(fd, mem, phdr_load->p_memsz);

  int (*_start)()= (int(*)()) mem + (ehdr->e_entry - phdr_load->p_vaddr);

  // int (*_entry)()= (int(*)())_start;

  int result = _start();

  printf("User _start return value = %d\n",result);
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
