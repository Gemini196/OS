#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <string>
#include <stdbool.h>
#include <list>
#include <wait.h>


using namespace std;
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

// forward declaration
class SmallShell;
class JobsList;

//----------------------------------------------------------------------------------
class Command {
  public:
    const char* cmd_line;
    SmallShell* smash;

  public:
    Command(const char* cmd_line);
    Command(const char* cmd_line, SmallShell* smash): cmd_line(cmd_line), smash(smash){}
    virtual ~Command(){}
    virtual void execute() = 0;
    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
};

//------------------------------ Inherited from Command ---------------------------------------

class BuiltInCommand : public Command {
  public:
    BuiltInCommand(const char* cmd_line);
    BuiltInCommand(const char* cmd_line, SmallShell* smash);
    virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
  public:
    ExternalCommand(const char* cmd_line);
    virtual ~ExternalCommand() {}
    void execute() override;
};

class PipeCommand : public Command {
  public:
    char* left_cmd;
    char* right_cmd;
    bool err_flag;
  public:
    PipeCommand(const char* cmd_line, SmallShell* smash);
    //virtual ~PipeCommand() {
     //   delete[] left_cmd;             // WHY IS THIS HERE?????????????????????????????????
      //  delete[] right_cmd;            // WHY IS THIS HERE?????????????????????????????????
    //}
    ~PipeCommand() = default;
    void execute() override;
    int run_child_process(bool is_left, int fd[]);
};

class RedirectionCommand : public Command {
    public:
    char* cmd;
    char* filepath;
    bool is_append;
  public:
    RedirectionCommand(const char* cmd_line, SmallShell* smash);
    //explicit RedirectionCommand(const char* cmd_line);
    virtual ~RedirectionCommand() {}
    void execute() override;
    //void prepare() override;
    //void cleanup() override;
};

//------------------------------ Inherited from BuiltInCommand ---------------------------------------

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members public:
  public:
    ChangeDirCommand(const char* cmd_line, char** plastPwd);
    ChangeDirCommand(const char* cmd_line, SmallShell* smash);
    virtual ~ChangeDirCommand() {}
    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
  public:
    GetCurrDirCommand(const char* cmd_line);
    GetCurrDirCommand(const char* cmd_line, SmallShell* smash);
    virtual ~GetCurrDirCommand(){}
    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
  public:
    ShowPidCommand(const char* cmd_line);
    ShowPidCommand(const char* cmd_line, SmallShell* smash);
    virtual ~ShowPidCommand(){}
    void execute() override;
};

class ChpromptCommand: public BuiltInCommand{
  public:
      ChpromptCommand(const char* cmd_line);
      ChpromptCommand(const char* cmd_line, SmallShell* smash);
      virtual ~ChpromptCommand(){}
      void execute() override;
};

class QuitCommand : public BuiltInCommand {
  // TODO: Add your data members
  public:
    QuitCommand(const char* cmd_line, SmallShell *smash);
    virtual ~QuitCommand() {}
    void execute() override;
};


class JobsCommand : public BuiltInCommand {
 // TODO: Add your data members
  public:
    JobsCommand(const char* cmd_line, SmallShell *smash);
    virtual ~JobsCommand() {}
    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
  public:
    ForegroundCommand(const char* cmd_line, JobsList* jobs);
    ForegroundCommand(const char* cmd_line, SmallShell* smash);
    virtual ~ForegroundCommand() {}
    void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
 // TODO: Add your data members
  public:
    BackgroundCommand(const char* cmd_line, SmallShell* smash);
    virtual ~BackgroundCommand() {}
    void execute() override;
};

class TimeoutCommand : public BuiltInCommand {
/* Optional */
// TODO: Add your data members
  public:
    explicit TimeoutCommand(const char* cmd_line);
    virtual ~TimeoutCommand() {}
    void execute() override;
};

class FareCommand : public BuiltInCommand {
    /* Optional */
    // TODO: Add your data members
  public:
    FareCommand(const char* cmd_line);
    virtual ~FareCommand() {}
    void execute() override;
};

class SetcoreCommand : public BuiltInCommand {
  /* Optional */
  // TODO: Add your data members
  public:
    SetcoreCommand(const char* cmd_line);
    virtual ~SetcoreCommand() {}
    void execute() override;
};

class KillCommand : public BuiltInCommand {
  /* Bonus */
 // TODO: Add your data members
  public:
    KillCommand(const char* cmd_line, JobsList* jobs);
    virtual ~KillCommand() {}
    void execute() override;
};

//------------------------------ Job Class ---------------------------------------

