#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;
void *file_content;
off_t filesize;
void *virtual_mem;

/*
 * release memory and other cleanups
 */
void loader_cleanup() {
  munmap(file_content, filesize);
  close(fd);
}

/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char** argv) {
  fd = open(argv[1], O_RDONLY);
  filesize = lseek(fd, 0, SEEK_END);
  file_content = malloc(filesize);

  lseek(fd, 0, SEEK_SET);
  read(fd, file_content, filesize);
  
  // 1. Load entire binary content into the memory from the ELF file.
  ehdr=(Elf32_Ehdr*)(file_content);
  phdr=(Elf32_Phdr*)(file_content + ehdr->e_phoff);
  Elf32_Phdr *entry_segment =NULL;


  // 2. Iterate through the PHDR table and find the section of PT_LOAD type that contains the address of the entrypoint method in fib.c
  int i = 0;
  while (i < ehdr->e_phnum) {
      if (phdr[i].p_type == PT_LOAD) {
          p_memsz = phdr[i].p_memsz;
          if (ehdr->e_entry >= phdr[i].p_vaddr && ehdr->e_entry < phdr[i].p_vaddr + p_memsz) {
              entry_segment = &phdr[i];
              break;
          }
      }
      i++;
  }


  // 3. Allocate memory of the size "p_memsz" using mmap function and then copy the segment content
  virtual_mem= mmap(NULL, p_memsz, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_PRIVATE,0,0); //HOPEFULLY
  if (virtual_mem == MAP_FAILED) {
    perror("mmap");
    munmap(file_content, filesize);
    close(fd);
  }

  size_t offset = 0;
  while (offset < entry_segment->p_memsz) {
      ((char *)virtual_mem)[offset] = ((char *)file_content)[entry_segment->p_offset + offset];
      offset++;
  }


  // 4. Navigate to the entrypoint address into the segment loaded in the memory in above step
  void *entry_address = virtual_mem + (ehdr->e_entry - entry_segment->p_vaddr);


  // 5. Typecast the address to that of function pointer matching "_start" method in fib.c.
  int (*_start)() = (int (*)())entry_address;


  // 6. Call the "_start" method and print the value returned from the "_start"
  int result = _start();
  printf("User _start return value = %d\n",result);
}

int main(int argc, char** argv) 
{
  if(argc != 2) {
    printf("Usage: %s <ELF Executable> \n",argv[0]);
    exit(1);
  }
  // 1. carry out necessary checks on the input ELF file
  // 2. passing it to the loader for carrying out the loading/execution
  load_and_run_elf(argv);
  // 3. invoke the cleanup routine inside the loader  
  loader_cleanup();
  return 0;
}
