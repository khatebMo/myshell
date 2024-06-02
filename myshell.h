#ifndef Myshell_H
#define Myshell_H

#include "stdio.h"
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "stdlib.h"
#include "errno.h"
#include <signal.h>
#include "unistd.h"
#include <fcntl.h>
#include <stdbool.h>


#define MAX_ARGS 100
#define capacity 1024

bool last_command_exists = false;
char command_history[capacity] = "";
char previousCommandBuffer[capacity]; // Buffer to store the previous command
char temporaryCommandBuffer[capacity]; // Temporary buffer to store a command 
char cmd[capacity]; // Buffer to store the current command
char *shellPrompt; // Prompt string for the shell
int status = 0; // Status of the last executed command (0 for success, non-zero for failure)

// Constants representing pipe writer and reader
int PIPE_WRITER = 1; 
int PIPE_READER = 0;

char *argv[capacity]; // Array to store command arguments
int stdoutfd; // File descriptor for stdout (used for redirection)
int main_pid; // Process ID of the main shell process 
pid_t runningProcces = -1; // Process ID of the currently running process (or -1 if no process is running)



typedef struct commands {
    char command[capacity];
    struct commands *next;
    struct commands *prev;
} commands;


typedef struct Var
{
    char *key;
    char *value;
} Var;

typedef struct Node
{
  Var* data;
  struct Node *next;
} Node;

typedef struct List
{
  Node *head;
} List;

void add(List *list, Var *vr);
char *searchVar(const char *key);

// shell functions

char *concatWithSpace(const char *str1, const char *str2); // Function to concatenate two strings with a space in between
void tryChangeDirectory(const char *path); // Function to change the curren directory
int countWordsInString(char *str); // Function to count the number of words in a string
void promptAndStoreVariable(const char *varName); // Function to handle reading a command and storing it as a variable
int countStringArray(char **args); // Function to count the number of elements in a null-terminated array of strings
void tokenizeCommand(char *cmd); // Function to split a command string into an array of arguments
void signalHandler(int sig); // Function to handle SIGINT and SIGTSTP signals
int processRedirection(char **argv, char **outfile, int size); // Function to handle file redirection based on the command arguments
char **findPipeCommands(char **args); // Function to find and separate commands based on the pipe symbol "|"
void printEscapedArguments(char **args); // Function to print arguments with proper escaping and handling of quotes
void manageCommandHistory(char *token); // Function to handle arrow key commands and manage command history



#endif