class JobsList {
public:
    class JobEntry {
    public:
        int job_id;
        int pid;
        time_t time_inserted;
        string command;
        bool is_stopped;
    public:
        JobEntry(int job_id, int pid, time_t time_inserted, string &command, bool is_stopped = false) :
                job_id(job_id), pid(pid), time_inserted(time_inserted), command(command), is_stopped(is_stopped) {}

        JobEntry(JobEntry &job) :
                job_id(job.job_id), pid(job.pid), time_inserted(job.time_inserted), command(string(job.command)),
                is_stopped(job.is_stopped) {}

        ~JobEntry() = default;

        bool operator==(const JobsList::JobEntry &job) { return job_id == job.job_id; }

        bool operator>=(const JobsList::JobEntry &job) { return job_id >= job.job_id; }

    };

public:
    std::list<JobEntry *> *jobs_list;
    int curr_max_job_id;

    JobsList() : curr_max_job_id(0) {
        jobs_list = new std::list<JobEntry *>;
    }

    ~JobsList() {
        delete jobs_list;
    }

    void addJob(Command *cmd, int pid, bool is_stopped) {
        //JobsList::JobEntry* new_job =
    }

    bool isEmpty() {
      return jobs_list->empty();
    }

    void printJobsList(){
        auto list_start = jobs_list->begin();
        auto list_end = jobs_list->end();
        for(; list_start != list_end; ++list_start){
            cout << "[" << (*list_start)->job_id << "] " ;
            cout << (*list_start)->command << " : " << (*list_start)->pid;
            time_t i_time;
            if(time(&i_time) == ((time_t)-1)){
                perror("smash error: time failed");
            }
            cout << " " << difftime(i_time, (*list_start)->time_inserted) << " secs";
            if ((*list_start)->is_stopped){
                cout << " (stopped)";
            }
            cout << endl;
        }
    }

    void killAllJobs(){
      auto list_start = jobs_list->begin();
      auto list_end = jobs_list->end();
        for(; list_start != list_end; ++list_start){
            cout << "[" << (*list_start)->job_id << "] " ;
            cout << (*list_start)->command << " : " << (*list_start)->pid;
            cout << endl;
             if(kill((*list_start)->pid, SIGKILL) == -1){
                perror("smash error: kill failed");
                removeJobById((*list_start)->job_id);
            }
        }
    }


    void updateMaxJobId(){
        auto it = jobs_list->begin();
        auto end = jobs_list->end();
        int max_id = 0;
        while (it != end){
            if((*it)->job_id >= max_id){
                max_id = (*it)->job_id;
            }
            it++;
        }
        curr_max_job_id = max_id;
    }


    void removeFinishedJobs() {
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

    JobEntry * getJobById(int jobId){
        auto first = jobs_list->begin();
        auto last = jobs_list->end();
        if(*first == nullptr){
            return nullptr;
        }
        while (first!=last) {
            if ((*first)->job_id == jobId){
                return *first;
            }
            ++first;
        }
        return nullptr;
    }

    //JobEntry *getLastJob(int *lastJobId);    //what?...
    JobEntry *getLastStoppedJob(int *jobId){
        auto it = jobs_list->begin();
        int max_stopped_id = 0;
        auto max = it;
        while (it != jobs_list->end()){
            if((*it)->job_id >= max_stopped_id && (*it)->is_stopped){
                max_stopped_id = (*it)->job_id;
                max = it;
            }
            it++;
        }
        *jobId = max_stopped_id;
        return *max;
    }

    void removeJobById(int jobId) {
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
};


//extern bool compareJobs(JobsList::JobEntry* job1, JobsList::JobEntry* job2){
  //  return job1->job_id < job2->job_id;
//}


//------------------------------ SmallShell ---------------------------------------

class SmallShell {
  private:
    // TODO: Add your data members
    SmallShell();
  
  public:
    string smash_name;
    //string parent_dir;      // DO WE NEED THIS??
    char* last_working_dir;
    JobsList* jobs_list;
    pid_t fg_pid;
    pid_t smash_pid;

    Command *CreateCommand(const char* cmd_line);
    Command *CreateCommand(const char* cmd_line, SmallShell* smash);
    SmallShell(SmallShell const&)      = delete; // disable copy ctor
    void operator=(SmallShell const&)  = delete; // disable = operator
    static SmallShell& getInstance() // make SmallShell singleton           //WHAT IS GOING ON HERE??!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
    {
      static SmallShell instance; // Guaranteed to be destroyed.
      // Instantiated on first use.
      return instance;
    }
    ~SmallShell();
    void executeCommand(const char* cmd_line);
    string getName() {return smash_name;};
    void setName(const string& new_name) {smash_name = new_name;};
};

#endif //SMASH_COMMAND_H_
