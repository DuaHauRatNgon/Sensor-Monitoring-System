#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pthread.h>
#include <errno.h>

#include "config.h"
#include "sbuffer.h"
#include "connmgr.h"
#include "datamgr.h"
#include "storagemgr.h"
#include "logger.h"


int main(int argc, char *argv[]) {
    
    int port = atoi(argv[1]);

    if (mkfifo(LOG_FIFO_NAME, 0666) == -1) {
        if (errno != EEXIST) {
            perror("Error creating FIFO");
            exit(EXIT_FAILURE);
        }
    }


    pid_t child_pid = fork();


    // log proccess
    if (child_pid == 0) {
        run_log_process(); 
        
        exit(EXIT_SUCCESS); 
    }

    
    // main proccess
    
    sbuffer_t *shared_buffer;
    sbuffer_init(&shared_buffer);
    

    pthread_t conn_thread, data_thread, storage_thread;
    
    pthread_create(&conn_thread, NULL, connmgr_run, (void *) argv[1]);
    pthread_create(&data_thread, NULL, datamgr_run, shared_buffer);
    pthread_create(&storage_thread, NULL, storagemgr_run, shared_buffer);


    pthread_join(conn_thread, NULL);
    pthread_join(data_thread, NULL);
    pthread_join(storage_thread, NULL);

    
    sbuffer_free(&shared_buffer);
    
    
    waitpid(child_pid, NULL, 0); 
    
    
    unlink(LOG_FIFO_NAME);

    return 0;
}