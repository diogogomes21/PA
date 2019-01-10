/**
 * @brief Several use cases for the select(2) call
 * @author Patricio R. Domingues (and PL2 class / 2018-2019)
 * @version 1.1
 */

//**********************************************************************
//                   select
//**********************************************************************

// includes
/* According to POSIX.1-2001, POSIX.1-2008 */
#include <sys/select.h>
/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

//----------------------------------------------------------------------
//  select loop: select is usually run with an outer loop
//----------------------------------------------------------------------

void close_descriptor(int *descriptors_V, int descriptor_idx,
                        int max_num_descriptors);
int get_max_descriptor(int *descriptors_V, int num_elms);
int set_read_descriptors(int *descriptors_V,int num_elms,fd_set *read_set_ptr);

int get_max_descriptor(int *descriptors_V, int num_elms){

    int max = -1;   // We assume that the minimum value is -1
    for(int i=0; i<num_elms; i++){
        if (descriptors_V[i] > max ){
            max = descriptors_V[i];
        }
    }
    return max;
}

int set_read_descriptors(int *descriptors_V,int num_elms,fd_set *read_set_ptr){
    int count_activated = 0;
    for(int i=0; i<num_elms; i++){
        if( descriptors_V[i] >= -1 ){
            FD_SET(descriptors_V[i],read_set_ptr);
            count_activated++;
        }
    }
    return count_activated;
}

void close_descriptor(int *descriptors_V, int descriptor_idx,
                        int max_num_descriptors){
    assert(descriptor_idx >= max_num_descriptors );

    close(descriptors_V[descriptor_idx]);
    descriptors_V[descriptor_idx] = -1;
}

    // We use the descriptors_V vector to hold all the descriptors
    // to be monitored
#define C_MAX_NUM_DESCRIPTORS   (64)
    int max_num_descriptors = C_MAX_NUM_DESCRIPTORS;
    size_t mem_size = sizeof(int)*max_num_descriptors;
    int *descriptors_V = malloc(mem_size);
    if(descriptors_V == NULL ){
           fprintf(stderr,"Cannot malloc %zu bytes\n",mem_size);
           exit(EXIT_FAILURE);
    }
    // descriptors_V vector is initialized to -1 (-1 => empty cell)
    memset(descriptors_V,-1,mem_size);

    struct timeval timeout;
    long timeout_sec = 10;  // adjust to match preferences
    long timeout_usec = 0;  // must be < 1E6
    assert(timeout_usec<1E6);

    int max_descriptor; // holds the max value of all descriptors
    while(1){
        // Get the maximum descriptor
        max_descriptor = get_max_descriptor(descriptors_V,
                            max_num_descriptors);
        // 1: config select (here, only for read set)
        FD_ZERO(&read_set);

        int num_set_descriptors = set_read_descriptors(
            descriptors_V, max_num_descriptors, &read_set);

        //...
        timeout.tv_sec = timeout_sec;
        timeout.tv_usec = timeout_usec;

        // 2: call select
        int ret_select = select(max_descriptor+1, &read_set,
                NULL, NULL,&timeout);
        if( ret_select == -1 ){
            ERROR(EXIT_FAILURE,"select failed");
        }
        if( ret_select == 0 ){
            printf("Timeout detected. Continuing...\n");
            continue;
        }
        if( ret_select > 0 ){
            // activity was detect in at least one descriptor
            int num_active = ret_select;
            for(int i=0; i<max_num_descriptors;i++){
                if( FD_ISSET(descriptors_V[i],&read_set) ){
                    // Read event on the descriptor
                    // Do as intended...
                    //TODO

                    num_active--;
                    continue; //????
                }
                if( num_active == 0 ){
                    // all active descriptors done
                    // bail out
                    continue;
                }
            }//for i
        }
    }//while(1)


    free(descriptors_V);

....    
