#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <string>
#include <stdbool>
#include <list>
#include <wait.h>


using namespace std;
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)


class Command {
  protected:
    const char* cmd_line;
    SmallShell* smash;
  public:
    Command(const char* cmd_line);
    Command(const char* cmd_line, SmallShell* smash): cmd_line(cmd_line), smash(smash){};
    virtual ~Command();
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
    virtual ~PipeCommand() {
        delete[] left_cmd;             // WHY IS THIS HERE?????????????????????????????????
        delete[] right_cmd;            // WHY IS THIS HERE?????????????????????????????????
    }
    void execute() override;
};

class RedirectionCommand : public Command {
 // TODO: Add your data members
  public:
    explicit RedirectionCommand(const char* cmd_line);
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
    virtual ~ChangeDirCommand() {}
    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
  public:
    GetCurrDirCommand(const char* cmd_line);
    virtual ~GetCurrDirCommand() {}
    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
  public:
    ShowPidCommand(const char* cmd_line);
    virtual ~ShowPidCommand() {}
    void execute() override;
};

class ChpromptCommand: public BuiltInCommand{
  public:
      ChpromptCommand(const char* cmd_line);
      ChpromptCommand(const char* cmd_line, SmallShell* smash);
      virtual ~ChpromptCommand();
      void execute() override;
};

class JobsList;
class QuitCommand : public BuiltInCommand {
  // TODO: Add your data members
  public:
    QuitCommand(const char* cmd_line, JobsList* jobs);
    virtual ~QuitCommand() {}
    void execute() override;
};


class JobsCommand : public BuiltInCommand {
 // TODO: Add your data members
  public:
    JobsCommand(const char* cmd_line, JobsList* jobs);
    virtual ~JobsCommand() {}
    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
  public:
    ForegroundCommand(const char* cmd_line, JobsList* jobs);
    virtual ~ForegroundCommand() {}
    void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
 // TODO: Add your data members
  public:
    BackgroundCommand(const char* cmd_line, JobsList* jobs);
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

  class JobEntry {
    protected:
        int job_id;
        int pid;
        time_t time_inserted;
        string command;
        bool is_stopped;
    public:
        JobEntry(int job_id, int pid, time_t time_inserted, string& command, bool is_stopped = false):
              job_id(job_id), pid(pid), time_inserted(time_insterted), command(command), is_stopped(stopped){};
        JobEntry(JobEntry& job):
              job_id(job.job_id), pid(job.pid), time_inserted(job.time_inserted), command(string(job.command)), is_stopped(job.is_stopped){};
        ~JobEntry() = default;
        bool operator==(const JobsList::JobEntry& job){return job_id == job.job_id;}
        bool operator>=(const JobsList::JobEntry& job){return job_id >= job.job_id;}
  };
  
  public:
    std::list<JobEntry*>* jobs_list;
    int curr_max_job_id;

    JobsList():curr_max_job_id(0){
        jobs_list = new std::list<JobEntry*>;
    }
    ~JobsList(){
        delete jobs_list;
    }
    void addJob(Command* cmd, int pid, bool is_stopped){
        JobsList::JobEntry* new_job =
    }
    void printJobsList();
    void killAllJobs();
    void removeFinishedJobs();
    JobEntry * getJobById(int jobId);
    void removeJobById(int jobId);
    JobEntry * getLastJob(int* lastJobId);
    JobEntry *getLastStoppedJob(int *jobId);
    // TODO: Add extra methods or modify exisitng ones as needed
};


//------------------------------ SmallShell ---------------------------------------

class SmallShell {
  private:
    // TODO: Add your data members
    SmallShell();
  
  public:
    string smash_name;
    string parent_dir;
    JobsList* jobs_list;
    pid_t fg_pid;

    Command *CreateCommand(const char* cmd_line);
    Command *CreateCommand(const char* cmd_line, SmallShell* smash);
    SmallShell(SmallShell const&)      = delete; // disable copy ctor
    void operator=(SmallShell const&)  = delete; // disable = operator
    static SmallShell& getInstance() // make SmallShell singleton
    {
      static SmallShell instance; // Guaranteed to be destroyed.
      // Instantiated on first use.
      return instance;
    }
    ~SmallShell() = default; // IS THIS OK?
    void executeCommand(const char* cmd_line);
    string getName() {return smash_name;};
    void setName(const string& new_name) {smash_name = new_name;};
};

#endif //SMASH_COMMAND_H_
