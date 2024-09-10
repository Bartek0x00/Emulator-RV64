#include <stdio.h>
#include <stdlib.h>

#pragma once

inline void error(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}