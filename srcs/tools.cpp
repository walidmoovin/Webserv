#include "webserv.hpp"

void *ft_memset(void *b, int c, size_t len) {
    size_t i;
    unsigned char *b_cpy;

    b_cpy = (unsigned char *)b;
    i = 0;
    while (i < len)
        *(unsigned char *)(b_cpy + i++) = (unsigned char)c;
    return ((void *)b);
}
