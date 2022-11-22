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

// TODO: Add your implementation for classes in Commands.h 

SmallShell::SmallShell() {
// TODO: add your implementation
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}


ChpromptCommand::~ChpromptCommand(){}

void ChpromptCommand::execute() {
    char** parsed_cmd = new char*[21];
    char* new_cmd = new char[COMMAND_ARGS_MAX_LENGTH];
    int len = _parseCommandLine(cmd_line, parsed_cmd);
    size_t pos = _isRedirectionCommand(cmd_line);
    string cmd(cmd_line);
    if(pos != string::npos){
        for(int i = 1; i < 21 && parsed_cmd[i]; i++){
            char redirection1[2] = ">";
            char redirection2[3] = ">>";
            if(strcmp(parsed_cmd[i], redirection1) == 0){
                string cmd_tmp(cmd_line);
                cmd_tmp = cmd_tmp.substr(pos + 1, string::npos);
                cmd_tmp = _ltrim(cmd_tmp);
                const char* output_file = cmd_tmp.c_str();
                int fd = open(output_file, O_CREAT | O_WRONLY | O_TRUNC, 0666);
                if(fd == -1){
                    perror("smash error: open failed");
                    delete[] new_cmd;
                    _deleteParse(parsed_cmd, len);
                    return;
                };
                if(close(1) == -1){
                    perror("smash error: close failed");
                    delete[] new_cmd;
                    _deleteParse(parsed_cmd, len);
                    return;
                }
                if(dup(fd) == -1){
                    perror("smash error: dup failed");
                    delete[] new_cmd;
                    _deleteParse(parsed_cmd, len);
                    return;
                }
                if(close(fd) == -1){
                    perror("smash error: close failed");
                    delete[] new_cmd;
                    _deleteParse(parsed_cmd, len);
                    return;
                }
                break;
            }
            else if(strcmp(parsed_cmd[i], redirection2) == 0){
                string cmd_tmp(cmd_line);
                cmd_tmp = cmd_tmp.substr(pos + 2, string::npos);
                cmd_tmp = _ltrim(cmd_tmp);
                const char* output_file = cmd_tmp.c_str();
                int fd = open(output_file, O_APPEND | O_WRONLY | O_CREAT, 0666);
                if(fd == -1){
                    perror("smash error: open failed");
                    delete[] new_cmd;
                    _deleteParse(parsed_cmd, len);
                    return;
                }
                if(close(1) == -1){
                    perror("smash error: close failed");
                    delete[] new_cmd;
                    _deleteParse(parsed_cmd, len);
                    return;
                }
                if(dup(fd) == -1){
                    perror("smash error: dup failed");
                    delete[] new_cmd;
                    _deleteParse(parsed_cmd, len);
                    return;
                }
                if(close(fd) == -1){
                    perror("smash error: close failed");
                    delete[] new_cmd;
                    _deleteParse(parsed_cmd, len);
                    return;
                }
                break;
            }
        }
        cmd = cmd.substr(0, pos - 1);
        cmd = _trim(cmd);
    }
    cmd_line = cmd.c_str();
    strcpy(new_cmd, cmd_line);
    removeAmpersand(new_cmd);
    len = _parseCommandLine(new_cmd, parsed_cmd);

    if (len == 1){
        string tmp = "smash";
        this->smash->setName(tmp);
    }
    else{
        this->smash->setName(parsed_cmd[1]);
    }
    delete[] new_cmd;
    _deleteParse(parsed_cmd, len);
}

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