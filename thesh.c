#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

struct cmd {
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
void sys_err(const char* str);

int main (void){

	char buf[1024]; 	//char buffer
	struct cmd command;

	printf(">> ");
	//gets(buf);

	while( ( gets(buf) != NULL )){
	    if (cmdscan(buf, &command)){
	      printf("Illegal Format: \n");
	      continue;
	    }
	
		if(strcmp(command.argv1[0],"exit") == 0 ) {		//terminate on cmd "exit"
            break;
        }

	    if (!command.piping && !command.background ){	//if no pipe and no bkgrnd
	    	switch(fork()){
				case -1:
					sys_err("fork error");
				case 0:			//child
					if(command.redirect_in){
						int fdin;
						if ( (fdin = open(command.infile, O_RDONLY)) < 0){
							sys_err("open error");
						}
						dup2(fdin, STDIN_FILENO);
						close(fdin);
					}
					if(command.redirect_out){
						int fdout;
						if(command.redirect_append){
							if ( (fdout = open(command.outfile, O_WRONLY | O_APPEND | O_CREAT, 0644)) < 0){
								sys_err("open error");
							}
						}else{
							if ( (fdout = open(command.outfile, O_WRONLY | O_TRUNC | O_CREAT, 0644)) < 0){
								sys_err("open error");
							}
						}
						dup2(fdout, STDOUT_FILENO);
						close(fdout);
					}
					
					execvp(command.argv1[0], command.argv1);
					sys_err("exec error");
				default:
					wait(NULL);
			}//end switch
	    } //end if	

	    if (!command.piping && command.background){		//if no pipe and bkg is TRUE
	    	switch(fork()){
	    		case -1:
	    			sys_err("fork error");
	    		case 0:					//child
	    			switch(fork()){		
	    				case -1:
	    					sys_err("fork error");
	    				case 0:			//grandchild
			    			if(command.redirect_in){
			    				int fdin;
							if ( (fdin = open(command.infile, O_RDONLY)) < 0){
								sys_err("open error");
							}
			    				dup2(fdin, STDIN_FILENO);
			    				close(fdin);
			    			}
			    			if(command.redirect_out){
			    				int fdout; 
			    				if(command.redirect_append){
			    					if ( (fdout = open(command.outfile, O_WRONLY | O_APPEND | O_CREAT, 0644)) < 0){
									sys_err("open error");
								}

			    				}else{
			    					if ( (fdout = open(command.outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0){
									sys_err("open error");
								}
			    				}
			    				dup2(fdout, STDOUT_FILENO);
			    				close(fdout);
			    			}

			    			execvp(command.argv1[0], command.argv1);
			    			sys_err("exec error");
						
	    				default:	//child exits after fork
	    					exit(0);
					} //end inner switch
	    		default:		//OP waits for child to die
	    			wait(NULL);
	    	}//end outter switch
	    }//end if	

	    if(command.piping && !command.background){		//if pipe is TRUE & no bckgnd (2 processes)
	    	int fd[2];

	    	if (pipe(fd) == -1){	//create pipe
	    		sys_err("pipe error");
	    	}

	    	switch (fork()){
	    		case -1:
	    			sys_err("fork error");
	    		case 0:				//child
		    		switch(fork()){
			    		case -1:
			    			sys_err("fork error");
			    		case 0:		//gchild
			    			if(command.redirect_in){		//if STDIN redirect, open infile
			    				int fdin;
							if ( (fdin = open(command.infile, O_RDONLY)) < 0){
								sys_err("open error");
							}
			    				dup2(fdin, STDIN_FILENO);
			    				close(fdin);
			    			}
			    			dup2(fd[1], STDOUT_FILENO);
			    			close(fd[0]);
			    			close(fd[1]);
			    			execvp(command.argv1[0], command.argv1);
			    			sys_err("exec error");
			    			
			    		default:	//waits for gchild to die
			    			wait(NULL);
			    			if(command.redirect_out){		// if STDOUT redirect, open outfile
			    				int fdout;
			    				if (command.redirect_append){
			    					if ( (fdout = open(command.outfile, O_WRONLY | O_CREAT | O_APPEND, 0644)) < 0){
									sys_err("open error");
								}
			    				}else{
			    					if ( (fdout = open(command.outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0){
									sys_err("open error");
								}
			    				}
			    				dup2(fdout, STDOUT_FILENO);
			    				close(fdout);
			    			}
			    			dup2(fd[0], STDIN_FILENO);
			    			close(fd[0]);
			    			close(fd[1]);
			    			execvp(command.argv2[0], command.argv2);
			    			sys_err("exec error");
					}//end inner switch
				default:	//wait for child to die
					close(fd[0]);
					close(fd[1]);
					wait(NULL);
	    	}//end outter switch
	    }//end if
	      
	      if(command.piping && command.background){		//if pipe && bkgnd is TRUE
	      		int fd[2];

	      		if (pipe(fd) == -1){
	      			sys_err("pipe error");
	      		}

	      		switch (fork()){
	      			case -1:
	      				sys_err("fork error");
	      			case 0:			//child
	      				switch( fork()){
	      					case -1: 
	      						sys_err("fork error");
	      					case 0:	//gchild
	      						switch (fork()){
	      							case -1:
	      								sys_err("fork error");
	      							case 0:		//ggchild
	      								if (command.redirect_in){
	      									int fdin;
										if ( (fdin = open(command.infile, O_RDONLY)) < 0){
											sys_err("open error");
										}
	      									dup2(fdin, STDIN_FILENO);
	      									close(fdin);
	      								}
	      								dup2(fd[1], STDOUT_FILENO);
	      								close(fd[0]);
			    						close(fd[1]);
						    			execvp(command.argv1[0], command.argv1);
						    			sys_err("exec error");
						    		default:
						    			wait(NULL);
						    			if(command.redirect_out){		// if STDOUT redirect, open outfile
					    					int fdout;
						    				if (command.redirect_append){
						    					if ( (fdout = open(command.outfile, O_WRONLY | O_CREAT | O_APPEND, 0644)) < 0){
												sys_err("open error");
											}
						    				}else{
						    					if ( (fdout = open(command.outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0){
												sys_err("open error");
											}
						    				}
						    				dup2(fdout, STDOUT_FILENO);
						    				close(fdout);
						    			}
						    			dup2(fd[0], STDIN_FILENO);
						    			close(fd[0]);
						    			close(fd[1]);
						    			execvp(command.argv2[0], command.argv2);
						    			sys_err("exec error");

			      						}// end inner-inner switch
			      			default:
			      				exit(0);
	      				}//end inner switch
	      			default:
	      				wait(NULL);
			      		close(fd[0]);
			      		close(fd[1]);
	      		}//end outer switch
	      	}// end if		    
 	} //end while
	//gets(buf);
		
	exit(0);
} //end main

//function to handle error handling messages
void sys_err(const char* str){
	perror(str);
	exit(-1);
}

