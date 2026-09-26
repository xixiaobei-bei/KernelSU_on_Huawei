#ifndef CTS_STRERROR_H
#define CTS_STRERROR_H

#define TS_LOG_ERR_FMT_STR "%d(%s)"
#define TS_LOG_ERR_ARG(errno) errno, cts_strerror(errno)

const char *cts_strerror(int errno);

#endif /* CTS_STRERROR_H */

