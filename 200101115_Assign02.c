//Importing necessary header files
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  // for file operations
#include <dirent.h>
#include <time.h>
#define  DELIMITADOR (char *)" "
#define  NUM 60
#define  CHAR_NUM 255  
 


 
char **tokenizer (char *input_line, char **input_redirection, char **output_redirection, int  *background_exec_mode , char  **pipe_pointer, int *pipe_int , int *t, char **append_pointer){
//  printf("1\n");
static char *parsered_string[CHAR_NUM];
//char *tok;
int n_token_idx=0;
int pipe_token=0;
 
*input_redirection = NULL;
*output_redirection = NULL;
*append_pointer=NULL;
// **pipe_pointer[CHAR_NUM];
*background_exec_mode=0;
 
if ((parsered_string[n_token_idx] = strtok (input_line, DELIMITADOR)) != NULL) {
  n_token_idx++;
//   printf("hi\n");
  while ((parsered_string[n_token_idx]=strtok ((char *)NULL, DELIMITADOR)) != NULL) {
    //   printf("2\n");
     if (strcmp (parsered_string[n_token_idx], ">>") == 0)
        *append_pointer = strtok ((char *)NULL, DELIMITADOR);
     else{
       if (strcmp (parsered_string[n_token_idx], "<") == 0)
           *input_redirection = strtok ((char *)NULL, DELIMITADOR);
       else{
           if (strcmp (parsered_string[n_token_idx], ">") == 0)
              *output_redirection = strtok ((char *)NULL, DELIMITADOR);
           else if (strcmp (parsered_string[n_token_idx], "|")==0){
                    // printf("inside parsing pipe\n");
                    *pipe_pointer = strtok ((char *)NULL, DELIMITADOR);
                    *pipe_int=n_token_idx;
            }
           else   
              n_token_idx++;
            // printf("%d\n",n_token_idx);  
       }
     }
   }
}
//  printf("4\n");
*t = n_token_idx - *pipe_int;
 return (parsered_string);
 
}
 
void _ls(const char *dir,int op_a, int op_l){
 
   struct dirent *d;
   DIR *dh = opendir(dir);
   if(!dh){
       if(errno = ENOENT){
           //if the directory is not found
           perror("Directory doesnt exist\n");
       }else{
           //if the directory is not readable then throw error and exit
           perror("Unable to read directory\n");
       }
       exit(EXIT_FAILURE);
   }
   //while the next entry is not readable we will print directory files
   while((d=readdir(dh)) != NULL){
       //if hidden files are found, we continue
       if(!op_a && d->d_name[0] == '.')
           continue;
       printf("%s ", d->d_name);
       if(op_l)
           printf("\n");   
   }
 
}

void
runsource(int pfd[],char **cmd1)	/* run the first part of the pipeline, cmd1 */
{
	int pid;	/* we don't use the process ID here, but you may wnat to print it for debugging */

	switch (pid = fork()) {

	case 0: /* child */
		dup2(pfd[1], 1);	/* this end of the pipe becomes the standard output */
		close(pfd[0]); 		/* this process don't need the other end */
		execvp(cmd1[0], cmd1);	/* run the command */
		perror(cmd1[0]);	/* it failed! */

	default: /* parent does nothing */
		break;

	case -1:
		perror("fork");
		exit(1);
	}
}

void
rundest(int pfd[],char **cmd2)	/* run the second part of the pipeline, cmd2 */
{
	int pid;

	switch (pid = fork()) {

	case 0: /* child */
		dup2(pfd[0], 0);	/* this end of the pipe becomes the standard input */
		close(pfd[1]);
        execvp(cmd2[0], cmd2);
		perror(cmd2[0]);

	default: /* parent does nothing */
		break;

	case -1:
		perror("fork");
		exit(1);
	}
}
 
