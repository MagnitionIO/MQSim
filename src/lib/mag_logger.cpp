//
// Created by Khubaib Umer on 18/01/2023.
//

#include "base.h"
#include "mag_logger.h"
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <stdarg.h>
#include <sys/queue.h>
#include <fcntl.h>
#include <zlib.h>
#include <dirent.h>

#if defined(__APPLE__)

#include <dispatch/dispatch.h>

#elif defined(__linux__)
#include <semaphore.h>
#endif

static pthread_rwlock_t init_lock;

#define GENERATE_PRINT_FUNC(LEVEL) \
    void print_##LEVEL(const char *fl, const char *fn, int ln, const char *fmt, ...) { \
        return_if(self->log_level > (LEVEL), ); \
        log_info_t *info = prepare_data(fl, fn, ln, LEVEL); \
        va_list temp, args; \
        va_start(temp, fmt); \
        va_copy(args, temp); \
        size_t len = vsnprintf(nullptr, 0, fmt, temp); \
        va_end(temp); \
        info->log_line = _calloc(len + 5, char); \
        vsnprintf(info->log_line, len + 4, fmt, args); \
        va_end(args); \
        self->push_back(info); \
    }

#define DECLARE_PRINT_FUNC(LEVEL) \
    void print_##LEVEL(const char *fl, const char *fn, int ln, const char *fmt, ...)

#define FUNC_PTR_PRINT(LEVEL) print_##LEVEL

#define GET_PRIVATE(x) (x)->p_data

/* Forward Declarations */
typedef struct log_info log_info_t;

DECLARE_PRINT_FUNC(LOG_TRACE);

DECLARE_PRINT_FUNC(LOG_DEBUG);

DECLARE_PRINT_FUNC(LOG_INFO);

DECLARE_PRINT_FUNC(LOG_WARN);

DECLARE_PRINT_FUNC(LOG_ERROR);

log_info_t *prepare_data(const char *fl, const char *fn, int ln, kLogLevel level);

BOOL change_log_level(kLogLevel level);

void change_file(void);

void *logging_thread(void *args);

BOOL queue_lock(void);

void queue_unlock(void);

BOOL push_to_queue(log_info_t *);

log_info_t *pop_from_queue(void);

void *zip_file_routine(void *);

void zip_all_uncompressed_files(void);

BOOL is_logger_init();

typedef struct log_info {
    tid_t tid;
    kLogLevel level;
    int line;
    char *file;
    char *func;
    char *log_line;

    TAILQ_ENTRY(log_info) entries;
} log_info_t;

typedef struct {
    logger_t *logger;
    size_t current_file_size;
    char *file_path;
    char *service;
    int64_t max_file_size;
    BOOL max_size_set;
    char *filename;
    size_t filename_len;
    FILE *logfile;
    kLogLevel log_level;
    BOOL is_running;
    pthread_t loggerThread;
    pthread_rwlock_t workerLock;

    struct {
        TAILQ_HEAD(tailhead, log_info) head;
        pthread_rwlock_t queue_lock;
    } p_data;

    struct {
#if defined(__APPLE__)
        dispatch_semaphore_t sem;
#elif defined(__linux__)
        sem_t sem;
#endif
        pthread_t tid;

        void (*send_signal)(void);

        void (*wait_signal)(void);
    } file_zip;

    BOOL (*lock)(void);

    void (*unlock)(void);

    BOOL (*push_back)(log_info_t *);

    log_info_t *(*pop_front)(void);

} loggerData_t;

static logger_t logger = {
        .trace = &FUNC_PTR_PRINT(LOG_TRACE),
        .debug = &FUNC_PTR_PRINT(LOG_DEBUG),
        .info = &FUNC_PTR_PRINT(LOG_INFO),
        .warn = &FUNC_PTR_PRINT(LOG_WARN),
        .error = &FUNC_PTR_PRINT(LOG_ERROR),

        .update_log_level = &change_log_level,
        .is_initialized = &is_logger_init,
        .cycle_file = &change_file
};

struct {
    kLogLevel first;
    const char *second;
} static const kLogLevels[] = {
        {.first = LOG_TRACE, .second = "TRACE"},
        {.first = LOG_DEBUG, .second = "DEBUG"},
        {.first = LOG_INFO, .second = "INFO"},
        {.first = LOG_WARN, .second = "WARN"},
        {.first = LOG_ERROR, .second = "ERROR"},
        {.first = LOG_NONE, .second = "Invalid"}
};

static loggerData_t *self = nullptr;

GENERATE_PRINT_FUNC(LOG_TRACE)

GENERATE_PRINT_FUNC(LOG_DEBUG)

GENERATE_PRINT_FUNC(LOG_INFO)

GENERATE_PRINT_FUNC(LOG_WARN)

GENERATE_PRINT_FUNC(LOG_ERROR)

void send_job_signal(void) {
#if defined(__APPLE__)
    dispatch_semaphore_signal(self->file_zip.sem);
#elif defined(__linux__)
    sem_post (&self->file_zip.sem);
#endif
}

