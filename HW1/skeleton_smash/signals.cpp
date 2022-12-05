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


/*
void alarmHandler(int sig_num) 
{  
  cout << "smash: got an alarm" << endl;

  SmallShell& smash = SmallShell::getInstance();
  
  time_t current_time;
  if(time(&current_time) == -1) {
    perror("smash error: time failed");
    return;
  }


  pid_t timedout_pid = (smash.TimeoutManager.top()).GetPid(); // the top of the queue is the one who timedout 
  
  
  // Piazza @92 (Assuming no more than 1 timeout per second)
  int what = waitpid(timedout_pid, nullptr, WNOHANG); 
  if(what == 0) {
    std::cout << "smash: " << (smash.TimeoutManager.top()).GetCMD() << " timed out!" << std::endl;
    if(kill(timedout_pid, SIGKILL) == -1) {
      perror("smash error: kill failed");
      return;
    }
  }
  smash.basic_list.removeJobByPid(timedout_pid);
  smash.TimeoutManager.pop();
  if (smash.TimeoutManager.size() == 0) { // no more timed commands
    return;
  }
    
  // Set next alarm
  const TimeoutInstance& next_timeout = smash.TimeoutManager.top();

  //assert(next_timeout.end_time > current_time); // If the end time has passed, an alarm should've been made

  unsigned int next_alarm = (unsigned int)difftime(next_timeout.end_time, current_time);
  alarm(next_alarm);
  return;
  
}*/

