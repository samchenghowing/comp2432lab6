// lab 6A
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main()
{
  int	fd[2];	// for the pipe
  char	buf[80], buf2[80];
  int	n;

  if (pipe(fd) < 0) {
     printf("Pipe creation error\n");
     exit(1);
  }
  // repeat a loop to write into the pipe and read from the same pipe
  while (1) {
        printf("Please input a line\n");
        n = read(STDIN_FILENO, buf, 80); // read a line from stdin
	if (n > 1) {
                buf[--n] = 0; // remove newline character
                printf("%d char in input line: [%s]\n", n, buf);

                write(fd[1], buf, n); // write to pipe
                printf("Input line [%s] written to pipe\n",buf);

                n = read(fd[0], buf2, 80); // read from pipe
                buf2[n] = 0;
                printf("%d char read from pipe: [%s]\n", n, buf2);
        }
  }
  printf("bye bye\n");
  close(fd[0]);
  close(fd[1]);
  exit(0);
}
