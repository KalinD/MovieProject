#ifndef COMMON_H
#define COMMON_H

#define BOOL unsigned char
#define TRUE (0 == 0)
#define FALSE (1 == 0)

typedef struct {
    unsigned long long id;
    char* title;
    unsigned short year;
    char** genres;
    unsigned char genres_count;
    char** tags;
    unsigned short tags_count;
    unsigned long long ratings_count;
    long double rating;
} Movie_t;

#endif // COMMON_H
