//
// Created by Khubaib Umer on 19/01/2023.
//

#ifndef JOBCONTROLLER_H
#define JOBCONTROLLER_H

#include <stdlib.h>

typedef struct {
    bool (*add_job)(void *self, void *job);

    void *(*get_job)(void *self);

    uint64_t (*get_job_id)(void *job);

    void *(*get_job_data)(void *job);

    void (*delete_job)(void *job);

    // for internal use only
    void *internal;
} job_controller_t;

job_controller_t *new_job_controller();

#endif //JOBCONTROLLER_H
