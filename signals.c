#define _GNU_SOURCE 
#include <stdio.h> 
#include <unistd.h> 
#include <signal.h> 
#include <sys/types.h> 
#include <stdlib.h> 
#include <stdbool.h> 
#include <sys/wait.h> 
#include <sys/resource.h> 
#include <sys/time.h> 
#include <sys/stat.h> 
#include <string.h> 
 
volatile sig_atomic_t state = 0; 
 
void handle_signal(int signum, siginfo_t* info, void* extra) { 
        pid_t pid; 
        const int value = info->si_value.sival_int; 
        sigqueue(pid,SIGRTMIN + signum,  (union sigval) {.sival_int = value}); 
} 
 
void handle_sigterm (int signum) {state = 3;} 
 
int main(int argc, char* argv[]) { 
 struct sigaction sig_action; 
 memset(&sig_action, 0, sizeof(sig_action)); 
 sig_action.sa_sigaction = handle_signal; 
 sig_action.sa_flags = SA_SIGINFO; 
 sigaction(SIGRTMIN, &sig_action, NULL); 
 struct sigaction action_term; 
 memset(&action_term, 0, sizeof(action_term)); 
 action_term.sa_handler = handle_sigterm; 
 action_term.sa_flags = SA_RESTART;
 sigaction(SIGTERM, &action_term, NULL);
 printf("pid: %d", getpid()); 
 fflush(stdout); 
 while(true) { 
   if (state == 3) 
   break; 
 } 
}
