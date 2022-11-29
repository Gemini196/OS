#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"

int main(int argc, char* argv[]) {
    if(signal(SIGTSTP , ctrlZHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-Z handler");
    }
    if(signal(SIGINT , ctrlCHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }

    //TODO: setup sig alarm handler

    SmallShell& smash = SmallShell::getInstance();

    smash.smash_name = "smash";
    smash.jobs_list = new JobsList();
    smash.last_working_dir = nullptr; // because it is char*
    smash.fg_pid = 0;

    while(true) {
//        if(close(0) == -1){
//            perror("smash error: close failed");
//            continue;
//        }
//        if(dup2(stdin_dp, 0) == -1){
//            perror("smash error: dup failed");
//            continue;
//        }
//        if(close(1) == -1){
//            perror("smash error: close failed");
//            continue;
//        }
//        if(dup2(stdout_dp,1) == -1){
//            perror("smash error: dup failed");
//            continue;
//        }

        std::cout << smash.getName() << "> ";
        std::string cmd_line;
        std::getline(std::cin, cmd_line);
        smash.jobs_list->removeFinishedJobs();
//        smash.jobs_list->jobs_list->sort(compareJobs);
        smash.executeCommand(cmd_line.c_str());
        smash.fg_pid = 0;
    }


    return 0;
}