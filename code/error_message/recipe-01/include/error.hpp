#ifndef ERROR_INC
#define ERROR_INC

#include <stdarg.h>
#include <stdexcept>

inline void error(const char *format, ...)
{
    const int buf_size = 1024;
    char    buf[buf_size] = {'\0'};
    va_list ap;

    va_start (ap, format);
    vsnprintf(buf, buf_size, format, ap);
    va_end (ap);
    throw std::runtime_error(buf);
}

#endif
