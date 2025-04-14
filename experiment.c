#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>
#include<unistd.h>
#include <limits.h>
#include <signal.h>
#include <malloc.h>
#include<string.h>

#define handle_error(msg) do {perror(msg); exit(EXIT_FAILURE);} while(0)

static char *buffer;

/* 
Note: calling printf() from a signal handler is not safe
(and should not be done in production programs), since
printf() is not async-signal-safe; see signal-safety(7).
Nevertheless, we use printf() here as a simple way of
showing that the handler was called. 
*/
static void handler(int sig, siginfo_t *si, void *unused){
  printf("Got SIGSEGV at address: 0x%lx\n", (long) si->si_addr);
  exit(EXIT_FAILURE);
}
  
int main(int argc, char *argv[]){
  char *p, *buffer; // * means 'a pointer to'
	char c; //c is a char
  int pagesize;
	int i = 0, size;
  
  struct sigaction sa;
  sa.sa_flags = SA_SIGINFO;
  sigemptyset(&sa.sa_mask);
  sa.sa_sigaction = handler;
  
  if(sigaction(SIGSEGV, &sa, NULL) == -1)
    handle_error("sigaction");

  // Initializing Pagesize, most systems its 4096 bytes
  pagesize = sysconf(_SC_PAGE_SIZE);
  printf("page size: %d bytes\n", pagesize);


  //variables
  char name[] = "Nathan";
  int name_length = 6;
  char *page9;
  char *page10;

  char *page7;
  char *page8;
  char last_name[] = "Kipple";
  int last_length = 6;
  int result;

  char *page5;
  char *page6;
  char username[] = "nxk22";
  int username_length = 5;

  char *copy_buffer;



  if(pagesize == -1)
    handle_error("sysconf");
 
  //Allocate a buffer; it will have the default protection of PROT_READ|PROT_WRITE. 
  size = pagesize * 10;
  p = memalign(pagesize, size); //Allocating buffer of size of ten pages
  
  if (p == NULL)
    handle_error("memalign");

  memset(p, 0x00, size); // Zeros
  memset(p, 0x41, size); // 'A'



  //pages

  page9 = p + pagesize * 9;
  page10 = p + pagesize * 10;

  page7 = p + 7 * pagesize;
  page8 = p + 8 * pagesize;

  page5 = p + 5 * pagesize;
  page6 = p + 6 * pagesize;
  




  for(i = 0; i < 10; i++){
    /*
    Printing all pages first bytes from first page. 
    The usage of %d format specifier causes compilation warnings.
    */	
	  printf("Address of %d Page: %lx\n", i + 1 , p + (i * pagesize));	
  }



  

  // TODO: Start writing code here and can define variables for functions above


  //mprotect settings
  //7 & 8
  result = mprotect(page7, 2 * pagesize, PROT_READ | PROT_WRITE);
    if (result == -1)
      handle_error("mprotect");

  //5 and 6
  result= mprotect(page5, pagesize * 2, PROT_WRITE);
    if (result == -1){
      handle_error("mprotect");
    }


  //copy buffer
  copy_buffer = malloc(2 * pagesize);
  if (copy_buffer == NULL) {
    handle_error("malloc");
  }


  memcpy(copy_buffer, page7, pagesize);
  memcpy(copy_buffer + pagesize, page8, pagesize);


  memcpy(copy_buffer, p + 6 * pagesize, pagesize);
  memcpy(copy_buffer + pagesize, p + 9 * pagesize, pagesize);



  memcpy(page7, last_name, last_length);
  memcpy(page8, last_name, last_length);


  memcpy(page9, name, name_length);
  memcpy(page10, name, name_length);


  memcpy(page5, username, username_length);
  memcpy(page6, username, username_length);



 


  //output
  //first name
  printf("page 9: %.*s\n", name_length, page9);
  printf("page 10: %.*s\n", name_length, page10);

  //last name
  printf("page 7: %.*s\n", last_length, page7);
  printf("page 8: %.*s\n", last_length, page8);

  //username
  printf("page 5: %.*s\n", page5);
  printf("page 6: %.*s\n", page6);


  


  printf("copy buffer:\n");
  //last name copy
  write(STDOUT_FILENO, copy_buffer, 2 * 6);


  printf("buffer overwrite:\n");
  //username and first name overwrite
  write(STDERR_FILENO, copy_buffer, 5 + 6);
   
  
  


  exit(EXIT_SUCCESS);
}
