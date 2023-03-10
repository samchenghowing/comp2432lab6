// lab 6B 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main()
{
  int	fd[2];	// for the pipe
  char	buf[80], buf2[80];
  int	even, n;

  if (pipe(fd) < 0) {
     printf("Pipe creation error\n");
     exit(1);
  }
  even = 1;
  // repeat a loop to write into the pipe and read from the same pipe
  while (1) {
        even = 1 - even; // toggle even variable
        if (even)
           printf("Please input an even line\n");
        else 
           printf("Please input an odd line\n");
        n = read(STDIN_FILENO, buf, 80); // read a line from stdin
        if (n <= 0) break; // EOF or error
        buf[--n] = 0; // remove newline character
        printf("%d char in input line: [%s]\n", n, buf);

        write(fd[1], buf, n); // write to pipe
        printf("Input line [%s] written to pipe\n",buf);

        if (even) { // only read with even loop
           n = read(fd[0], buf2, 80);
           buf2[n] = 0;
           printf("%d char read from pipe: [%s]\n", n, buf2);
        }
  }
  printf("bye bye\n");
  close(fd[0]);
  close(fd[1]);
  exit(0);
}