void wait_for_job_signal(void) {
#if defined(__APPLE__)
    dispatch_semaphore_wait(self->file_zip.sem, DISPATCH_TIME_FOREVER);
#elif defined(__linux__)
    sem_wait (&self->file_zip.sem);
#endif
}

void init_semaphore() {
#if defined(__APPLE__)
    self->file_zip.sem = dispatch_semaphore_create(0);
#elif defined (__linux__)
    do_assert(sem_init (&self->file_zip.sem, 0, 0) == 0);
#endif
}

void destroy_semaphore() {
#if defined(__APPLE__)
    dispatch_release(self->file_zip.sem);
#elif defined(__linux__)
    sem_destroy(&self->file_zip.sem);
#endif
}

BOOL is_logger_init() { return (BOOL) (self != nullptr); }

void logger_free(void *ptr) {
    if (ptr) {
        free(ptr);
    }
}

char *mkfile_name(bool startup, size_t *len) {
    time_t timer;
    char buffer[64] = {0};
    struct tm *tm_info;
    struct timespec cur_time = {0};

    timer = time(nullptr);
    tm_info = localtime(&timer);
    clock_gettime(CLOCK_MONOTONIC, &cur_time);

    size_t millis = (size_t) (cur_time.tv_nsec / 1.0e6);

    size_t bytes = strftime(buffer, 48, "%Y_%m_%d_%H_%M_%S", tm_info);
    *len = 40 + bytes;
    char *time_stamp = _calloc(*len, char);
    if (startup) {
        snprintf(time_stamp, *len, "%s_LogFile_%s_%03zu_startup.log", self->service, buffer, millis);
    } else {
        snprintf(time_stamp, *len, "%s_LogFile_%s_%03zu.log", self->service, buffer, millis);
    }
    return time_stamp;
}

__attribute__((constructor)) void init_big_lock() {
    printf("Init Logger Giant Lock\n");
    pthread_rwlock_init(&init_lock, NULL);
}

__attribute__((destructor)) void cleanup_big_lock() {
    pthread_rwlock_destroy(&init_lock);
    printf("Destroy Logger Giant Lock\n");
}

logger_t *create_logger(const char *service, const char *path) {
    pthread_rwlock_wrlock(&init_lock);
    if (self == nullptr) {
        self = _calloc(1, loggerData_t);
        self->service = strdup(service);
        self->logger = &logger;
        pthread_rwlock_init(&GET_PRIVATE(self).queue_lock, 0);
        pthread_rwlock_init(&self->workerLock, 0);
        if (path != NULL)
            self->file_path = strdup(path);
        self->lock = &queue_lock;
        self->unlock = &queue_unlock;
        self->push_back = &push_to_queue;
        self->pop_front = &pop_from_queue;
    } else {
        pthread_rwlock_unlock(&init_lock);
        return self->logger;
    }

    self->is_running = True;
    if (path != NULL) {
        do_assert(chdir(self->file_path) == 0);
        self->filename = mkfile_name(True, &self->filename_len);
        self->logfile = fopen(self->filename, "w+");
    } else {
        self->logfile = stdout;
    }
    TAILQ_INIT(&GET_PRIVATE(self).head);

    pthread_create(&self->loggerThread, nullptr, logging_thread, nullptr);
    pthread_rwlock_unlock(&init_lock);
    return self->logger;
}

BOOL set_max_file_size(int64_t max) {
    return_if(self == nullptr, False);
    self->lock();

    if (self->max_size_set == False) {
        self->max_file_size = max;
        self->max_size_set = True;
    }
    if (max != -1) {
        init_semaphore();
        self->file_zip.send_signal = &send_job_signal;
        self->file_zip.wait_signal = &wait_for_job_signal;
        pthread_create(&self->file_zip.tid, nullptr, zip_file_routine, nullptr);
    }
    self->unlock();
    return True;
}

BOOL change_log_level(kLogLevel level) {
    self->log_level = level;
    return True;
}

logger_t *get_logger(void) {
    return self->logger;
}

BOOL queue_lock(void) {
    pthread_rwlock_wrlock(&GET_PRIVATE(self).queue_lock);
    return True;
}

void queue_unlock(void) {
    pthread_rwlock_unlock(&GET_PRIVATE(self).queue_lock);
}

BOOL close_logger(void) {
    self->lock();
    self->is_running = False;
    self->unlock();
//    if (self->max_file_size != -1)
//        pthread_cancel(self->file_zip.tid);
//    pthread_cancel(self->loggerThread);
    return True;
}

log_info_t *prepare_data(const char *fl, const char *fn, int ln, kLogLevel level) {
    log_info_t *info = _calloc(1, log_info_t);
    info->level = level;
    info->tid = get_tid();
    info->line = ln;
    info->file = (fl) ? strdup(fl) : strdup("??");
    info->func = (fn) ? strdup(fn) : strdup("??");
    return info;
}

BOOL push_to_queue(log_info_t *node) {
    self->lock();
    TAILQ_INSERT_TAIL(&GET_PRIVATE(self).head, node, entries);
    self->unlock();
    return True;
}

