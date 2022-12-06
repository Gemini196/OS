#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"

int stdout_dp = dup(1);
int stdin_dp = dup(0);

int main(int argc, char* argv[]) {
    if(signal(SIGTSTP , ctrlZHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-Z handler");
    }
    if(signal(SIGINT , ctrlCHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }

    struct sigaction sa;
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = alarmHandler;

    if (sigaction(SIGALRM,&sa,nullptr) < 0) {
        perror("smash error: failed to set alarm handler");
    }



    SmallShell& smash = SmallShell::getInstance();


    while(true) {
       if(close(0) == -1){
           perror("smash error: close failed");
           continue;
       }
       if(dup2(stdin_dp, 0) == -1){
           perror("smash error: dup failed");
           continue;
       }
       if(close(1) == -1){
           perror("smash error: close failed");
           continue;
       }
       if(dup2(stdout_dp,1) == -1){
           perror("smash error: dup failed");
           continue;
       }

        std::cout << smash.getName() << "> ";
        std::string cmd_line;
        std::getline(std::cin, cmd_line);
        smash.jobs_list->removeFinishedJobs();
//        smash.jobs_list->jobs_list->sort(compareJobs);
        smash.executeCommand(cmd_line.c_str());
        //smash.fg_pid = 0;
    }


    return 0;
}