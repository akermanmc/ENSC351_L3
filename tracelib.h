#ifndef PROJECT_TRACELIB_H
#define PROJECT_TRACELIB_H

#ifdef __cplusplus
extern "C" {
#endif

void trace_start(const char* filename);

void trace_event_start(const char* name, const char* categories, const char* arguments);

void trace_event_end(const char* arguments);

void trace_instant_global(const char* name);

void trace_object_new(const char* name, const void* obj_pointer);

void trace_object_gone(const char* name, const void* obj_pointer);

void trace_counter(const char* name, const char* key, const char* value);

void trace_flush();

void trace_end();

#ifdef __cplusplus
}
#endif

#endif //PROJECT_TRACELIB_H
