#include "loader.h"

//typedef int (*StartFunction)();

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;
void *file_content;
off_t filesize;
void *virtual_mem;

/*
 * release memory and other cleanups
 */


/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char** argv) {
  fd = open(argv[1], O_RDONLY);
  if (fd == -1) {
    perror("Error opening file");
    exit(1);
  }
  filesize = lseek(fd, 0, SEEK_END);
  if (filesize == -1) {
        perror("Error getting file size");
        close(fd);
        return;
  }
  //struct stat st;
  //if (fstat(fd, &st) == -1) {
  //  perror("fstat");
  //  close(fd);
  //  exit(1);
  //}
  file_content = malloc(filesize);
  if (!file_content) {
      perror("Memory allocation failed");
      close(fd);
      return;
  }

  lseek(fd, 0, SEEK_SET);
  read(fd, file_content, filesize);

  /*if (read(fd, file_content, filesize) < 0) {
        perror("Error reading file");
        close(fd);
        free(file_content);
        return;
  }*/
  
  // 1. Load entire binary content into the memory from the ELF file.
  //file_content=mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  //if (file_content == MAP_FAILED) {
  //  perror("mmap");
  //  close(fd);
  //  exit(1);
  //}

  //filesize = st.st_size;
  ehdr=(Elf32_Ehdr*)(file_content);
  phdr=(Elf32_Phdr*)(file_content + ehdr->e_phoff); //____SEGMENTATION FAULT____ now fine!!!!
  Elf32_Addr entry_point=ehdr->e_entry;
  Elf32_Phdr *entry_segment =NULL;
  printf("CHECK 3\n");

  // 2. Iterate through the PHDR table and find the section of PT_LOAD type that contains the address of the entrypoint method in fib.c
  unsigned int p_memsz;
  for(int i=0; i<ehdr->e_phnum; ++i){
    if(phdr[i].p_type==PT_LOAD){
      p_memsz=phdr[i].p_memsz;
      if(ehdr->e_entry>=phdr->p_vaddr && ehdr->e_entry<phdr->p_vaddr+p_memsz){
        entry_segment=&phdr[i];
        break;
      }
    }
  }
  printf("CHECK 4\n");


  // 3. Allocate memory of the size "p_memsz" using mmap function and then copy the segment content
  virtual_mem= mmap(NULL, p_memsz, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_PRIVATE,0,0); //HOPEFULLY
  if (virtual_mem == MAP_FAILED) {
    perror("mmap");
    munmap(file_content, filesize);
    close(fd);
  }
  printf("CHECK 5\n");

  // Replace the memcpy line with this loop
  for (size_t offset = 0; offset < entry_segment->p_memsz; offset++) {
      ((char *)virtual_mem)[offset] = ((char *)file_content)[entry_segment->p_offset + offset];
  }


  // 4. Navigate to the entrypoint address into the segment loaded in the memory in above step
  void *entry_address = virtual_mem + (ehdr->e_entry - entry_segment->p_vaddr);


  // 5. Typecast the address to that of function pointer matching "_start" method in fib.c.
  int (*_start)() = (int (*)())entry_address;
  printf("CHECK 8\n");
  // 6. Call the "_start" method and print the value returned from the "_start"
  int result = _start();
  printf("CHECK 9\n");
  printf("User _start return value = %d\n",result);
}


void loader_cleanup() {
  munmap(file_content, filesize);
  close(fd);
}

int main(int argc, char** argv) 
{
  printf("ARGC: %d\n", argc);
  printf("ARGV: %s\n", argv[1]);
  if(argc != 2) {
    printf("%d\n", argc);
    printf("Usage: %s <ELF Executable> \n",argv[0]);
    exit(1);
  }
  // 1. carry out necessary checks on the input ELF file
  // 2. passing it to the loader for carrying out the loading/execution

  // load_and_run_elf(argv);
  load_and_run_elf(argv);
  // 3. invoke the cleanup routine inside the loader  
  loader_cleanup();
  return 0;
}

