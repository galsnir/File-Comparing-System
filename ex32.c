// Gal Snir 313588279

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <wait.h>

#define MAX_SIZE 151
#define OUTPUT "output.txt" // The file where each student c file will be printed into
#define RESULT "results.csv" // The file where the final results will be printed into

// This function goes through all the folders inside the input folder sends then to the searchFolder function
// and then grade the it according to it's results
int searchFolder(char* search_path,char* input_path,char* output_file);

// This function gets a path of a folder and check it's content for c file, if it finds a folder
// it goes through it and search it for a c file recursively
int searchFiles(char* dir_path,char* input_path,char* correctOutput);

// This function gets a file compiles it and return 0 if it has compiled well or -1 if it had a compilation error
int compileFile(char* file_name, char* path);

// This function gets a path of an a.out file runs it and return 0 if it hasn't timed out or -1 if it had timed out
int runFile(char* input_path,char* output_file);

/* This function gets a source text file and a target text file compare them using the function of part A
and returns it's result */
int comp(char* source,char* target);

// This function print an error call
void error();

/* This program gets a path and searches through all it's direcotries to find c file when it see one it compiles it
   runs it and compare it's results with an correct output file and prints the results of each directory into a
   results csv file */
int main(int argc, char* argv[] )
{
    int i = 0;
    char *temp;
    int conf_file;
    char buffer[MAX_SIZE*3]; // This buffer will hold all the text found in the conf file
    char line[3][MAX_SIZE]; // This array of lines will hold all the lines found in the conf file
    if (argc != 2)
        error();
    conf_file = open(argv[1], O_RDONLY); // we will read the input file and copy it's content into the conf file
    if (conf_file < 0)
        error();

    read(conf_file,buffer,MAX_SIZE*3); // we will read the copy the conf file into the buffer

    // we split the buffer into lines and insert their content into the the lines array
    temp = strtok(buffer,"\n");
    while (temp != NULL)
    {
        strcpy(line[i],temp);
        temp = strtok(NULL, "\n");
        i++;
    }

    // We send the lines into the searchFile function that will compile run and compare the files in the input folder
    searchFiles(line[0],line[1],line[2]);
    unlink("./runFile.out"); // Deleting the run file of the program

    return 0;
}

// This function goes through all the folders inside the input folder sends then to the searchFolder function
// and then grade the it according to it's results
int searchFiles(char* dir_path,char* input_path,char* correct_output)
{
    DIR *dir;
    int res;
    int cop_res;
    int isFirst = 1; // Checks if this is the first line for /n printing
    struct dirent *dirnt;
    char output_file[MAX_SIZE];
    char inside_path[MAX_SIZE];
    if ((dir = opendir(dir_path)) == NULL) // We open the input directory
        error();

    // We go through all the dirctories in the input folder
    while (dirnt = readdir(dir))
    {
        strcpy(inside_path, dir_path);
        if (strcmp(dirnt->d_name, "..") != 0 && strcmp(dirnt->d_name, ".") != 0)
        {
            // If it is a folder we send it to the searchFolder function and grade it
            if (dirnt->d_type == DT_DIR)
            {
                strcpy(output_file, dir_path);
                strcat(strcat(strcat(strcat(output_file, "/"),dirnt->d_name),"/"), OUTPUT);
                strcat(strcat(inside_path, "/"), dirnt->d_name);
                res = searchFolder(inside_path, input_path, output_file);
                int results_file = open(RESULT,O_WRONLY | O_CREAT | O_APPEND , S_IRWXU | S_IRWXG | S_IRWXO); // opening the results file
                char buffer[MAX_SIZE];
                strcpy(buffer,dirnt->d_name);

                // If it isn't the first line we print the /n charachter
                if (isFirst == 0)
                {
                    write(results_file,"\n",1);
                }

                // If the was no c file in the folder we score it 0
                if (res == 0)
                {
                    strcat(buffer,",0,NO_C_FILE");
                    write(results_file,buffer,strlen(buffer));
                }

                    // If the c file in the folder compiled with an error we score it 20
                else if (res == 1)
                {
                    strcat(buffer,",20,COMPILATION_ERROR");
                    write(results_file,buffer,strlen(buffer));
                }

                    // If the was no a.out fil got timed out we score it 40
                else if (res == 2)
                {
                    strcat(buffer,",40,TIMEOUT");
                    write(results_file,buffer,strlen(buffer));
                }

                    // If the was c file compile well and the a.out file ran properly we check the file results
                else if (res == 3)
                {
                    // We compare the output file the correct output file
                    cop_res = (comp(output_file, correct_output));

                    // If the output file and the correct output file were diffrent we will score it 60
                    if (cop_res == 2)
                    {
                        strcat(buffer,",60,BAD_OUTPUT");
                        write(results_file,buffer,strlen(buffer));
                    }

                        // If the output file and the correct output file were alike we will score it 80
                    else if (cop_res == 3)
                    {
                        strcat(buffer,",80,SIMILAR_OUTPUT");
                        write(results_file,buffer,strlen(buffer));
                    }

                        // If the output file and the correct output file were similar we will score it 100
                    else if (cop_res == 1)
                    {
                        strcat(buffer,",100,GREAT_JOB");
                        write(results_file,buffer,strlen(buffer));
                    }
                }
            unlink(output_file); //  Deleting all the output files
            isFirst = 0;
            }
        }
    }
}

