
#include "myshell.h"

commands   *historyRoot; 
List variables;

void add(List *list, Var *vr) {
    Node *new_node = (Node *)malloc(sizeof(*new_node));
    if (new_node == NULL) {
        return;
    }
    new_node->data = vr;
    new_node->next = NULL;

    if (list->head == NULL) {
        list->head = new_node;
    } else {
        Node *cur = list->head;
        while (cur->next != NULL) {
            cur = cur->next;
        }
        cur->next = new_node;
    }
}


char *searchVar(const char *key) {
    Node *cur = variables.head;
    while (cur != NULL) {
        if (strcmp(cur->data->key, key) == 0) {
            return cur->data->value;
        }
        cur = cur->next;
    }
    return NULL;
}


// Function to concatenate two strings with a space in between
char *concatWithSpace(const char *str1, const char *str2) {
    size_t n_str1 = strlen(str1), n_str2 = strlen(str2); // Calculate the lengths of the input strings
    char *result = malloc((n_str1 + n_str2 + 2) * sizeof(char));  // Allocate memory for the concatenated string with space in between
    if (result == NULL) {
        return NULL; // Return NULL in case of memory allocation failure
    }
    strcpy(result, str1); // Copy the first string into the new string

    // Concatenate a space and the second string to the new string
    strcat(result, " ");
    strcat(result, str2);
    return result;// Return the concatenated string
}

// Function to change the curren directory
void tryChangeDirectory(const char *path) {
    int status = chdir(path);
    if (status == -1) { // Attempt to change the current directory
        perror("chdir() failed"); // Print an error message with perror
        exit(EXIT_FAILURE); // Exit the program with failure status
    }
    // Print a success message after successfully changing the directory
}

// Function to count the number of words in a string
int countWordsInString(char *str){
    int counter = 0;
    int inWord = 0;
    for (int i = 0; str[i] != '\0'; i++){ // Iterate through the characters in the string
        if (str[i] == ' '){ // Check if the current character is a space
            inWord = 0; // Not in a word if a space is encountered
        }
        else if (!inWord){ // Check if the current character is not a space and the previous character was a space (or it's the end of the string)
            counter++; // Increment the count when the start of a new word is detected
            inWord = 1;
        }
        // Check if the current character is not a space and the next character is a space or the end of the string
        if (str[i] != ' ' && (str[i + 1] == ' ' || str[i + 1] == '\0')){
            inWord = 0; // Not in a word if the end of a word is detected
        }
    }
    return counter; // Return the count of words
}

// Function to handle reading a command and storing it as a variable
void promptAndStoreVariable(const char *varName) {
    char input[capacity];
    printf("%s = ", varName); // Prompt user for input

    // Read input from the user
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = '\0'; // Remove the newline character

    // Allocate memory for a new variable
    Var *vr = malloc(sizeof(Var));
    if (vr == NULL) {
        fprintf(stderr, "Memory allocation error.\n");
        return;
    }

    // Allocate memory for the key
    size_t keyLen = strlen(varName) + 2; 
    vr->key = malloc(keyLen * sizeof(char));
    if (vr->key == NULL) {
        fprintf(stderr, "Memory allocation error.\n");
        free(vr);
        return;
    }

    // Construct the key by prepending '$' to the variable name
    snprintf(vr->key, keyLen, "$%s", varName);

    // Duplicate and store the input as the value
    vr->value = strdup(input);
    if (vr->value == NULL) {
        fprintf(stderr, "Memory allocation error.\n");
        free(vr->key);
        free(vr);
        return;
    }

    // Add the new variable to the linked list
    add(&variables, vr);
}

// Function to count the number of elements in a null-terminated array of strings
int countStringArray(char **args){
    int counter = 0;

    // Iterate through the array until a NULL pointer is encountered
    for (char **p = args; *p != NULL; ++p){
        counter++;
    }
    return counter;
}

