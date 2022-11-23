#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif


//---------------------------Helper Function Implementation-----------------------------
string _ltrim(const std::string& s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for(std::string s; iss >> s; ) {
    args[i] = (char*)malloc(s.length()+1);
    memset(args[i], 0, s.length()+1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

void deleteParsedCmd(char** parsed_cmd, int len){
    if(parsed_cmd == nullptr){
        return;
    }
    for(int i = 0; i < len; i++){
        delete parsed_cmd[i];
    }
}

bool _isBackgroundCommand(const char* cmd_line) {
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos) {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&') {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}


size_t index_of_redirection(const char* cmd_line) {
    const string str(cmd_line);
    return str.find_first_of('>');
}


//------------------------------ Inherited from Command ---------------------------------------
PipeCommand::PipeCommand(const char *cmd_line, SmallShell *smash) : Command(cmd_line, smash), err_flag(false){
    string cmd_s(cmd_line);
    if(cmd_s[cmd_s.find_first_of("|") + 1] == '&'){// stderr pipe
        err_flag = true;
    }
    string right_tmp;
    if(err_flag){
        right_tmp = cmd_s.substr(cmd_s.find_first_of("|") + 2, string::npos);
    }
    else{
        right_tmp = cmd_s.substr(cmd_s.find_first_of("|") + 1, string::npos);
    }
    right_tmp = _trim(right_tmp);
    string left_tmp;
    if(cmd_s.find_first_of("|") == 0){
        left_tmp = "";
    }
    else{
        left_tmp = cmd_s.substr(0, cmd_s.find_first_of("|"));
    }
    left_tmp = _trim(left_tmp);

    left_cmd = new char [left_tmp.length()+1];
    strcpy(left_cmd, left_tmp.c_str());
    right_cmd = new char [right_tmp.length()+1];
    strcpy(right_cmd, right_tmp.c_str());
}

// Child process runs dup2
void child_fd_dup2(int fd, bool is_left, bool err_flag=false){
    int index_in_fdt = 0;
    if(is_left){
        index_in_fdt = (err_flag) ? 2 : 1;
    }
    if(dup2(fd, index_in_fdt) == -1){
        perror("smash error: dup failed");
        return;
    }
}

// RETURN ERROR VALUES !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// new child process opens pipe and executes command
void run_child_process(bool is_left, bool err_flag, int fd[]){
    int index_in_fdt = 0, to_dup = fd[0];
    if (is_left){
        to_dup = fd[1];
        index_in_fdt = (err_flag) ? 2 : 1;
    }

    setpgrp();
    child_fd_dup2(to_dup, is_left, err_flag)

    if(close(fd[0]) == -1 || close(fd[1]) == -1){
         perror("smash error: close failed");
        return;
    }

    char* cmd = (is_left) ? left_cmd : right_cmd;
    string cmd_str (cmd);

    // not sure why this is needed, will check later
    if(!cmd_str.compare("showpid")){
        char* new_str = new char[strlen(cmd) + 3];
        strcpy(new_str, cmd);
        strcat(new_str, " $");
        delete[] cmd; // do we need this?
        cmd = cmd_str;
    }
    smash->executeCommand(cmd);
    if(close(index_in_fdt) == -1){
        perror("smash error: close failed");
        return;
    }
}


void PipeCommand::execute() {
    int fd[2];
    if(pipe(fd) == -1){
        perror("smash error: pipe failed");
        return;
    }
    pid_t left_pid = fork();
    if(left_pid == -1){                             // fork failed
        perror("smash error: fork failed");
        return;
    }

    else if(left_pid == 0){                         // left child
        run_child_process(true, err_flag, fd);
        exit(0);
    }

    pid_t right_pid = fork();
    if(right_pid == -1){                            // fork failed
        perror("smash error: fork failed");
        return;
    }
    else if(right_pid == 0){                        // right child
        run_child_process(false, err_flag, fd);
        exit(0);
    }

    // parent
    int status;
    if(close(fd[0]) == -1 || close(fd[1]) == -1){
        perror("smash error: close failed");
        return;
    }
    if(waitpid(left, &status, WUNTRACED) == -1 ||
        waitpid(right, &status, WUNTRACED) == -1){
        perror("smash error: wait failed");
        return;
    }
}


//------------------------------ Inherited from BuiltInCommand ---------------------------------------

void ChpromptCommand::execute() {
    char** parsed_cmd = new char*[COMMAND_MAX_ARGS];
    char* cmd_copy = new char[COMMAND_ARGS_MAX_LENGTH];

    strcpy(cmd_copy, cmd_line);
    _removeBackgroundSign(cmd_copy);
    int len = _parseCommandLine(cmd_copy, parsed_cmd);

    if (len == 1){
        // string tmp = "smash";
        this->smash->setName("smash");
    }
    else{
        this->smash->setName(parsed_cmd[1]);
    }
    delete[] cmd_copy;
    deleteParsedCmd(parsed_cmd, len);
}

//------------------------------ SmallShell Implementation ---------------------------------------

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {
    if(command.compare("") == 0){
        return nullptr;
    }
    if (command.compare("chprompt") == 0) {
        return new ChpromptCommand(cmd_line, this);
  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
  // TODO: Add your implementation here
  // for example:
  // Command* cmd = CreateCommand(cmd_line);
  // cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
}


// TODO: Add your implementation for classes in Commands.h 

SmallShell::SmallShell() {
// TODO: add your implementation
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}