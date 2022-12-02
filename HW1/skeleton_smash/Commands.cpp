#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <fcntl.h>

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

bool _isNumber(string str) {
    for (unsigned int i = 0; i < str.length(); i++) {
        if (isdigit(str[i]) == false)
            return false;
    }
    return true;
}


//------------------------------ Inherited from Command ----------------------------------------------

void ExternalCommand::execute() {
    //char** parsed_cmd = new char*[21];
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
            JobsList::JobEntry new_job(smash->jobs_list->curr_max_job_id, pid, i_time, cmd, false);
            smash->jobs_list->removeFinishedJobs();
            smash->jobs_list->addJob(cmd_copy, pid, i_time, false);
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
    string cmd_str(cmd);

    // not sure why this is needed, will check later
    if (!cmd_str.compare("showpid")) {
        char *new_cmd = new char[strlen(cmd) + 3];
        strcpy(new_cmd, cmd);
        strcat(new_cmd, " $");
        delete[] cmd; // do we need this?
        cmd = new_cmd;
    }
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
        filepath++;
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


void RedirectionCommand::execute() {
    int file_flags = O_WRONLY | O_CREAT;
    if (is_append)                      // append ">>"
        file_flags |= O_APPEND;
    else                                // override ">"
        file_flags |= O_TRUNC;

    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH; // file permissions

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
                if (smash->last_working_dir != nullptr)
                    delete[] smash->last_working_dir;                           // update the value of last_working_dir
                string curr_dir(curr_path);
                smash->last_working_dir = new char[curr_dir.length()];
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
    int id_to_remove = -1;

    strcpy(cmd_copy, cmd_line);
    int num_of_args = _parseCommandLine(cmd_copy, parsed_cmd);

    if (num_of_args > 2)                                    // too many args
        std::cerr << "smash error: fg: invalid arguments" << std::endl;

    else if (num_of_args == 1)                              // no args
    {
        if (smash->jobs_list->isEmpty())
            std::cerr << "smash error: fg: jobs list is empty"<< std::endl;
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

    if (id_to_remove != -1) {
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

            cout << _rtrim(job_to_fg->command) << "& : " << job_to_fg->pid << endl;
            
            // REMOVE JOB
            int pid = smash->jobs_list->getJobById(id_to_remove)->pid, child_status = -1;
            smash->fg_pid = pid;

            smash->jobs_list->removeJobById(id_to_remove);                        // remove the job from joblist
            waitpid(id_to_remove, &child_status, WUNTRACED); // waitpid failure? or something
            smash->fg_pid = smash->smash_pid;                                    // the fg job finished - the fg job now is the smash itself
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
    string job_id(parsed_cmd[1]);

    delete[] cmd_copy;
    deleteParsedCmd(parsed_cmd, num_of_args);

    if (num_of_args > 2) {//too many args
        cerr << "smash error: bg: invalid arguments" << endl;
        return;
    } else if (num_of_args == 2) {// 1 arguments


        if (!_isNumber(job_id)) { // bad arg
            cerr << "smash error: bg: invalid arguments" << endl;
            return;
        } else { //all good
            JobsList::JobEntry *job = smash->jobs_list->getJobById(stoi(job_id));
            if (job != nullptr) { // found the job
                if (!(job->is_stopped)) {
                    cerr << "smash error: bg: job-id " << job_id << " is already running in the background"
                         << endl; // job is not stopped
                    return;
                } else {
                    cout << job->command << " : " << job->pid
                         << endl;                                               // job is stopped
                    if (kill(job->pid, SIGCONT) == -1) {
                        perror("smash error: kill failed");
                        return;
                    } else {
                        job->is_stopped = false;
                    }
                }
            } else { // job id not found
                cerr << "smash error: bg: job-id " << job_id << " does not exist" << endl;
                return;
            }
        }

    } else { // len == 1, no arguments, get max_job_id to bg
        // check if there are no stopped jobs

        int job_id;
        JobsList::JobEntry *job = smash->jobs_list->getLastStoppedJob(&job_id);
        if (job_id == 0) {
            cerr << "smash error: bg: there is no stopped jobs to resume" << endl;
            return;
        } else {
            cout << job->command << " : " << job->job_id << endl;
            if (kill(job->pid, SIGCONT) == -1) {
                perror("smash error: kill failed");
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

    else{
        // valid input, finding the job
        int id = stoi(job_id); //jobID
        JobsList::JobEntry* job = smash->jobs_list->getJobById(id);
        if(job == *(smash->jobs_list->jobs_list->end()) || job_id.find_first_of('-') != string::npos){ // job list is empty / job not found
            cerr << "smash error: kill: job-id " << id << " does not exist" << endl;
            delete[] copy_cmd;
            deleteParsedCmd(parsed_cmd, num_of_args);
            return;
        }

        int signal = stoi(signal_num);
        signal = abs(signal);
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
    jobs_list = new std::list<JobEntry *>;
}

JobsList::~JobsList() {
    delete jobs_list;
}


// CHECK THIS OUT

void JobsList::addJob(const char *cmd_line, int pid, time_t time, bool is_stopped = false) {
    string cmd(cmd_line);
    JobsList::JobEntry* new_job = new JobsList::JobEntry(curr_max_job_id+1, pid, time, cmd, is_stopped);
    new_job->is_stopped = is_stopped;
    jobs_list->push_back(new_job);
    curr_max_job_id++;
}


bool JobsList::isEmpty() {
    return jobs_list->empty();
}

void JobsList::printJobsList() {
    auto list_start = jobs_list->begin();
    auto list_end = jobs_list->end();
    for (; list_start != list_end; ++list_start) {
        cout << "[" << (*list_start)->job_id << "] ";
        cout << _rtrim((*list_start)->command) << "& : " << (*list_start)->pid;
        time_t i_time;
        if (time(&i_time) == ((time_t) -1)) {
            perror("smash error: time failed");
        }
        cout << " " << difftime(i_time, (*list_start)->time_inserted) << " secs";
        if ((*list_start)->is_stopped) {
            cout << " (stopped)";
        }
        cout << endl;
    }
}

void JobsList::killAllJobs() {
    auto list_start = jobs_list->begin();
    auto list_end = jobs_list->end();
    for (; list_start != list_end; ++list_start) {
        
        std::cout <<  (*list_start)->pid << ": " << _rtrim((*list_start)->command) << "&" << std::endl;
        //cout << "[" << (*list_start)->job_id << "] ";
        //cout << (*list_start)->command << " : " << (*list_start)->pid;
        //cout << endl;
        if (kill((*list_start)->pid, SIGKILL) == -1) {
            perror("smash error: kill failed");
            removeJobById((*list_start)->job_id);
        }
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
        it++;
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
        auto pid = (*first)->pid;
        int status = waitpid(pid, nullptr, WNOHANG);
        if (status > 0) {
            removeJobById((*(first++))->job_id);
        } else {
            first++;
        }
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
        it++;
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

SmallShell::SmallShell() : last_working_dir(nullptr) {
// TODO: add your 

// don't forget to fill the smash_pid property!!
// don't forget to set working dir properties to: nullptr
}

SmallShell::~SmallShell() {
    delete jobs_list;
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
    else
        return new ExternalCommand(cmd_line, this);
    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    Command *cmd = CreateCommand(cmd_line);
    if (cmd == nullptr) {
        return;
    }
    cmd->execute();
    delete cmd;
}