int main(int argc, char* argv[]){
   int i,backg,pipe_int=0;
   char f_input[NUM];
   char *pc = NULL;
   char *f_entry , *f_exit;
   char *append_pointer;
   char **apna = NULL;
   char for_pwd[NUM];
   char cwd[PATH_MAX];
   char* username;
   getcwd(cwd,sizeof(cwd));
   char parent_cwd[PATH_MAX];
   strcpy(parent_cwd, cwd);
   printf("\n-------------Welcome to Mini Shell----------------\n");
   while(1){
       int save_in, save_out,t=0,std_err;
       save_in = dup(STDIN_FILENO);
       save_out = dup(STDOUT_FILENO);
       std_err = dup(2);
       dup2(save_in, STDIN_FILENO);
       dup2(save_out, STDOUT_FILENO);
       username = getlogin();
       char *pipe_pointer=NULL;
       getcwd(cwd,sizeof(cwd));
       printf("\n%s:\t%s  $>",username ,cwd);
       fflush(stdin);
       fflush(stdout);
       pc=(fgets(f_input,NUM,stdin));
       i=strlen(f_input);
       // printf("%d\n", i);
       f_input[i-1]=('\0');
       if(!strlen(f_input))
           continue;
       apna = tokenizer( f_input , &f_entry , &f_exit , &backg , &pipe_pointer, &pipe_int,&t,&append_pointer);


         

       // piping data
 
       if(pipe_pointer != NULL){

              char **cmd1=NULL,**cmd2=NULL;
           cmd1 = (char **)malloc(sizeof(char *)*(pipe_int+1));
           //printf("%d\n", pipe_int);
            for(int p=0;p<(pipe_int);p++){
                cmd1[p] = (char *)malloc(strlen(apna[p])+1);
                strcpy(cmd1[p],apna[p]);
            }
            cmd1[pipe_int]=NULL;
            // printf("%d\n",t);
            t++;
            cmd2 = (char **)malloc(sizeof(char *)*(t+1));
            cmd2[0] = (char *)malloc(strlen(pipe_pointer)+1);
            strcpy(cmd2[0],pipe_pointer);
            for(int p=1;p<(t);p++){
                // printf("%s\n",apna[p+pipe_int-1]);
                cmd2[p] = (char *)malloc(strlen(apna[p+pipe_int-1])+1);
                strcpy(cmd2[p],apna[p+pipe_int-1]);
                // printf("%s\n",cmd2[p]);
            }
            cmd2[t]=NULL;

           int pipefd[2];
        //    pid_t p1,p2;
           if(pipe(pipefd)==-1){
               perror("pipe");
               exit(EXIT_FAILURE);
           }
        runsource(pipefd,cmd1);
        rundest(pipefd,cmd2);
        close(pipefd[0]);close(pipefd[1]);
        int pid, status;
        while ((pid = wait(&status)) != -1){}
       }
       else
       {

       //redirecting if applicable
 
       int gid;
 
       if( f_entry != NULL){
           gid = open(f_entry,O_RDONLY);
           close(0);                       // std input is closed
           if(gid != -1)
               dup2(gid,STDIN_FILENO);
           close(gid);   
       }
       if( f_exit != NULL){
           gid = open(f_exit,O_WRONLY|O_CREAT|O_TRUNC,0755);
           close(1);                       // std onput is closed
           if(gid != -1)
               dup2(gid,STDOUT_FILENO);
           close(gid);   
       }else if(append_pointer != NULL){
           gid = open(append_pointer,O_WRONLY|O_APPEND|O_CREAT,0755);
           close(1);                       // std onput is closed
           if(gid != -1)
               dup2(gid,STDOUT_FILENO);
           close(gid);
       }
 
       // appending history to history
 
       FILE *history;
       //creating history file in the parent cwd and not in present cwd
       char *name_with_extension, *name_with_extension1;
       name_with_extension = malloc(strlen(parent_cwd) + 1 + 8); /* make space for the new string (should check the return value ...) */
       name_with_extension1 = malloc(strlen(parent_cwd) + 1 + 8); /* make space for the new string (should check the return value ...) */
       strcpy(name_with_extension, parent_cwd);                  /* copy name into the new var */
       strcpy(name_with_extension1, parent_cwd);                  /* copy name into the new var */
       strcat(name_with_extension, "/");             /* add the extension */
       strcat(name_with_extension1, "/");             /* add the extension */
       strcat(name_with_extension, "history");             /* add the extension */
       strcat(name_with_extension1, "readme");             /* add the extension */
       history = fopen(name_with_extension,"a");
       int count=0,count2=0;
       char **temp = apna; // temp is a pointer to a *pointer*, not a pointer to a *char*
       while (*temp != NULL){
           fprintf(history,"%s ", *temp);
           temp++;
       }
       fprintf(history,"\n");
       fclose(history);
      
       // EXIT command
 
       if( strcmp (apna[0],"exit")==0){
           printf("\n\t END OF YOUR MINI SHELL\n\n");
           remove(name_with_extension);     // destroys the history file which was created
           return -1;
       }
 
       // PWD command
 
       else if(strcmp(apna[0],"pwd")==0){
           //printf("inside pwd command\n");
           getcwd(for_pwd,NUM);
           printf("%s",for_pwd);
       }
 
       // cd command
 
       else if(strcmp(apna[0],"cd")==0){
           if(apna[1]==NULL){
               chdir (getenv("HOME"));
           }
           else
               if(chdir(apna[1])==-1){
                   printf("\n%s", strerror(errno));
                   // exit(-1);
               }
               else
               //printf("%s",apna[1]);
               chdir(apna[1]);
       }
 
       // ls command
 
       else if(strcmp(apna[0],"ls")==0){
           int op_a=0,op_l=0;
           if(apna[1]==NULL);
           else{
               if(apna[1][0]=='-'){
                   //checking if option is passed
                   //options supporting: a,l
                   char *p = (char*)(apna[1]+1);
                   while(*p){
                       if(*p == 'a')
                           op_a=1;
                       else if(*p == 'l') 
                       {
                           op_l=1;
                           // printf("\'l\' detected\n");
                       }
                       else{
                           perror("Option not available\n");
                           exit(EXIT_FAILURE);
                       }
                       p++;       
                   }
               }
           }   
           _ls(".",op_a,op_l);
           printf("\n");
       }
 
       // clear command
 
       else if(strcmp(apna[0],"cls")==0 || strcmp(apna[0],"clear")==0){
           system("clear");
       }
 
       // cat command
 
       else if(strcmp(apna[0],"cat")==0){
           int fdold,count;
           char buffer[2048]; //characer buffer to store the bytes
           fdold=open(apna[1], O_RDONLY);
           if(fdold==-1){
               printf("cannot open file\n");
           }else{
               while((count=read(fdold,buffer,sizeof(buffer)))>0){
                   printf("%s",buffer);
               }
           }
          
       }
 
       // touch command
 
       else if(strcmp(apna[0],"touch")==0){
           FILE *fptr;
           fptr = fopen(apna[1],"w");
           fclose(fptr);
       }    
 
       // remove command
 
       else if(strcmp(apna[0],"rm")==0){
           if (remove(apna[1]) == -1){
               printf("Error :  %s",strerror(errno));
           }
       }
 
       // creating a directory
 
       else if(strcmp(apna[0],"dir")==0){
           if(mkdir(apna[1],0777)==-1){
               printf("Error :  %s",strerror(errno));
           }
       }
 
       // removing a directory
 
       else if(strcmp(apna[0],"rmdir")==0){
           if(rmdir(apna[1])==-1){
               printf("Error :  %s",strerror(errno));
           }
       }
 
       // displaying time
 
       else if(strcmp(apna[0],"date")==0){
           time_t current_time = time(NULL);
           struct tm *tm = localtime(&current_time);
           printf("\nCurrent Date and Time:\n");
           printf("%s\n", asctime(tm));
       }
 
       // history command - concatnates history to the terminal
 
       else if(strcmp(apna[0],"history")==0){
           int c_for_history;
           FILE* hisptr;
           hisptr = fopen(name_with_extension,"r");
           while((c_for_history =fgetc(hisptr) )!= EOF){
               putchar(c_for_history);
           }
           //printf("\n");
       }

       else if(strcmp(apna[0],"help")==0){
            FILE *fptr;
            char filename[100];
            int c;
            // Open file
            fptr = fopen(name_with_extension1, "r");
            // Read contents from file
            c = fgetc(fptr);
            while (c != EOF)
            {
                printf("%c", c);
                c = fgetc(fptr);
            }

            fclose(fptr);
       }
 
        //setenv command

       else if(strcmp(apna[0],"setenv")==0){
                                                            /// only accepts format of setenv TERM = vt100; (include the space)
        //    printf("%s %s", apna[1],apna[3]);
        setenv(apna[1],apna[3], 0);                        /// only sets doesnt overwrite
       }

        else if(strcmp(apna[0],"unsetenv")==0){
                                                            /// only accepts format of setenv TERM = vt100; (include the space)
        //    printf("%s %s", apna[1],apna[3]);
        unsetenv(apna[1]);                 /// only sets doesnt overwrite
       }

       // execv commands
 
       else{
               // Forking a child
           pid_t pid = fork();
      
           if (pid == -1) {
               printf("\n%s", strerror(errno));
           } else if (pid == 0) {
               if (execvp(apna[0], apna) < 0) {
                   printf("\nCould not execute command..");
               }
               exit(0);
           } else {
               // waiting for child to terminate
               wait(NULL);
           }
       }

       }


       dup2(save_in, STDIN_FILENO);
       dup2(save_out, STDOUT_FILENO);

   }
}