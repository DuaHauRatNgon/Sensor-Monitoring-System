#ifndef _SBUFFER_H_

#define _SBUFFER_H_

#include <pthread.h>
#include <stdbool.h>
#include <time.h>


/* data raw tu sensor node (TCP packet) */
typedef struct {
    uint16_t id;            // sensor id
    double value;           // temp
    time_t ts;              // timestamp
} 
sensor_data_t;


typedef struct sbuffer_node {
    sensor_data_t data;
    bool read_by_data_mgr;    
    bool read_by_storage_mgr; 
    
    struct sbuffer_node *next;
} 
sbuffer_node_t;


typedef struct sbuffer {
    sbuffer_node_t *head;
    sbuffer_node_t *tail;
    
    pthread_mutex_t mutex;     
    
    pthread_cond_t cond_new_data;   
} 
sbuffer_t;


// api cua sbuffer
int sbuffer_init(sbuffer_t **buffer);

int sbuffer_free(sbuffer_t **buffer);

int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data);

int sbuffer_read(sbuffer_t *buffer, sensor_data_t *data, int reader_id);


#endif // _SBUFFER_H_