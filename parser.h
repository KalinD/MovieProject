#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

#define PARSE_OK 0
#define MOVIE_PARSE_ERROR 1
#define TAGS_PARSE_ERROR 2

#define MAX_LINE_LENGTH 300U // Longest found was 200-250
#define YEAR_STRING_SIZE 7U // Handle the space, parentheses, and year " (2005)"

int get_movies_count(unsigned long long * const count);
int parse_all(Movie_t * const movies, unsigned long long const * const movies_size);

#endif // PARSER_H