// This function gets a path of a folder and check it's content for c file, if it finds a folder
// it goes through it and search it for a c file recursively
int searchFolder(char* search_path,char* input_path,char* output_file)
{
    DIR* dir;
    int res = 0;
    int isCFile = 0;
    struct dirent *dirnt;
    char new_path[MAX_SIZE];
    char temp[MAX_SIZE];
    if ((dir = opendir(search_path)) == NULL) // We open the input directory
        error();
    while (dirnt = readdir(dir))
    {
        if (strcmp(dirnt->d_name, "..") != 0 && strcmp(dirnt->d_name, ".") != 0)
        {
            isCFile = 0;
            strcpy(new_path, search_path);
            strcpy(temp, dirnt->d_name);
            if ((temp[strlen(temp) - 2] == '.') && (temp[strlen(temp) - 1] == 'c'))
                isCFile = 1;

            // if we find a folder we go through it and search it for a c file recursively
            if (dirnt->d_type == DT_DIR)
            {
                strcat(strcat(new_path, "/"), dirnt->d_name);
                res = searchFolder(new_path, input_path,output_file);
            }

            // If we found a c file we will compile it and then run it's a.out
            else if (isCFile)
            {
                // If the file compiled well but didn't but the a.out file timed out we return 2
                if (compileFile(dirnt->d_name, new_path) == 0)
                {
                    res = 2;
                    // If the file compiled well but and the a.out file haven't timed out we return 3
                    if (runFile(input_path,output_file) == 0)
                    {
                        res = 3;
                    }
                }

                // If the file haven't compiled well we return 1
                else
                {
                    res = 1;
                }

                return res;
            }
        }
    }
    //unlinkfiles(new_path);
    return res;
}

// This function gets a file compiles it and return 0 if it has compiled well or -1 if it had a compilation error
int compileFile(char* file_name, char* path)
{
    int status;
    char compile_path[MAX_SIZE];
    strcpy(compile_path,path);
    strcat(strcat(compile_path,"/"),file_name);
    char* args[] = {"gcc","-o","runFile.out",compile_path,NULL};

    // We fork the process so the execvp will work in a different process
    pid_t pid = fork();
    if (pid == -1)
    {
        error();
    }

    // In the son process we will execute the compilation of the c file
    else if (pid == 0)
    {
        if (execvp(args[0],args) < 0)
            error();
        exit(0);
    }

        // The main process will wait for the execvp process to complete
    else if (pid > 0)
    {
        waitpid(pid,&status,0);

        if (WEXITSTATUS(status) == EXIT_FAILURE)
        {
            return -1;
        }

        else
        {
            return 0;
        }
    }
}


// This function gets a path of an a.out file runs it and return 0 if it hasn't timed out or -1 if it had timed out
int runFile(char* input_path,char* output_file)
{
    char *args[] = {"./runFile.out", NULL};
    pid_t first_pid;

    // We fork the process so the execvp will work in a different process
    pid_t run_pid = fork();
    if (run_pid == -1)
    {
        error();
    }

    // In the son process we will run the a.out file
    else if (run_pid == 0)
    {
        int read = open(input_path, O_RDONLY); // We use file descriptor the read the input file
        int write = open(output_file, O_WRONLY | O_CREAT,S_IRWXU | S_IRWXG | S_IRWXO); // We use file descriptor the write to the output file
        dup2(read, 0); // We set the program to read form the input file instead of the stdin
        dup2(write, 1); // We set the program to write to the output file instead of the stdout
        if (execvp(args[0], args) < 0) // We run the a.out file
            error();
        close(read);
        close(write);
        exit(0);
    }

    // We fork the process so the could set a process to sleep for 5 seconds and check if it finished
    // before the a.out file has finished running
    pid_t timeout_pid = fork();
    if (timeout_pid == -1)
    {
        error();
    }

    // In the son process will sleep for 5 seconds
    else if (timeout_pid == 0)
    {
        sleep(5);
        exit(0);
    }

    // We will wait fo the first process to finish
    do
    {
        first_pid = wait(NULL);
    }

        // We will keep on waiting until the a.out or the timeout process will return
    while (first_pid != timeout_pid && first_pid != run_pid);

    // If the a.out process finished before the timeout process we will return 0
    if (first_pid == run_pid)
    {
        kill(timeout_pid, SIGKILL);
        return 0;
    }

        // If the timeout process finished before the a.out process we will return 1
    else
    {
        kill(run_pid, SIGKILL);
        return 1;
    }
}


// This function gets a source text file and a target text file compare them using the function of part A
// and returns it's result
int comp(char* source,char* target)
{
    int status;
    char* args[] = {"./comp.out",source,target,NULL};

    // We fork the process so the execvp will work in a different process
    pid_t pid = fork();
    if (pid == -1)
    {
        error();
    }

    // In the son process we will run the comp.out file
    if (pid == 0)
    {
        if (execvp(args[0],args) < 0)
            error();
        exit(0);
    }

    // The parent process will wait until the son process will finish and return it's result
    if (pid > 0)
    {
        waitpid(pid,&status,WUNTRACED);
        return WEXITSTATUS(status);
    }
}

void error()
{
    char * errorMassege ={"Error in system call\n"};
    write(2, errorMassege ,strlen(errorMassege));
    exit(1);
}