log_info_t *pop_from_queue() {
    self->lock();
    log_info_t *msg = GET_PRIVATE(self).head.tqh_first;
    TAILQ_REMOVE(&GET_PRIVATE(self).head, GET_PRIVATE(self).head.tqh_first, entries);
    self->unlock();
    return msg;
}

const char *get_log_str(kLogLevel level) {
    switch (level) {
        case LOG_TRACE ... LOG_NONE:
            return kLogLevels[level].second;
        default:
            return kLogLevels[LOG_NONE].second;
    }
}

void change_file() {
    if (self->max_file_size != -1) {
        printf("Changing File, current File Size %zu\n", self->current_file_size);
        fclose(self->logfile);
        self->file_zip.send_signal(); // Post compressor thread a new Job
        logger_free(self->filename);
        self->filename = mkfile_name(False, &self->filename_len);
        self->logfile = fopen(self->filename, "w+");
        do_assert(self->logfile != nullptr);
        self->current_file_size = 0;
    }
}

char *get_current_time_stamp() {
    time_t timer;
    char buffer[48] = {0};
    struct tm *tm_info;
    struct timespec curtime = {0};

    timer = time(nullptr);
    tm_info = localtime(&timer);
    clock_gettime(CLOCK_MONOTONIC, &curtime);
    size_t millis = (size_t) (curtime.tv_nsec / 1.0e6);

    size_t time_len = strftime(buffer, sizeof(buffer), "%Y %m %d %H:%M:%S", tm_info);
    size_t len = strnlen(buffer, time_len) + 10;
    char *ts = _calloc(len, char);
    snprintf(ts, len, "%s,%03zu", buffer, millis);
    return ts;
}

void *logging_thread(void *args) {
    (void) args;
    const struct timespec sleep_time = {
            .tv_sec = 0,
            .tv_nsec = 10
    };

    while (self->is_running) {
        while (self->is_running && self->lock() && TAILQ_EMPTY(&GET_PRIVATE(self).head)) {
            self->unlock();
            nanosleep(&sleep_time, nullptr);
        }
        if (!self->is_running) {
            goto cleanup;
        }
        self->unlock();

        char *ts = get_current_time_stamp();
        log_info_t *msg = self->pop_front();
        self->current_file_size +=
                fprintf(self->logfile, "%s [%llu] %s : {%s:%d} %s > [ %s ]\n", ts, msg->tid, get_log_str(msg->level),
                        msg->file, msg->line,
                        msg->func, msg->log_line);
        fflush(self->logfile);
        logger_free(msg->func);
        logger_free(msg->file);
        logger_free(msg->log_line);
        logger_free(msg);
        logger_free(ts);

//        if (self->current_file_size >= ((size_t) self->max_file_size)) {
//            self->logger->cycle_file();
//        }
    }
    cleanup:
    fclose(self->logfile);
    logger_free(self->file_path);
    logger_free(self->service);
    logger_free(self);
    pthread_exit(0);
}

unsigned long file_size(char *filename) {
    FILE *pFile = fopen(filename, "rb");
    fseek(pFile, 0, SEEK_END);
    unsigned long size = ftell(pFile);
    fclose(pFile);
    return size;
}

BOOL do_compress(char *in, char *out) {
    FILE *infile = fopen(in, "r");
    gzFile outfile = gzopen(out, "wb");

    return_if(!infile || !outfile, False);

    char inbuffer[KB(4)] = {0};
    size_t num_read;
    size_t total_read = 0;
    while ((num_read = fread(inbuffer, 1, sizeof(inbuffer), infile)) > 0) {
        total_read += num_read;
        gzwrite(outfile, inbuffer, num_read);
    }
    fclose(infile);
    remove(in);
    gzclose(outfile);
    printf("Read %zu bytes, Wrote %lu bytes, Compression factor %4.2f%%n\n",
           total_read,
           file_size(out),
           (1.0 - file_size(out) * 1.0 / total_read) * 100.0);

    return True;
}

void *zip_file_routine(void *args) {
    (void) args;
    zip_all_uncompressed_files();
    while (self->is_running) {
        self->file_zip.wait_signal();
        zip_all_uncompressed_files();
    }
    return nullptr;
}

void zip_all_uncompressed_files(void) {
    DIR *cwd;
    struct dirent *dir;

    cwd = opendir(self->file_path);
    do_assert(cwd != nullptr);
    while ((dir = readdir(cwd)) != nullptr) {
        char *file = strdup(dir->d_name);
        if ((file[0] == '.') || (strstr(file, ".z") != nullptr)
            || (strncmp(file, self->filename, self->filename_len) == 0)) {
            logger_free(file);
            continue;
        } else {
            size_t len = strlen(file) + 3;
            char *compressed_filename = _calloc(len, char);
            snprintf(compressed_filename, len, "%s.z", file);
            do_compress(file, compressed_filename);
            printf("Original file %s, New File Name: %s\n", file, compressed_filename);
            logger_free(compressed_filename);
        }
        logger_free(file);
    }
    closedir(cwd);
}
