#pragma once

#include <cstddef>

size_t strlen(const char* str);

char* strcpy(char* destination, const char* source);

char* strcat(char* destination, const char* source);

char* strncat(char* destination, const char* source, const size_t count);