// Function to split a command string into an array of arguments
void tokenizeCommand(char *cmd){
    char *token = strtok(cmd, " ");
    int i = 0;

    // Iterate through the tokens and populate the argv array
    while (i < MAX_ARGS && token != NULL){
        argv[i] = token;
        token = strtok(NULL, " ");
        i++;
    }
    argv[i] = NULL; // Ensure the last element of the array is NULL
}

// Function to handle SIGINT and SIGTSTP signals
void signalHandler(int sig){
    if (sig == SIGTSTP) { // Handle SIGTSTP signal
        exit(0);
    }
    if (sig == SIGINT) { // Handle SIGINT signal
        if (getpid() == main_pid) { // Check if the process ID is the main process ID
            printf("\nYou typed Control-C!\n");
            fflush(stdout);
            strcpy(cmd, "^C"); // Update cmd with "^C"

            // Print the prompt and a space after catching SIGINT
            write(STDIN_FILENO, shellPrompt, strlen(shellPrompt)+1);
            write(STDIN_FILENO, " ", 1);
        }
    }
}

// Function to handle file redirection based on the command arguments
int processRedirection(char **argv, char **outfile, int size){
    if (size < 2) { // Check if there are enough arguments for redirection
        return -1;
    }

    // Get the redirection symbol and filename
    char *redir = argv[size - 2];
    char *filename = argv[size - 1];

    // Check for output redirection
    if (strcmp(redir, ">") == 0 || strcmp(redir, ">>") == 0){
        *outfile = filename;
        return STDOUT_FILENO;
    }

    // Check for error output redirection
    else if (!strcmp(redir, "2>")){
        *outfile = filename;
        return STDERR_FILENO;
    }

    // Check for input redirection
    else if (!strcmp(redir, "<")){
        *outfile = filename;
        return STDIN_FILENO;
    }
    return -1; // No redirection or unsupported redirection, return -1
}

// Function to find and separate commands based on the pipe symbol "|"
char **findPipeCommands(char **args){
    while (*args != NULL){
        if (strcmp(*args, "|") == 0){
            return args;
        }
        args++;
    }
    return NULL;
}

// Function to print arguments with proper escaping and handling of quotes
void printEscapedArguments(char **args){
    if (args == NULL) { // Check if args is NULL
        fprintf(stderr, "Error: args is NULL\n");
        return;
    }

    // Iterate through each argument
    while (*args != NULL){
        int escaped = 0;
        int in_quote = 0;
        int in_single_quote = 0;
        int i = 0;

        // Iterate through each character in the argument
        while ((*args)[i] != '\0'){
            if (escaped) {
                switch ((*args)[i]) {// Handle escaped characters
                    case ' ':
                        putchar(' ');
                        break;
                    case '\\':
                        putchar('\\');
                        break;
                    case '"':
                        putchar('"');
                        break;
                    case '\'':
                        putchar('\'');
                        break;
                    default:
                        fprintf(stderr, "Error: unexpected escaped character\n");
                        return;
                }
                escaped = 0;
            } else if (in_quote) { // Handle characters inside double quotes
                if ((*args)[i] == '\\') {
                    escaped = 1;
                } else if ((*args)[i] == '"') {
                    in_quote = 0;
                } else {
                    putchar((*args)[i]);
                }
            } else if (in_single_quote) { // Handle characters inside single quotes
                if ((*args)[i] == '\'') {
                    in_single_quote = 0;
                } else {
                    putchar((*args)[i]);
                }
            } else { // Handle characters outside quotes
                if ((*args)[i] == '\\') {
                    escaped = 1;
                } else if ((*args)[i] == '"') {
                    in_quote = 1;
                } else if ((*args)[i] == '\'') {
                    in_single_quote = 1;
                } else if ((*args)[i] == ' ') {
                    putchar('\\');
                    putchar(' ');
                } else {
                    putchar((*args)[i]);
                }
            }
            i++;
        }

        putchar(' '); // Add space between arguments
        args++;
    }
    putchar('\n'); // Print newline at the end
}

