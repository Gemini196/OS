#include <iostream>
#include <signal.h>
#include <unistd.h>
#include "signals.h"
#include "Commands.h"
#include <assert.h>
#include <wait.h>


using namespace std;

#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

// add to ctrlC & ctrlZ: if the fg process is timeout - 
// CtrlC- remove it from timout_list
// CtrlZ - add job_id to TimoutEntry of the process

void ctrlZHandler(int sig_num) {
    cout << "smash: got ctrl-Z" << endl;
    SmallShell& smash = SmallShell::getInstance();
    if(smash.fg_pid == smash.smash_pid)
      return;

    if(kill(smash.fg_pid, sig_num) == -1){
        perror("smash error: kill failed");
        return;
    }

    time_t i_time;
    if (time(&i_time) == ((time_t) -1)) {
        perror("smash error: time failed");
        return;
    }
    JobsList::JobEntry* job_to_stop = smash.jobs_list->getJobBypid(smash.fg_pid);
    if (job_to_stop != nullptr)
        job_to_stop->is_stopped = true;
    else
        smash.jobs_list->addJob(smash.cmd_line, smash.fg_pid, i_time, true);
    
    cout << "smash: process " << smash.fg_pid << " was stopped" << endl;
    smash.fg_pid = smash.smash_pid;
}

void ctrlCHandler(int sig_num) {
    cout << "smash: got ctrl-C" << endl;
    SmallShell& smash = SmallShell::getInstance();

    if(smash.fg_pid == smash.smash_pid)
        return;

    if(kill(smash.fg_pid, sig_num) == -1){
        perror("smash error: kill failed");
        return;
    }

    cout << "smash: process " << smash.fg_pid << " was killed" << endl;
    smash.fg_pid = smash.smash_pid;
}



// FIND THE CLOSEST INSTANCE (IN TIMEOUT LIST) THAT NEEDS TO GET AN ALARM AND KILL IT

// just got SIG_ALRM - time to smash some timeout
void alarmHandler(int sig_num){
    SmallShell& smash = SmallShell::getInstance();
    auto top = smash.jobs_list->timeout_queue->top();
    JobsList::TimeoutEntry to_delete = *top;
    pid_t timedout_pid = to_delete.pid;

    if(kill(timedout_pid, 0) != 0) { // process died before timeout
        if (smash.jobs_list->getJobBypid(timedout_pid) != nullptr)
            smash.jobs_list->removeJobBypid(timedout_pid);
        smash.jobs_list->timeout_queue->pop();
        return;
    }

  cout << "smash: got an alarm" << endl;
  time_t current_time;
  if(time(&current_time) == -1) {
    perror("smash error: time failed");
    return;
  }


    cout << "smash: " << to_delete.command << " timed out!" << endl;
    if(kill(timedout_pid, SIGKILL) == -1) {
        perror("smash error: kill failed");
        return;
    }

  // remove the bastard from all queues and lists
  if (smash.jobs_list->getJobBypid(timedout_pid) != nullptr)
    smash.jobs_list->removeJobBypid(timedout_pid);
  smash.jobs_list->timeout_queue->pop();

}


