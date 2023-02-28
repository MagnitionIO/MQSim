//
// Created by Khubaib Umer on 18/01/2023.
//

#ifndef LIB_MQSIM_H_
#define LIB_MQSIM_H_

#include "base.h"
#include "logger.h"
#include <stdlib.h>

enum kREQUEST_TYPE {
    INVALID = -1, WRITE = 0, READ
};

#define DEFINE_INTEGRATION_BIT extern bool is_integration_enabled;

#define TEST_INTEGRATION_BIT (is_integration_enabled == true)

typedef struct request_type {
#ifdef __cplusplus

    request_type(kREQUEST_TYPE _type, request_id_t _req_id, uint16_t _id, uint64_t _time, uint64_t _size, uint64_t _lba)
            : type(_type), req_id(_req_id), dev_id(_id), arrival_time(_time), size_in_sectors(_size),
              start_sector_addr(_lba) {}

    request_type()
            : type(INVALID), req_id(0), dev_id(0), arrival_time(0), size_in_sectors(0), start_sector_addr(0) {};
#endif
    enum kREQUEST_TYPE type;
    uint16_t dev_id;
    uint64_t arrival_time;
    uint64_t size_in_sectors;
    uint64_t start_sector_addr;
    uint64_t req_id;
} request_type_t;

typedef struct {
    void *host;
    void *ssd;
    void *engine;
    void *datastore;
} system_t;

typedef uint64_t (*cb_f)(void);

typedef void (*done_cb_f)(uint64_t);

///< @fn init_system
///< @param handle handle for library instance
///< @param ssd_cfg path to ssd_cfg xml
///< @brief Initializes Host and SSD using the ssd_cfg file for Integration Based Flow
EXTERN_C bool init_system(system_t **handle, const char *ssd_cfg);

///< @fn ignite_system
///< @brief Starts MQSim Servicing thread to service incoming requests
EXTERN_C void ignite_system();

///< @fn set_callbacks
///< @param clock callback fptr to logical clock
///< @param req_complete callback for request completion
///< @param timer callback for starting wakeup timer
///< @brief Sets callback functions for libMQSim
EXTERN_C bool set_callbacks(cb_f clock, done_cb_f req_complete, done_cb_f timer);

///< @fn dispatch_request
///< @param req User Request for simulation
///< @brief Dispatches a request to MQSim Servicing Thread for Simulation
EXTERN_C bool dispatch_request(request_type_t *req);

///< @fn timer_fired
///< @param requested_time requested wakeup time
///< @brief Intimate libMQSim that the wakeup time has expired
EXTERN_C void timer_fired(uint64_t requested_time);

///< @fn destroy
///< @param system library instance handle
///< @brief Destroy the integration Layer and exit
EXTERN_C void destroy(system_t *system);

///< @fn process_request
///< @param handle handle for library instance
///< @param request IO Request
///< @param required_time next scheduling time
///< @param curr_time current system logical time
///< @param queue Insertion to Queue function
///< @brief Synchronous mode request processing
EXTERN_C bool process_request(system_t *handle, request_type_t *request,
                              uint64_t *required_time, uint64_t curr_time,
                              insert_to_queue_f queue);

///< @fn dump_results
///< @param handle handle for library instance
///< @param curr_time current system logical time
///< @brief Collect results to compare with MQSim current result
///< until we have our mechanism to produce per request delays
EXTERN_C void dump_results(system_t *handle, uint64_t curr_time);

///< @fn getSimTime
///< @param handle handle for library instance
///< @brief Get MQSim current logical time
EXTERN_C uint64_t getSimTime(system_t *handle);

EXTERN_C void print_handle(system_t *handle);

EXTERN_C void *getNextStoreEntry(system_t *handle);

EXTERN_C void register_engine(void *handle);
EXTERN_C void deregister_engine(void *handle);

EXTERN_C void register_datastore(void *handle);
EXTERN_C void deregister_datastore(void *handle);

EXTERN_C void register_host(void *handle);
EXTERN_C void deregister_host(void *handle);

#endif /* LIB_MQSIM_H_ */