// Function to execute a command with redirection and other features
int execute(char **args){
    char *outfile;
    int i = countStringArray(args), fd, amper, redirect = -1, rv = -1;
    pid_t pid;
    int hasPip = 0;
    char **pipPointer = findPipeCommands(args);
    int pipe_fd[2];

    if (pipPointer != NULL){
        // Handling pipes
        hasPip = 1;
        *pipPointer = NULL;
        i = countStringArray(args);
        pipe(pipe_fd);

        if (fork() == 0){
            close(pipe_fd[PIPE_WRITER]);
            close(STDIN_FILENO);
            dup(pipe_fd[PIPE_READER]);
            execute(pipPointer + 1);
            exit(0);
        }

        stdoutfd = dup(STDOUT_FILENO);
        dup2(pipe_fd[PIPE_WRITER], STDOUT_FILENO);
    }

    // Various built-in commands
    if (args[0] == NULL)
        return 0;

    // Variable assignment 
    if (args[0][0] == '$' && i >= 3){
        Var *var = (Var *)malloc(sizeof(Var));
        var->key = malloc((strlen(args[0]) + 1));
        var->value = malloc((strlen(args[2]) + 1));

        strcpy(var->key, args[0]);
        strcpy(var->value, args[2]);

        add(&variables, var);
        return 0;
    }

    // 'read' command
    if(!strcmp(args[0], "read")){
        promptAndStoreVariable(argv[1]);
        return 0;
    }

    // Change directory
    if (!strcmp(args[0], "cd")){
        tryChangeDirectory(args[1]);
        return 0;
    }

    // Set shell prompt
    if (!strcmp(args[0], "prompt")){
        free(shellPrompt);
        shellPrompt = malloc(strlen(args[2]) + 1);
        strcpy(shellPrompt, args[2]);
        return 0;
    }

    // Echo command
    if (!strcmp(args[0], "echo")){
        char **echo_args = args + 1;
        if (!strcmp(*echo_args, "$?")){
            printf("%d\n", status);
            return 0;
        }

        // Echo each argument, replacing variables if necessary
        while (*echo_args){
            if (*echo_args && *echo_args[0] == '$'){
                char *v = searchVar(*echo_args);
                if (v != NULL)
                    printf("%s ", v);
            }
        
            else
                printf("%s ", *echo_args);

            echo_args++;
        }
        printf("\n");
        return 0;
    }

    // Background execution check
    if (!strcmp(args[i - 1], "&")){
        amper = 1;
        args[i - 1] = NULL;
    }
    else
        amper = 0;

    // Process redirection
    int redirectFd = processRedirection(args, &outfile, i);

    // Execute the command
    if ((runningProcces = fork()) == 0){
        if (redirectFd >= 0){
            // Handle output redirection
            if (!strcmp(args[i - 2], ">>")){
                fd = open(outfile, O_WRONLY | O_CREAT, 0644);
                lseek(fd, 0, SEEK_END);
            }
            else if (!strcmp(args[i - 2], ">") || !strcmp(args[i - 2], "2>")){
                fd = creat(outfile, 0660);
            }
            else{
                fd = open(outfile, O_RDONLY);
            }
            close(redirectFd);
            dup(fd);
            close(fd);
            args[i - 2] = NULL;
        }
        execvp(args[0], args);
    }
    if (amper == 0){ // Wait for the child process to finish (if not a background process)
        wait(&status);
        rv = status;
        runningProcces = -1;
    }
    if (hasPip){  // Restore standard output and close pipe in case of a piped command
        close(STDOUT_FILENO);
        close(pipe_fd[PIPE_WRITER]);
        dup(stdoutfd);
        wait(NULL);
    }

    return rv;
}

