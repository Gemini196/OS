#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <fcntl.h>
#include <sys/time.h>

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


//---------------------------Helper Function Implementation-------------------------------------------
string _ltrim(const std::string &s) {
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string &s) {
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string &s) {
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char *cmd_line, char **args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for (std::string s; iss >> s;) {
        args[i] = (char *) malloc(s.length() + 1);
        memset(args[i], 0, s.length() + 1);
        strcpy(args[i], s.c_str());
        args[++i] = nullptr;
    }
    return i;

    FUNC_EXIT()
}

void deleteParsedCmd(char **parsed_cmd, int len) {
    if (parsed_cmd == nullptr) {
        return;
    }
    for (int i = 0; i < len; i++) {
        free(parsed_cmd[i]);
    }
    delete[] parsed_cmd;
}

bool _isBackgroundCommand(const char *cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char *cmd_line) {
    const string str(cmd_line);
    // find last character other than spaces
    size_t idx = str.find_last_not_of(WHITESPACE);
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


size_t index_of_redirection(const char *cmd_line) {
    const string str(cmd_line);
    return str.find_first_of('>');
}

bool _isNumber(const string& str) {
    return str.find_first_not_of("-0123456789") == std::string::npos;
//    for (unsigned int i = 0; i < str.length(); i++) {
//        char tmp = str[i];
//        if (isdigit(tmp) == false){
//            cout << str[i] << endl;
//            return false;
//        }
//    }
//    return true;


}


//------------------------------ Inherited from Command ----------------------------------------------

void ExternalCommand::execute() {
    char *cmd_copy = new char[COMMAND_ARGS_MAX_LENGTH];
    strcpy(cmd_copy, cmd_line);
    _removeBackgroundSign(cmd_copy);

    int child_status = 0;
    pid_t pid;
    pid = fork();

    if (pid == -1) {  // fork failed
        perror("smash error: fork failed");
        delete[] cmd_copy;
        return;
    }
    // child process
    if (pid == 0) {
        setpgrp();
        char *bash_args[] = {(char *) "/bin/bash", (char *) "-c", cmd_copy, nullptr};
        if (execv("/bin/bash", bash_args) == -1) {
            perror("smash error: execv failed");
            delete[] cmd_copy;
            exit(1); //exit child's process
        }
        delete[] cmd_copy;
        exit(0);
    }

    // parent
    smash->smash_pid = getpid();
    if (_isBackgroundCommand(cmd_line)) //do in background
    {
        // add the job to the jobs list (its parent does this)
        if (pid) {
            int status;
            if (waitpid(pid, &status, WNOHANG) == -1) {
                perror("smash error: wait failed");
                delete[] cmd_copy;
                return;
            }
            time_t i_time;
            if (time(&i_time) == ((time_t) -1)) {
                perror("smash error: time failed");
                delete[] cmd_copy;
                return;
            }

            string cmd(cmd_line);
            // JobsList::JobEntry new_job(smash->jobs_list->curr_max_job_id, pid, i_time, cmd, false);
            smash->jobs_list->removeFinishedJobs();
            smash->jobs_list->addJob(cmd_line, pid, i_time, false);
        }

    }
    else { // do in foreground
        smash->fg_pid = pid;
        if (waitpid(pid, &child_status, WUNTRACED) == -1) {
            perror("smash error: wait failed");
            delete[] cmd_copy;
            return;
        }
        smash->fg_pid = smash->smash_pid;
    }
    delete[] cmd_copy;
}


//==============================================================================================//
//-------------------------------SPECIAL COMMANDS IMPLEMENTATION--------------------------------//
//==============================================================================================//


//IO redirection
PipeCommand::PipeCommand(const char *cmd_line, SmallShell *smash) : Command(cmd_line, smash), err_flag(false) {
    string cmd_s(cmd_line), right_tmp, left_tmp;
    if (cmd_s[cmd_s.find_first_of('|') + 1] == '&') // stderr pipe
        err_flag = true;

    if (err_flag)
        right_tmp = cmd_s.substr(cmd_s.find_first_of('|') + 2, string::npos);
    else
        right_tmp = cmd_s.substr(cmd_s.find_first_of('|') + 1, string::npos);

    right_tmp = _trim(right_tmp);
    if (cmd_s.find_first_of('|') == 0)
        left_tmp = "";
    else
        left_tmp = cmd_s.substr(0, cmd_s.find_first_of('|'));

    left_tmp = _trim(left_tmp);

    left_cmd = new char[left_tmp.length() + 1];
    right_cmd = new char[right_tmp.length() + 1];

    strcpy(left_cmd, left_tmp.c_str());
    strcpy(right_cmd, right_tmp.c_str());
}

PipeCommand::~PipeCommand()
{
    delete[] left_cmd;
    delete[] right_cmd;
}

// Child process runs dup2
void child_fd_dup2(int fd, bool is_left, bool err_flag = false) {
    int index_in_fdt = 0;
    if (is_left) {
        index_in_fdt = (err_flag) ? 2 : 1;
    }
    if (dup2(fd, index_in_fdt) == -1) {
        perror("smash error: dup failed");
        return;
    }
}

// new child process opens pipe and executes command
int PipeCommand::run_child_process(bool is_left, int fd[]) {
    int index_in_fdt = 0, to_dup = fd[0];            // define fdt indexes
    if (is_left) {                                    // change fdt indexes for left
        to_dup = fd[1];
        index_in_fdt = (err_flag) ? 2 : 1;
    }
    setpgrp();
    child_fd_dup2(to_dup, is_left, err_flag);

    if (close(fd[0]) == -1 || close(fd[1]) == -1) {
        perror("smash error: close failed");
        return -1;
    }

    char *cmd = (is_left) ? left_cmd : right_cmd;
    
    //string cmd_str(cmd);

    /*
    // not sure why this is needed, will check later
    if (!cmd_str.compare("showpid")) {
        char *new_cmd = new char[strlen(cmd) + 3];
        strcpy(new_cmd, cmd);
        strcat(new_cmd, " $");
        delete[] cmd; // do we need this?
        cmd = new_cmd;
    }*/
    smash->executeCommand(cmd);
    if (close(index_in_fdt) == -1) {
        perror("smash error: close failed");
        return -1;
    }
    return 0;
}

void PipeCommand::execute() {
    int fd[2];                                       //fd[0]-read, fd[1]-write
    if (pipe(fd) == -1) {
        perror("smash error: pipe failed");
        return;
    }
    pid_t left_pid = fork();
    if (left_pid == -1) {                             // fork left failed
        perror("smash error: fork failed");
        return;
    } else if (left_pid == 0) {                         // left child
        if (run_child_process(true, fd) == 0)
            exit(0);
        return;
    }

    pid_t right_pid = fork();
    if (right_pid == -1) {                            // fork failed
        perror("smash error: fork failed");
        return;
    } else if (right_pid == 0) {                        // right child
        if (run_child_process(false, fd) == 0)
            exit(0);
        return;
    }

    // parent
    int status;
    if (close(fd[0]) == -1 || close(fd[1]) == -1) {
        perror("smash error: close failed");
        return;
    }
    if (waitpid(left_pid, &status, WUNTRACED) == -1 ||
        waitpid(right_pid, &status, WUNTRACED) == -1) {
        perror("smash error: wait failed");
        return;
    }
}


RedirectionCommand::RedirectionCommand(const char *cmd_line, SmallShell *smash) : Command(cmd_line, smash),
                                                                                  filepath(nullptr), is_append(false) {
    string cmd_s(cmd_line), command = "", file;
    int cmd_end_index = cmd_s.find_first_of('>');
    int filepath_start_index = cmd_end_index + 1;

    //if append   (default: override)
    if (cmd_s[filepath_start_index] == '>') {
        is_append = true;
        filepath_start_index++;
    }

    if (cmd_end_index != 0)
        command = cmd_s.substr(0, cmd_end_index);
    file = cmd_s.substr(filepath_start_index, string::npos);

    command = _trim(command);
    file = _trim(file);


    cmd = new char[command.length() + 1];
    filepath = new char[file.length() + 1];

    strcpy(cmd, command.c_str());
    strcpy(filepath, file.c_str());
}

RedirectionCommand::~RedirectionCommand()
{
    delete[] cmd;
    delete[] filepath;
}


void RedirectionCommand::execute() {
    int file_flags = O_WRONLY | O_CREAT;
    if (is_append)                      // append ">>"
        file_flags |= O_APPEND;
    else                                // override ">"
        file_flags |= O_TRUNC;
    
    mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO ;
    //mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH; // file permissions

    int fd = open(filepath, file_flags, mode);
    if (fd == -1) {
        perror("smash error: open failed");
        return;
    }

    int stdout_copy = dup(1);
    if (stdout_copy == -1) {              //backup the stdout entry in fdt
        perror("smash error: dup failed");
        if (close(fd) == -1)             //backup failed? - ABORT!!
            perror("smash error: close failed");
        return;
    }
    if (dup2(fd, 1) == -1) {            // replace stout entry content with our file
        perror("smash error: dup2 failed");
        if (close(fd) == -1)            // action failed? - ABORT!
            perror("smash error: close failed");
        return;
    }

    smash->executeCommand(cmd);         // write into file

    if (dup2(stdout_copy, 1) == -1)     // restore stdout to its rightful place
        perror("smash error: dup2 failed");

    if (close(stdout_copy) == -1)       // close the backup index you've created
        perror("smash error: close failed");

    if (close(fd) == -1)                // close the file
        perror("smash error: close failed");

}


//=========================================================================================//
//-----------------------------Inherited from BuiltInCommand-------------------------------//
//=========================================================================================//

//Chprompt
ChpromptCommand::ChpromptCommand(const char *cmd_line, SmallShell *smash) : BuiltInCommand(cmd_line, smash) {}

void ChpromptCommand::execute() {
    char **parsed_cmd = new char *[COMMAND_MAX_ARGS];
    char *cmd_copy = new char[COMMAND_ARGS_MAX_LENGTH];

    strcpy(cmd_copy, cmd_line);
    _removeBackgroundSign(cmd_copy);
    int num_of_args = _parseCommandLine(cmd_copy, parsed_cmd);

    if (num_of_args == 1) {
        this->smash->setName("smash");
    } else {
        this->smash->setName(parsed_cmd[1]);
    }
    delete[] cmd_copy;
    deleteParsedCmd(parsed_cmd, num_of_args);
}


// ShowPid
ShowPidCommand::ShowPidCommand(const char *cmd_line, SmallShell *smash) : BuiltInCommand(cmd_line, smash) {}

void ShowPidCommand::execute() {
    cout << "smash pid is " << smash->smash_pid << endl;
}

//pwd
GetCurrDirCommand::GetCurrDirCommand(const char *cmd_line, SmallShell *smash) : BuiltInCommand(cmd_line, smash) {}

void GetCurrDirCommand::execute() {
    char *buff = get_current_dir_name();
    string path(buff);
    cout << path << endl;
    free(buff);
}


//ChangeDir
ChangeDirCommand::ChangeDirCommand(const char *cmd_line, SmallShell *smash) : BuiltInCommand(cmd_line, smash) {}

void ChangeDirCommand::execute() {
    char **parsed_cmd = new char *[COMMAND_MAX_ARGS];        // args array
    char *cmd_copy = new char[COMMAND_ARGS_MAX_LENGTH];     // copy of the unparsed command

    strcpy(cmd_copy, cmd_line);
    _removeBackgroundSign(cmd_copy);
    int num_of_args = _parseCommandLine(cmd_copy, parsed_cmd);
    delete[] cmd_copy;

    char* curr_path = get_current_dir_name();           //malloc
    
    if (num_of_args > 2)
        std::cerr << "smash error: cd: too many arguments" << std::endl;
    else if (num_of_args < 2)                           // Not specified in HW instructions
        std::cerr << "smash error: cd: invalid arguments" << std::endl;
    
    else{
            char* new_path = parsed_cmd[1];

            if (strcmp(parsed_cmd[1], "-") == 0) {         // change current dir to last working dir
                if (smash->last_working_dir == nullptr)
                {
                    std::cerr << "smash error: cd: OLDPWD not set" << std::endl;  // illegal command
                    free(curr_path);
                    deleteParsedCmd(parsed_cmd, num_of_args);
                    return;
                }
                new_path = smash->last_working_dir;  
            }
            
            if (chdir(new_path) == -1)                      // change current dir to new working dir using chdir syscall
                perror("smash error: chdir failed");

            else {
                //if (smash->last_working_dir != nullptr)
                delete[] smash->last_working_dir;                           // update the value of last_working_dir
                string curr_dir(curr_path);
                smash->last_working_dir = new char[curr_dir.length()+1];
                strcpy(smash->last_working_dir, curr_dir.c_str());
            }
        }

    free(curr_path);
    deleteParsedCmd(parsed_cmd, num_of_args);
}

//jobs

JobsCommand::JobsCommand(const char *cmd_line, SmallShell *smash) : BuiltInCommand(cmd_line, smash) {}

void JobsCommand::execute() {
    smash->jobs_list->removeFinishedJobs();
    //   smash->jobs_list->jobs_list->sort(compareJobs);
    smash->jobs_list->printJobsList();
}

// fg

ForegroundCommand::ForegroundCommand(const char *cmd_line, SmallShell *smash) : BuiltInCommand(cmd_line, smash) {}

void ForegroundCommand::execute() {
    char **parsed_cmd = new char *[COMMAND_MAX_ARGS];        // args array
    char *cmd_copy = new char[COMMAND_ARGS_MAX_LENGTH];     // copy of the unparsed command
    int id_to_remove = 0;

    strcpy(cmd_copy, cmd_line);
    int num_of_args = _parseCommandLine(cmd_copy, parsed_cmd);

    if (num_of_args > 2)                                    // too many args
        std::cerr << "smash error: fg: invalid arguments" << std::endl;

    else if (num_of_args == 1)                              // no args
    {
        if (smash->jobs_list->isEmpty())
            std::cerr << "smash error: fg: jobs list is empty" << std::endl;
        else
            id_to_remove = smash->jobs_list->get_last_job_id();

    } else {                                                  // single arg
        string arg(parsed_cmd[1]);
        if (!_isNumber(arg))
            std::cerr << "smash error: fg: invalid arguments" << std::endl;
        else
            sscanf(parsed_cmd[1], "%d", &id_to_remove);
    }

    delete[] cmd_copy;
    deleteParsedCmd(parsed_cmd, num_of_args);

    if (id_to_remove != 0) {
        JobsList::JobEntry *job_to_fg = smash->jobs_list->getJobById(id_to_remove);

        if (job_to_fg == nullptr){   // job to remove doesn't exist
            std::cerr << "smash error: fg: job-id " << id_to_remove << " does not exist" << std::endl;
            return;
        }

        else { 
            if (kill(job_to_fg->pid, SIGCONT) == -1) {
                perror("smash error: kill failed");
                return;
            }

            JobsList::JobEntry* job_to_run = smash->jobs_list->getJobById(id_to_remove);
            int pid = job_to_run->pid;
            int child_status = -1;
            smash->fg_pid = pid;

            strcpy(smash->cmd_line, (job_to_run->command).c_str());
            cout << job_to_run->command << " : " << pid << endl;
            waitpid(pid, &child_status, WUNTRACED); // waitpid failure? or something

            if(!WIFSTOPPED(child_status)) {
                smash->jobs_list->removeJobById(job_to_run->job_id);
            }
            smash->fg_pid    = smash->smash_pid;
        }
    }
}




//bg


BackgroundCommand::BackgroundCommand(const char *cmd_line, SmallShell *smash) : BuiltInCommand(cmd_line, smash) {}


void BackgroundCommand::execute() {
    char **parsed_cmd = new char *[COMMAND_MAX_ARGS];
    char *cmd_copy = new char[COMMAND_ARGS_MAX_LENGTH];

    strcpy(cmd_copy, cmd_line);
    _removeBackgroundSign(cmd_copy);

    int num_of_args = _parseCommandLine(cmd_copy, parsed_cmd);

    delete[] cmd_copy;

    if (num_of_args > 2) {//too many args
        cerr << "smash error: bg: invalid arguments" << endl;
        deleteParsedCmd(parsed_cmd, num_of_args);
        return;
    }
    else if (num_of_args == 2) {// 1 arguments
        string job_id(parsed_cmd[1]);
        if (!_isNumber(job_id)) { // bad arg
            cerr << "smash error: bg: invalid arguments" << endl;
            deleteParsedCmd(parsed_cmd, num_of_args);
            return;
        }
        else { //all good
            JobsList::JobEntry *job = smash->jobs_list->getJobById(stoi(job_id));
            if (job != nullptr) { // found the job
                if (!(job->is_stopped)) {
                    cerr << "smash error: bg: job-id " << job_id << " is already running in the background"
                         << endl; // job is not stopped
                    deleteParsedCmd(parsed_cmd, num_of_args);
                    return;
                } else {
                    cout << job->command << " : " << job->pid
                         << endl;                                               // job is stopped
                    if (kill(job->pid, SIGCONT) == -1) {
                        perror("smash error: kill failed");
                        deleteParsedCmd(parsed_cmd, num_of_args);
                        return;
                    } else {
                        job->is_stopped = false;
                    }
                }
            } else { // job id not found
                cerr << "smash error: bg: job-id " << job_id << " does not exist" << endl;
                deleteParsedCmd(parsed_cmd, num_of_args);
                return;
            }
        }

    }
    else { // len == 1, no arguments, get max_job_id to bg
        // check if there are no stopped jobs

        int job_id;
        JobsList::JobEntry *job = smash->jobs_list->getLastStoppedJob(&job_id);
        if (job_id == 0) {
            cerr << "smash error: bg: there is no stopped jobs to resume" << endl;
            deleteParsedCmd(parsed_cmd, num_of_args);
            return;
        } else {
            cout << job->command << " : " << job->pid << endl;
            if (kill(job->pid, SIGCONT) == -1) {
                perror("smash error: kill failed");
                deleteParsedCmd(parsed_cmd, num_of_args);
                return;
            } else {
                job->is_stopped = false;
            }
        }
    }
}

QuitCommand::QuitCommand(const char *cmd_line, SmallShell *smash) : BuiltInCommand(cmd_line, smash) {}

void QuitCommand::execute() {
    char **parsed_cmd = new char *[COMMAND_MAX_ARGS];        // args array
    char *cmd_copy = new char[COMMAND_ARGS_MAX_LENGTH];     // copy of the unparsed command

    strcpy(cmd_copy, cmd_line);
    _removeBackgroundSign(cmd_copy);
    int num_of_args = _parseCommandLine(cmd_copy, parsed_cmd);

    if (strcmp(parsed_cmd[1], "kill") == 0) {
        cout << "smash: sending SIGKILL signal to " << smash->jobs_list->jobs_list->size() << " jobs:" << endl;
        if(!smash->jobs_list->jobs_list->empty()){
            smash->jobs_list->killAllJobs();
        }
    }
    delete[] cmd_copy;
    deleteParsedCmd(parsed_cmd, num_of_args);
    //delete smash->jobs_list;
    delete this;
    exit(0);
}

KillCommand::KillCommand(const char *cmd_line, SmallShell *smash) : BuiltInCommand(cmd_line, smash) {}


void KillCommand::execute() {
    char** parsed_cmd = new char*[COMMAND_MAX_ARGS];
    char* copy_cmd = new char[COMMAND_ARGS_MAX_LENGTH];

    strcpy(copy_cmd, cmd_line);
    _removeBackgroundSign(copy_cmd);
    int num_of_args = _parseCommandLine(copy_cmd, parsed_cmd);

    if(num_of_args != 3){ // invalid num of arguments
        cerr << "smash error: kill: invalid arguments" << endl;
        delete[] copy_cmd;
        deleteParsedCmd(parsed_cmd, num_of_args);
        return;
    }

    string signal_num(parsed_cmd[1]);
    string job_id(parsed_cmd[2]);

    if(signal_num.find_first_not_of("-0123456789") != string::npos || signal_num[0] != '-' ||
            signal_num.find_last_of('-') != signal_num.find_first_of('-') ||
            job_id.find_first_not_of("-0123456789") != string::npos){ // invalid syntax
        cerr << "smash error: kill: invalid arguments" << endl;
        delete[] copy_cmd;
        deleteParsedCmd(parsed_cmd, num_of_args);
        return;
    }
    int signal = stoi(signal_num);
    signal = abs(signal);
    if (0 > signal || signal > 31){
        cerr << "smash error: kill: invalid arguments" << endl;
        delete[] copy_cmd;
        deleteParsedCmd(parsed_cmd, num_of_args);
        return;
    }

    else{
        // valid input, finding the job
        int id = stoi(job_id); //jobID
        JobsList::JobEntry* job = smash->jobs_list->getJobById(id);
        if(job == nullptr || job_id.find_first_of('-') != string::npos){ // job list is empty / job not found
            cerr << "smash error: kill: job-id " << id << " does not exist" << endl;
            delete[] copy_cmd;
            deleteParsedCmd(parsed_cmd, num_of_args);
            return;
        }

        auto pid = job->pid;
        if(kill(pid, signal) == -1){
            perror("smash error: kill failed");
            delete[] copy_cmd;
            deleteParsedCmd(parsed_cmd, num_of_args);
            return;
        }
        else{
            if(signal == 20){
                job->is_stopped = true;
            }
            else if(signal == 18){
                job->is_stopped = false;
            }
            cout << "signal number " << signal << " was sent to pid " << pid << endl;
        }
    }
    delete[] copy_cmd;
    deleteParsedCmd(parsed_cmd, num_of_args);
}

//--------------------------------------- Jobs Implementation ----------------------------------------

JobsList::JobsList() : curr_max_job_id(0) {
    jobs_list = new list<JobEntry *>;
    timeout_queue = new priority_queue<TimeoutEntry*, vector<TimeoutEntry*>, CmpEndTime>;
}

JobsList::~JobsList() {
    if (!jobs_list->empty())
        deleteJobsList();
    if (!timeout_queue->empty())
        deleteTimeoutQueue();
    delete jobs_list;
    delete timeout_queue;
}


// CHECK THIS OUT

void JobsList::addJob(string cmd_line, int pid, time_t time, bool is_stopped = false) {
    string cmd(cmd_line);
    auto* new_job = new JobsList::JobEntry(curr_max_job_id+1, pid, time, cmd, is_stopped);
    new_job->is_stopped = is_stopped;
    jobs_list->push_back(new_job);
    curr_max_job_id++;
}


bool JobsList::isEmpty() {
    return jobs_list->empty();
}

void JobsList::printJobsList() {
    auto i = jobs_list->begin();

    while (i != jobs_list->end())
    {
        cout << "[" << (*i)->job_id << "] ";
        cout << (*i)->command << " : " << (*i)->pid;
        time_t i_time;
        if (time(&i_time) == ((time_t) -1)) {
            perror("smash error: time failed");
        }
        cout << " " << difftime(i_time, (*i)->time_inserted) << " secs";
        if ((*i)->is_stopped) {
            cout << " (stopped)";
        }
        cout << endl;
        i++;
    }

}

void JobsList::killAllJobs() {
    std::list<JobEntry*>::iterator i = jobs_list->begin();
    while (i != jobs_list->end())
    {
        cout <<  (*i)->pid << ": " << (*i)->command << endl;
        if (kill((*i)->pid, SIGKILL) == -1)
            perror("smash error: kill failed");
        removeJobById((*i++)->job_id); 
    }
}

void JobsList::deleteJobsList() {
    std::list<JobEntry*>::iterator i = jobs_list->begin();
    while (i != jobs_list->end())
    {
        if (kill((*i)->pid, SIGKILL) == -1)
            perror("smash error: kill failed");
        removeJobById((*i++)->job_id); 
    }
}

void JobsList::updateMaxJobId() {
    auto it = jobs_list->begin();
    auto end = jobs_list->end();
    int max_id = 0;
    while (it != end) {
        if ((*it)->job_id >= max_id) {
            max_id = (*it)->job_id;
        }
        ++it;
    }
    curr_max_job_id = max_id;
}



void JobsList::removeFinishedJobs() {
    auto first = jobs_list->begin();
    auto last = jobs_list->end();
    if (*first == nullptr) {
        return;
    }
    while (first != last) {
        string first_word_in_cmd = ((*first)->command).substr(0, ((*first)->command).find_first_of(' '));
        if (!(first_word_in_cmd == "timeout")) {
            auto pid = (*first)->pid;
            int status = waitpid(pid, nullptr, WNOHANG);
            if (status > 0) {
                removeJobById((*(first++))->job_id);
            }
        }
        first++;
    }
}

JobsList::JobEntry *JobsList::getJobById(int jobId) {
    auto first = jobs_list->begin();
    auto last = jobs_list->end();
    if (*first == nullptr) {
        return nullptr;
    }
    while (first != last) {
        if ((*first)->job_id == jobId) {
            return *first;
        }
        ++first;
    }
    return nullptr;
}

JobsList::JobEntry *JobsList::getJobBypid(int pid) {
    auto first = jobs_list->begin();
    auto last = jobs_list->end();
    if (*first == nullptr) {
        return nullptr;
    }
    while (first != last) {
        if ((*first)->pid == pid) {
            return *first;
        }
        ++first;
    }
    return nullptr;
}

//JobEntry *getLastJob(int *lastJobId);    //what?...
JobsList::JobEntry *JobsList::getLastStoppedJob(int *jobId) {
    auto it = jobs_list->begin();
    int max_stopped_id = 0;
    auto max = it;
    while (it != jobs_list->end()) {
        if ((*it)->job_id >= max_stopped_id && (*it)->is_stopped) {
            max_stopped_id = (*it)->job_id;
            max = it;
        }
        ++it;
    }
    *jobId = max_stopped_id;
    return *max;
}

void JobsList::removeJobById(int jobId) {
    auto first = jobs_list->begin();
    if (*first == nullptr) {
        return;
    }
    JobEntry *job = getJobById(jobId);
    if (job->job_id == jobId) {//remove jobID and update new max_job_id
        jobs_list->remove(job);
        delete job;
        updateMaxJobId();
    }

}

void JobsList::removeJobBypid(int pid) {
    auto first = jobs_list->begin();
    if (*first == nullptr) {
        return;
    }
    JobEntry *job = getJobBypid(pid);
    if (job->pid == pid) {//remove job and update new max_job_id
        jobs_list->remove(job);
        delete job;
        updateMaxJobId();
    }

}

int JobsList::get_last_job_id() {
    auto it = jobs_list->begin();
    int max = -1;
    if (*it == nullptr) {
        return max;
    }
    while (it != jobs_list->end()){
        max = (*it)->job_id;
        ++it;
    }
    return max;
}

//------------------------------------- SmallShell Implementation ------------------------------------

SmallShell::SmallShell() : smash_name("smash"), last_working_dir(nullptr), cmd_line(nullptr), jobs_list(nullptr), fg_pid(getpid()), smash_pid(getpid()) {
    jobs_list = new JobsList();
    cmd_line = new char[COMMAND_ARGS_MAX_LENGTH];  // maximum len for a command
}

SmallShell::~SmallShell() {
    delete[] cmd_line;   // maybe automatic??
    delete jobs_list;
    delete[] last_working_dir;
}

Command *SmallShell::CreateCommand(const char *cmd_line) {
    string cmd_s = _trim(string(cmd_line));
    string command = cmd_s.substr(0, cmd_s.find_first_of(' '));

    //WARNING, check if u need to check if cmd_line is finished
    if (cmd_s.find_first_of('|') != string::npos)
        return new PipeCommand(cmd_line, this);
    else if (cmd_s.find_first_of('>') != string::npos)
        return new RedirectionCommand(cmd_line, this);
    else if (command.empty())
        return nullptr;
    else if (command == "chprompt")
        return new ChpromptCommand(cmd_line, this);
    else if (command == "showpid")
        return new ShowPidCommand(cmd_line, this);
    else if (command == "pwd")
        return new GetCurrDirCommand(cmd_line, this);
    else if (command == "cd")
        return new ChangeDirCommand(cmd_line, this);
    else if (command == "jobs")
        return new JobsCommand(cmd_line, this);
    else if (command == "kill")
        return new KillCommand(cmd_line, this);
    else if (command == "fg")
        return new ForegroundCommand(cmd_line, this);
    else if (command == "bg")
        return new BackgroundCommand(cmd_line, this);
    else if (command == "quit")
        return new QuitCommand(cmd_line, this);
    else if (command == "timeout")
        return new TimeoutCommand(cmd_line, this);
    else
        return new ExternalCommand(cmd_line, this);
    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    Command *cmd = CreateCommand(cmd_line);
    if (cmd == nullptr) {
        return;
    }

    string cmd_s = _trim(string(cmd_line));

    //update foreground command
    //update foreground command

    strcpy(this->cmd_line, cmd_s.c_str());

    this->jobs_list->removeFinishedJobs();
    cmd->execute();
    delete cmd;

}



//==============================================================================================//
//--------------------------------TIMEOUT COMMAND IMPLEMENTATION--------------------------------//
//==============================================================================================//


TimeoutCommand::TimeoutCommand(const char *cmd_line, SmallShell *smash) : BuiltInCommand(cmd_line, smash) {}

void TimeoutCommand::execute() 
{
    char** parsed_cmd = new char*[COMMAND_MAX_ARGS];
    char* copy_cmd = new char[COMMAND_ARGS_MAX_LENGTH];
    strcpy(copy_cmd, cmd_line);
    _removeBackgroundSign(copy_cmd);

    int num_of_args = _parseCommandLine(copy_cmd, parsed_cmd);


    if(num_of_args < 3){ // invalid num of args OR duration <= 0
        cerr << "smash error: timeout: invalid arguments" << endl;
        delete[] copy_cmd;
        deleteParsedCmd(parsed_cmd, num_of_args);
        return;
    }

    int duration = stoi(parsed_cmd[1]);

    if (duration <= 0) {
        cerr << "smash error: timeout: invalid arguments" << endl;
        delete[] copy_cmd;
        deleteParsedCmd(parsed_cmd, num_of_args);
        return;
    }


    // run the command
    string cmd_s = _trim(string(cmd_line));
    string duration_str = cmd_s.substr(cmd_s.find_first_of(" \t") + 1);
    string actual_command = duration_str.substr(duration_str.find_first_of(" \t") + 1);

    int child_status = 0;
    pid_t fork_pid;
    fork_pid = fork();
    
    if (fork_pid == -1){        // fork failed
        perror("smash error: fork failed");
        delete[] copy_cmd;
        deleteParsedCmd(parsed_cmd, num_of_args);
        return;
    }

    if (fork_pid == 0) // son
    {
        setpgrp();
        char* bash_args[] = {(char*)"/bin/bash", (char*)"-c", (char*)actual_command.c_str() ,NULL};
        if (execv("/bin/bash", bash_args) == -1)
        {
            perror("smash error: execv failed");
            exit(1); //exit child's process
        }
    }

    else // father
    {
        // if son failed creation or returned 1 exit status - ABORT!!
        if(kill(fork_pid, 0) != 0) {
            delete[] copy_cmd;
            deleteParsedCmd(parsed_cmd, num_of_args); 
            return;
        }
    
        //create an alarm
        time_t current_time;
        if(time(&current_time) == -1) {
            perror("smash error: time failed");
            delete[] copy_cmd;
            deleteParsedCmd(parsed_cmd, num_of_args); 
            return;
        }

        string cmd(cmd_line);
        smash->jobs_list->addTimeout(cmd, duration, fork_pid, current_time);
        alarm(duration);

        // timeout is a background job
        if (_isBackgroundCommand(cmd_line)){
            smash->jobs_list->removeFinishedJobs();
            smash->jobs_list->addJob(cmd, fork_pid, current_time, false);  // add to jobs list
//            smash->jobs_list->addTimeout(cmd, duration, fork_pid, current_time);
//            alarm(duration);
        }

        // timeout is a foreground job
        else {
//            string cmd(cmd_line);
//            smash->jobs_list->addTimeout(cmd, duration, fork_pid, current_time);
//            alarm(duration);
            smash->fg_pid = fork_pid;
            waitpid(fork_pid, &child_status, WUNTRACED); // waitpid failure? or something
            smash->fg_pid = smash->smash_pid;
        }
    }
}

void JobsList::addTimeout(string cmd_line, int duration, pid_t pid , time_t time_inserted) {
    string cmd(cmd_line);
    auto new_timeout = new JobsList::TimeoutEntry(duration, pid, time_inserted, cmd);
    timeout_queue->push(new_timeout); // MUST DEFINE TO PRIORITY QUEUE WHAT'S PRIORITY!
}

void JobsList::deleteTimeoutQueue() {
    while (!timeout_queue->empty()){
        auto to_delete = timeout_queue->top();
        timeout_queue->pop();
        delete to_delete;
    }
}



// for priority queue sort
//bool JobsList::TimeoutEntry::operator<(const JobsList::TimeoutEntry& other) const {
//  return this->end_time > other.end_time;
//}

/*
pid_t TimeoutInstance::GetPid() const {
  return this->pid;
}

std::string TimeoutInstance::GetCMD() const {
  return this->cmd_line;
}*/

//struct CmpEndTime : public binary_function<JobsList::TimeoutEntry*, JobsList::TimeoutEntry*, bool>{
//    bool operator()(const JobsList::TimeoutEntry* first, const JobsList::TimeoutEntry* second){
//        return first->end_time < second->end_time;
//    }
//};
