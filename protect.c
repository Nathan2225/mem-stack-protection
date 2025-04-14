#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>
#include <limits.h>
#include <signal.h>
#include <malloc.h>
#include <string.h>

#define handle_error(msg) \
  do                      \
  {                       \
    perror(msg);          \
    exit(EXIT_FAILURE);   \
  } while (0)

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
  char *p, *buffer;
  char c;
  int pagesize;
  int i = 0, size;

  struct sigaction sa;
  
  sa.sa_flags = SA_SIGINFO;
  sigemptyset(&sa.sa_mask);
  sa.sa_sigaction = handler;

  if (sigaction(SIGSEGV, &sa, NULL) == -1)
    handle_error("sigaction");

  // usually 4096 bytes
  pagesize = sysconf(_SC_PAGE_SIZE); 
  if (pagesize == -1)
    handle_error("sysconf");

  //Allocate mem. It will have the default protection of PROT_READ|PROT_WRITE. 
  size = pagesize * 2; // two pages pls
  p = memalign(pagesize, size); 
  
  if (p == NULL)
    handle_error("memalign");

  memset(p, 0x00, size);
  memset(p, 0x41, size);

  printf("First Page:\n");
  //Print first 3 bytes of first page
  for (i = 0; i < 3; i++){
    printf("%d=%c, %lx\n", i + 1, *(p + i), p + i);
  }
  //Print last 3 bytes of first page
  for (i = pagesize - 3; i < pagesize; i++){
    printf("%d=%c, %lx\n", i + 1, *(p + i), p + i);
  }
  
  printf("Second Page:\n");
  //Print first 3 bytes of second page
  for (i = pagesize; i < (pagesize + 3); i++){
    printf("%d=%c, %lx\n", i + 1, *(p + i), p + i);
  }
  //Print last 3 bytes of second page
  for (i = size - 3; i < size; i++){
    printf("%d=%c, %lx\n", i + 1, *(p + i), p + i);
  }

  //using mprotect on first page with read and write access
  if (mprotect(size, pagesize, PROT_READ | PROT_WRITE) == -1){
    handle_error("mprotect");
  }

  //call it 'buffer' now
  buffer = p;
  i = 0;

  // overwrite first and last 3 bytes of first page
  *(buffer + i) = 'S'; 
  *(buffer + (i + 1)) = 'E';
  *(buffer + (i + 2)) = 'C';
  *(buffer + (pagesize - 3)) = 'D';
  *(buffer + (pagesize - 2)) = 'E';
  *(buffer + (pagesize - 1)) = 'V';

  printf("First Page after overwriting:\n");
  //Print first 3 bytes of first page
  for (i = 0; i < 3; i++){
    printf("%d=%c, %lx\n", i + 1, *(p + i), p + i); 
  }
  //Print last 3 bytes of first page
  for (i = pagesize - 3; i < pagesize; i++){
    printf("%d=%c, %lx\n", i + 1, *(p + i), p + i);
  }
  
  if (mprotect(p + pagesize, pagesize, PROT_READ) == -1){
    handle_error("mprotect");
  }

  i = pagesize; // i = 4096, so that i + 4096 will point to second page
  // overwrite first and last 3 bytes of second page
  *(buffer + i) = 'S'; 
  *(buffer + (i + 1)) = 'E';
  *(buffer + (i + 2)) = 'C';
  *(buffer + (size - 3)) = 'D';
  *(buffer + (size - 2)) = 'E';
  *(buffer + (size - 1)) = 'V';

  printf("Second Page after overwriting:\n");
  //Print first 3 bytes of second page
  for (i = pagesize; i < (pagesize + 3); i++){
    printf("%d=%c, %lx\n", i + 1, *(p + i), p + i);
  }
  //Print last 3 bytes of first page
  for (i = size - 3; i < size; i++){
    printf("%d=%c, %lx\n", i + 1, *(p + i), p + i);
  }

  printf("Done\n");

  exit(EXIT_SUCCESS);
}