// Function to handle arrow key commands and manage command history
void manageCommandHistory(char *token) {
    static char currentCmd[1024] = "";
    static commands   *historyRoot  = NULL;

    if (historyRoot == NULL) { // Initialize commands   linked list if not done already
        historyRoot = (commands   *) malloc(sizeof(commands  ));
        historyRoot->prev = NULL;
        historyRoot->next = NULL;
    }
    if (token != NULL && strcmp(token, "\033[A") == 0) { // Handle arrow up key
        if (historyRoot->prev != NULL) {
            strcpy(cmd, historyRoot->prev->command);
            historyRoot = historyRoot->prev;
            token = strtok(cmd, " ");
        } else {
            strcpy(cmd, currentCmd);
        }
    } else if (token != NULL && strcmp(token, "\033[B") == 0) { // Handle arrow down key
        if (historyRoot->next != NULL) {
            strcpy(cmd, historyRoot->next->command);
            historyRoot = historyRoot->next;
            token = strtok(cmd, " ");
        } else {
            strcpy(cmd, "");
        }
    } else if (token != NULL && strlen(cmd) != 0) { // Save current command in commands   linked list
        strcpy(historyRoot->command, currentCmd);
        if (historyRoot->next == NULL) { // Create a new node in the commands   linked list if not done already
            commands   *next = (commands  *) malloc(sizeof(commands  ));
            next->prev = historyRoot;
            next->next = NULL;
            historyRoot->next = next;
        }
        // Move to the next node in the commands   linked list
        historyRoot = historyRoot->next;
        strcpy(historyRoot->command, "");
    }
    strcpy(currentCmd, cmd);
}


int main()
{
    main_pid = getpid();
    signal(SIGINT, signalHandler);
    signal(SIGQUIT, signalHandler);
    shellPrompt = malloc(7);
    strcpy(shellPrompt, "hello:");
    int cmdPosition = -1;
    char *b;
    int i;
    char ch;
    char *token;

    historyRoot = (commands  *) malloc(sizeof(commands  ));
    historyRoot->next = NULL;
    historyRoot->prev = NULL;

    while (1){
        printf("%s ", shellPrompt);
       
        fgets(cmd , capacity, stdin);
        cmd[strlen(cmd) - 1] = '\0';
        strcpy(temporaryCommandBuffer, cmd);
     
        if (!strcmp(cmd, "quit"))
            break;

        if (strcmp(cmd, "!!") == 0) {
            if (!last_command_exists) {
                printf("No previous command\n");
                continue;
            }
            printf("%s\n", command_history);
            system(command_history);
            continue;
        }
        // Save command to history
        strcpy(command_history, cmd);
        last_command_exists = true;         

        if (cmd[0] == 'i' && cmd[1] == 'f'){
            // take all the command ecxept the first argument
            strcpy(cmd, temporaryCommandBuffer + 2);
            char then[1024];
            printf("> ");
            fgets(then, 1024, stdin);
            if (then[0] == 't' && then[1] == 'h' && then[2] == 'e' && then[3] == 'n'){
                char ThenCommand[1024];
                printf("> ");
                fgets(ThenCommand, 1024, stdin);
                char NextCommand[1024];
                printf("> ");
                fgets(NextCommand, 1024, stdin);
                
                if (NextCommand[0] == 'f' && NextCommand[1] == 'i'){
                    if (!system(cmd)){ // check if if statement is true, and execute the command
                        strcpy(cmd, " ");
                        system(ThenCommand);                             
                    }
                    else{
                        strcpy(cmd, " ");
                        continue;
                    }
                }
                // check if there is an else statement
                else if (NextCommand[0] == 'e' && NextCommand[1] == 'l' && NextCommand[2] == 's' && NextCommand[3] == 'e'){
                    char elseCommand[1024];
                    printf("> ");
                    fgets(elseCommand, 1024, stdin);
                    char fi[1024];
                    printf("> ");
                    fgets(fi, 1024, stdin);
                    if (fi[0] == 'f' && fi[1] == 'i'){ // check if if statement is true, and execute the command
                        if (!system(cmd)){
                            strcpy(cmd, " ");
                            system(ThenCommand);
                        }
                        else{ // check if if statement is false, and execute the command
                            strcpy(cmd, " ");
                            system(elseCommand);
                        }
                    }
                }
            }
            else{
                printf ("syntax error\n");
                continue;
            }
        }
            
        token = strtok(previousCommandBuffer, " ");
        manageCommandHistory(token);
        tokenizeCommand(cmd);

        status = execute(argv);
    }
}
