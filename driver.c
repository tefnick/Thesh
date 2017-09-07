#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

struct cmd
  {
    int redirect_in;     /* Any stdin redirection?         */
    int redirect_out;    /* Any stdout redirection?        */
    int redirect_append; /* Append stdout redirection?     */
    int background;      /* Put process in background?     */
    int piping;          /* Pipe prog1 into prog2?         */
    char *infile;        /* Name of stdin redirect file    */
    char *outfile;       /* Name of stdout redirect file   */
    char *argv1[10];     /* First program to execute       */
    char *argv2[10];     /* Second program in pipe         */
  };

int cmdscan(char *cmdbuf, struct cmd *com);

int
main()
{
  char buf[1024];
  struct cmd command;
  int i;

  while( ( gets(buf) != NULL ) ){
    if (cmdscan(buf,&command)){
      printf("Illegal Format: \n");
      continue;
    }
    i = 0;
    printf("prog1 = ");
    printf("%s\n",command.argv1[i++]);
    while( command.argv1[i] != NULL )
      printf("        %s\n",command.argv1[i++]);
    printf("redirect_in = %d\nredirect_out = %d\nredirect_append = %d\nbackground = %d\npiping = %d\n",
         command.redirect_in, command.redirect_out, command.redirect_append, 
         command.background, command.piping);
    if ( command.redirect_in )
      printf("infile = %s\n", command.infile);
    if ( command.redirect_out )
      printf("outfile = %s\n", command.outfile);
    if ( command.piping ) {
      i = 0;
      printf("prog2 = ");
      printf("%s\n",command.argv2[i++]);
      while( command.argv2[i] != NULL )
        printf("        %s\n",command.argv2[i++]);
    }
  }

  return(0);
}
