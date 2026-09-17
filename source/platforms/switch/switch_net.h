#ifndef TMC_SWITCH_NET_H
#define TMC_SWITCH_NET_H
#include <stddef.h>
long Port_Net_HttpRequest(const char* url, const char* post_data,
                          const char* content_type, char** out_body,
                          size_t* out_len);
#endif
