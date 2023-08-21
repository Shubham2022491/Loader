#include "loader.h"

typedef int (*StartFunction)();

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;
void *file_content;
int filesize;

/*
 * release memory and other cleanups
 */


/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char* argv) {
  fd = open(argv, O_RDONLY);

  struct stat st;
  // 1. Load entire binary content into the memory from the ELF file.
  file_content=mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  filesize = st.st_size;

  ehdr=(Elf32_Ehdr*)(file_content);
  phdr=(Elf32_Phdr*)(file_content + ehdr->e_phoff);
  Elf32_Addr entry_point=ehdr->e_entry;
  Elf32_Phdr *entry_segment =NULL;


  // 2. Iterate through the PHDR table and find the section of PT_LOAD type that contains the address of the entrypoint method in fib.c
  unsigned int pmemsz;
  for(int i=0; i<ehdr->e_phnum; ++i){
    if(phdr[i].p_type==PT_LOAD){
      pmemsz=phdr[i].p_memsz;
      if(ehdr->e_entry>=phdr->p_vaddr && ehdr->e_entry<phdr->p_vaddr+pmemsz){
        entry_segment=&phdr[i];
        break;
      }
    }
  }


  // 3. Allocate memory of the size "p_memsz" using mmap function and then copy the segment content
  void* virtual_mem=mmap(NULL, pmemsz, PROT_READ|PROT_EXEC, MAP_ANONYMOUS|MAP_PRIVATE,0,0); //HOPEFULLY


  // 4. Navigate to the entrypoint address into the segment loaded in the memory in above step
  int offset_within_segment = entry_point - entry_segment -> p_vaddr;
  void *entry_address = (char* )file_content + entry_segment -> p_offset + offset_within_segment;

  // 5. Typecast the address to that of function pointer matching "_start" method in fib.c.
  int (*_start)() = (int (*)())(StartFunction)entry_address;
  // 6. Call the "_start" method and print the value returned from the "_start"
  int result = _start();
  printf("User _start return value = %d\n",result);
}


void loader_cleanup() {
  munmap(file_content, filesize);
  close(fd);
}

int main(int argc, char** argv) 
{
  if(argc != 2) {
    printf("Usage: %s <ELF Executable> \n",argv[0]);
    exit(1);
  }
  // 1. carry out necessary checks on the input ELF file
  // 2. passing it to the loader for carrying out the loading/execution
  load_and_run_elf(argv[1]);
  // 3. invoke the cleanup routine inside the loader  
  loader_cleanup();
  return 0;
}
