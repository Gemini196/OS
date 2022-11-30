#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <string>
#include <stdbool.h>
#include <list>
#include <wait.h>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

using namespace std;

// forward declaration
class SmallShell;
class JobsList;

//----------------------------------------- Command -----------------------------------------
class Command {
  public:
    const char* cmd_line;
    SmallShell* smash;

  public:
    Command(const char* cmd_line, SmallShell* smash): cmd_line(cmd_line), smash(smash){}
    virtual ~Command(){}
    virtual void execute() = 0;
};

//------------------------------ Inherited from Command ---------------------------------------

class BuiltInCommand : public Command {
  public:
    BuiltInCommand(const char* cmd_line, SmallShell* smash) : Command(cmd_line, smash){}
    //BuiltInCommand(const char* cmd_line, SmallShell* smash);
    virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
  public:
    ExternalCommand(const char *cmd_line, SmallShell *smash) : Command(cmd_line, smash) {}
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
    virtual ~PipeCommand() {}
    void execute() override;
    int run_child_process(bool is_left, int fd[]);
};

class RedirectionCommand : public Command {
    public:
    char* cmd;
    char* filepath;
    bool is_append;
  public:
    explicit RedirectionCommand(const char* cmd_line, SmallShell* smash);
    virtual ~RedirectionCommand() {}
    void execute() override;
    //void prepare() override;
    //void cleanup() override;
};

//------------------------------ Inherited from BuiltInCommand ---------------------------------------

class ChangeDirCommand : public BuiltInCommand {
  public:
    ChangeDirCommand(const char* cmd_line, SmallShell* smash);
    virtual ~ChangeDirCommand() {}
    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
  public:
    GetCurrDirCommand(const char* cmd_line, SmallShell* smash);
    virtual ~GetCurrDirCommand() {}
    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
  public:
    ShowPidCommand(const char* cmd_line, SmallShell* smash);
    virtual ~ShowPidCommand() {}
    void execute() override;
};

class ChpromptCommand: public BuiltInCommand{
  public:
      ChpromptCommand(const char* cmd_line, SmallShell* smash);
      virtual ~ChpromptCommand() {}
      void execute() override;
};

class QuitCommand : public BuiltInCommand {
  public:
    QuitCommand(const char* cmd_line, SmallShell *smash);
    virtual ~QuitCommand() {}
    void execute() override;
};

class JobsCommand : public BuiltInCommand {
  public:
    JobsCommand(const char* cmd_line, SmallShell *smash);
    virtual ~JobsCommand() {}
    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
  public:
    ForegroundCommand(const char* cmd_line, SmallShell* smash);
    virtual ~ForegroundCommand() {}
    void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
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

    KillCommand(const char *cmd_line, SmallShell *smash);

    virtual ~KillCommand() {}
    void execute() override;
};

//----------------------------------------- Job Class ------------------------------------------------

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
    public:
    JobsList();
    ~JobsList();
    void addJob(const char* cmd_line, int pid, time_t time, bool is_stopped);
    bool isEmpty();
    void printJobsList();
    void killAllJobs();
    void updateMaxJobId();
    void removeFinishedJobs();
    JobEntry * getJobById(int jobId);
    JobEntry *getLastStoppedJob(int *jobId);
    void removeJobById(int jobId);
};


//extern bool compareJobs(JobsList::JobEntry* job1, JobsList::JobEntry* job2){
  //  return job1->job_id < job2->job_id;
//}


//------------------------------------ SmallShell ----------------------------------------------------

class SmallShell {
  private:
    SmallShell();
  
  public:
    string smash_name;
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
