#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>


#ifdef __GNUC__
#define DECL_FORMAT(x, y, z) __attribute__((format(x, (y), (z))))
#else
#define DECL_FORMAT(x, y, z)
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern FILE *LogFile;

#if defined(__GNUC__) && !defined(_WIN32)
#define AL_PRINT(T, MSG, ...) fprintf(LogFile, "AL lib: %s %s: "MSG, T, __FUNCTION__ , ## __VA_ARGS__)
#else
void al_print(const char *type, const char *func, const char *fmt, ...) DECL_FORMAT(printf, 3,4);
#define AL_PRINT(T, ...) al_print((T), __FUNCTION__, __VA_ARGS__)
#endif

#ifdef __ANDROID__
#include <android/log.h>
#define LOG_ANDROID(T, MSG, ...) __android_log_print(T, "openal", "AL lib: %s: "MSG, __FUNCTION__ , ## __VA_ARGS__)
#else
#define LOG_ANDROID(T, MSG, ...) ((void)0)
#endif

enum LogLevel {
    NoLog,
    LogError,
    LogWarning,
    LogTrace,
    LogRef
};
extern enum LogLevel LogLevel;

#define TRACEREF(...) do {                                                    \
    if(LogLevel >= LogRef)                                                    \
        AL_PRINT("(--)", __VA_ARGS__);                                        \
} while(0)

#define TRACE(...) do {                                                       \
    if(LogLevel >= LogTrace)                                                  \
        AL_PRINT("(II)", __VA_ARGS__);                                        \
    LOG_ANDROID(ANDROID_LOG_DEBUG, __VA_ARGS__);                              \
} while(0)

#define WARN(...) do {                                                        \
    if(LogLevel >= LogWarning)                                                \
        AL_PRINT("(WW)", __VA_ARGS__);                                        \
    LOG_ANDROID(ANDROID_LOG_WARN, __VA_ARGS__);                               \
} while(0)

#define ERR(...) do {                                                         \
    if(LogLevel >= LogError)                                                  \
        AL_PRINT("(EE)", __VA_ARGS__);                                        \
    LOG_ANDROID(ANDROID_LOG_ERROR, __VA_ARGS__);                              \
} while(0)

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LOGGING_H */
