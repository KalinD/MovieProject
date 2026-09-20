#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

static int parse_movie(char const * const line, Movie_t * const movie);
static int parse_tag(char const * const line, unsigned long long * const out_movie_id, char ** const out_tag);
static int parse_rating(char const * const line, unsigned long long * const out_movie_id, unsigned char * const out_rating);
static int get_movie_by_id(unsigned long long const * const movie_id, Movie_t const * const movies, unsigned long long const * const movies_size, unsigned long long * const out_movie_index);

int get_movies_count(unsigned long long * const count) {
    FILE* file = fopen("./movies.dat", "r");

    if (NULL == file) {
        printf("File 'movies.dat' could not be opened.\n");
        return MOVIE_PARSE_ERROR;
    }

    unsigned long long movies_count = 0;
    while (false == feof(file)) {
        if ('\n' == fgetc(file)) {
            ++movies_count;
        }
    }

    if (0 != fclose(file)) {
        printf("File 'movies.dat' could not be closed.\n");
        return MOVIE_PARSE_ERROR;
    }

    *count = movies_count;
    return PARSE_OK;
}

int parse_all(Movie_t * const movies, unsigned long long const * const movies_size) {
    FILE* file = fopen("./movies.dat", "r");

    if (NULL == file) {
        printf("File 'movies.dat' could not be opened.\n");
        return MOVIE_PARSE_ERROR;
    }

    unsigned long long movie_index = 0;
    char line[MAX_LINE_LENGTH] = {'\0'};
    while (NULL != fgets(line, MAX_LINE_LENGTH, file)) {
        parse_movie(line, &movies[movie_index]);
        ++movie_index;
    }

    if (0 != fclose(file)) {
        printf("File 'movies.dat' could not be closed.\n");
        return MOVIE_PARSE_ERROR;
    }

    // Get Tags
    file = fopen("./tags.dat", "r");
    if (NULL == file) {
        printf("File 'tags.dat' could not be opened.\n");
        return TAGS_PARSE_ERROR;
    }

    while (NULL != fgets(line, MAX_LINE_LENGTH, file)) { // Longest Tag line was around 111 characters
        unsigned long long movie_id = 0;
        char* tag;
        parse_tag(line, &movie_id, &tag);

        unsigned long long current_movie_index = 0;
        const int ret_val = get_movie_by_id(&movie_id, movies, movies_size, &current_movie_index);
        if (0 != ret_val) {
            // Error getting the movie!
            printf("No movie with id: %llu\n", movie_id);
            continue;
        }
        char** new_tags = (char**) calloc((movies[current_movie_index].tags_count + 1), sizeof(char*));
        unsigned short i = 0;
        if (0 != movies[current_movie_index].tags_count) {
            for (i = 0U; i < movies[current_movie_index].tags_count; ++i) {
                new_tags[i] = movies[current_movie_index].tags[i];
            }
        } else {
            movies[current_movie_index].tags = calloc(1, (sizeof(char*)));
        }

        new_tags[i] = tag;
        if (NULL != movies[current_movie_index].tags) {
            free(movies[current_movie_index].tags);
        }
        movies[current_movie_index].tags = new_tags;
        ++movies[current_movie_index].tags_count;
        ++movie_index;
    }

    if (0 != fclose(file)) {
        printf("File 'tags.dat' could not be closed.\n");
        return TAGS_PARSE_ERROR;
    }

    // Get Ratings
    file = fopen("./ratings.dat", "r");
    if (NULL == file) {
        printf("File 'ratings.dat' could not be opened.\n");
        return RATINGS_PARSE_ERROR;
    }

    while (NULL != fgets(line, MAX_LINE_LENGTH, file)) { // Longest Tag line was around 111 characters
        unsigned long long movie_id = 0;
        unsigned char rating;
        parse_rating(line, &movie_id, &rating);

        unsigned long long current_movie_index = 0;
        const int ret_val = get_movie_by_id(&movie_id, movies, movies_size, &current_movie_index);
        if (0 != ret_val) {
            // Error getting the movie!
            printf("No movie with id: %llu\n", movie_id);
            continue;
        }

        movies[current_movie_index].rating = ((movies[current_movie_index].rating * movies[current_movie_index].ratings_count) + rating) / (movies[current_movie_index].ratings_count + 1U);
        ++movies[current_movie_index].ratings_count;
    }

    if (0 != fclose(file)) {
        printf("File 'ratings.dat' could not be closed.\n");
        return MOVIE_PARSE_ERROR;
    }

    return PARSE_OK;
}

static int parse_rating(char const * const line, unsigned long long * const out_movie_id, unsigned char * const out_rating) {
    // UserID::MovieID::Rating::Timestamp
    unsigned long user_id = 0U;
    unsigned short index = 0U;

    // Working on User ID - currently ignored
    while ((index < MAX_LINE_LENGTH) && (':' != line[index])) {
        user_id = (user_id * 10U) + (line[index] - '0');
        ++index;
    }
    if (index >= (MAX_LINE_LENGTH - 1U)) {
        return TAGS_PARSE_ERROR;
    }
    index += 2U; // Skip "::"

    // Working on Movie ID
    unsigned long long movie_id = 0U;
    while ((index < MAX_LINE_LENGTH) && (':' != line[index])) {
        movie_id = (movie_id * 10U) + (line[index] - '0');
        ++index;
    }
    if (index >= (MAX_LINE_LENGTH - 1U)) {
        return TAGS_PARSE_ERROR;
    }
    *out_movie_id = movie_id;
    index += 2U; // Skip "::"

    // Working on Rating
    // Should be 1 character
    unsigned char rating = 0U;
    if (('0' <= line[index]) && (line[index] <= '5')) { // ratings go to 5
        rating = (line[index] - '0');
    } else {
        printf("Trying to parse rating %c!\n", line[index]);
        return RATINGS_PARSE_ERROR;
    }
    *out_rating = rating;
    index += 2U; // Skip "::"

    // Working on Timestamp - Not used yet
    unsigned long long timestamp = 0U;
    while ((index < MAX_LINE_LENGTH) && ('\0' != line[index])) {
        timestamp = (timestamp * 10U) + (line[index] - '0');
        ++index;
    }
    if (index >= MAX_LINE_LENGTH) {
        return TAGS_PARSE_ERROR;
    }

    return PARSE_OK;
}

static int parse_tag(char const * const line, unsigned long long * const out_movie_id, char ** const out_tag) {
    unsigned long user_id = 0U;
    unsigned short index = 0U;

    // Working on User ID - currently ignored
    while ((index < MAX_LINE_LENGTH) && (':' != line[index])) {
        user_id = (user_id * 10U) + (line[index] - '0');
        ++index;
    }
    if (index >= (MAX_LINE_LENGTH - 1U)) {
        return TAGS_PARSE_ERROR;
    }
    index += 2U; // Skip "::"

    // Working on Movie ID
    unsigned long long movie_id = 0U;
    while ((index < MAX_LINE_LENGTH) && (':' != line[index])) {
        movie_id = (movie_id * 10U) + (line[index] - '0');
        ++index;
    }
    if (index >= (MAX_LINE_LENGTH - 1U)) {
        return TAGS_PARSE_ERROR;
    }
    *out_movie_id = movie_id;
    index += 2U; // Skip "::"

    // Working on Movie Tag
    const unsigned char tag_start = index;
    while (index < (MAX_LINE_LENGTH - 1U)) {
        ++index;
        if ((':' == line[index]) && (':' == line[index + 1U])) {
            break;
        }
    }

    if (index >= (MAX_LINE_LENGTH - 1U)) {
        return TAGS_PARSE_ERROR;
    }

    char* tag = (char*) calloc((index - tag_start + 1U), sizeof(char));
    for (unsigned short position = tag_start; position < index; ++position) {
        tag[position - tag_start] = line[position];
    }
    tag[index - tag_start] = '\0';  // Add terminating 0
    *out_tag = tag;
    index += 2U; // Skip "::"

    // Working on Timestamp - Not used yet
    unsigned long long timestamp = 0U;
    while ((index < MAX_LINE_LENGTH) && ('\0' != line[index])) {
        timestamp = (timestamp * 10U) + (line[index] - '0');
        ++index;
    }
    if (index >= MAX_LINE_LENGTH) {
        return TAGS_PARSE_ERROR;
    }

    return PARSE_OK;
}

static int parse_movie(char const * const line, Movie_t * const movie) {
    unsigned long id = 0U;
    unsigned short index = 0U;

    // Working on Movie ID
    while ((index < MAX_LINE_LENGTH) && (':' != line[index])) {
        id = (id * 10U) + (line[index] - '0');
        ++index;
    }

    if (index >= (MAX_LINE_LENGTH - 1U)) {
        return MOVIE_PARSE_ERROR;
    }

    movie->id = id;
    index += 2; // Skip "::"

    // Working on Movie Title
    const unsigned char title_start = index;
    unsigned short year = 0U;
    while (index < (MAX_LINE_LENGTH - 1U)) {
        ++index;
        if ((':' == line[index]) && (':' == line[index + 1U])) {
            break;
        }
    }

    if (index >= (MAX_LINE_LENGTH - 1U)) {
        return MOVIE_PARSE_ERROR;
    }

    char* title = (char*) calloc((((index - title_start) - YEAR_STRING_SIZE) + 1), sizeof(char));
    for (unsigned short position = title_start; position < (index - YEAR_STRING_SIZE); ++position) {
        title[position - title_start] = line[position];
    }
    // 2U is for space and '('
    // 4U is because movies exist only with year with 4 digits
    for (unsigned short year_index = (index - YEAR_STRING_SIZE + 2U); year_index < (unsigned short)((index - YEAR_STRING_SIZE) + (2U + 4U)); ++year_index) {
        year = (year * 10U) + (line[year_index] - '0');
    }
    title[index - title_start - YEAR_STRING_SIZE] = '\0';  // Add terminating 0
    movie->title = title;
    movie->year = year;
    index += 2U; // Skip "::"

    // Working on Movie Genres
    unsigned char genre_start = index;
    while ((index < MAX_LINE_LENGTH) && ('\0' != line[index]) && ('\n' != line[index])) {
        ++index;
    }
    if (index >= (MAX_LINE_LENGTH - 1U)) {
        return MOVIE_PARSE_ERROR;
    }

    char** genres = (char**) calloc(2U, sizeof(char*));
    unsigned short genres_count = 0U;
    unsigned short genres_max_size = 2U;
    for (unsigned short position = genre_start; position < index; ++position) {
        while ((MAX_LINE_LENGTH > position) && ('|' != line[position]) && ('\n' != line[position]) && '\0' != line[position]) {
            ++position;
        }
        char* genre = (char*) calloc(((position - genre_start) + 1U), sizeof(char));
        for (unsigned short genre_index = genre_start; genre_index < position; ++genre_index) {
            genre[genre_index - genre_start] = line[genre_index];
        }
        genre[position - genre_start] = '\0'; // Add terminating 0
        if ('|' == line[position]) {
            genre_start = position + 1U;
        }
        if (genres_count + 1U >= genres_max_size) {
            char** temp_genres = (char**) calloc(genres_max_size << 1, sizeof(char*));
            for (unsigned short temp_index = 0U; temp_index < genres_count; ++temp_index) {
                temp_genres[temp_index] = genres[temp_index];
            }
            free(genres);
            genres = temp_genres;
            genres_max_size <<= 1;
        }
        genres[genres_count] = genre;
        ++genres_count;
    }
    movie->genres = genres;
    movie->genres_count = genres_count;

    // Init rest of params
    movie->tags_count = 0U;

    return PARSE_OK;
}

static int get_movie_by_id(unsigned long long const * const movie_id, Movie_t const * const movies, unsigned long long const * const movies_size, unsigned long long * const out_movie_index) {
    unsigned long long i = 0U;
    // log_2(10681) = 13.4 (10681 is the number of values in current .dat file
    // Can be calculated dynamically.
    // For values up to 15 (just in case) handle linearly.
    // Everything else is handled with binary search for log_2 efficiency.
    if (*movie_id <= 15) {
        for (unsigned short index = 0; index <= 15; ++index) {
            if (*movie_id == movies[index].id) {
                *out_movie_index = index;
                return 0;
            }
        }
        return  -1;
    }
    unsigned long long l = 0U;
    unsigned long long r = (*movies_size - 1U);
    while (l <= r) {
        const unsigned long long m = l + (r - l) / 2U;
        if (*movie_id == movies[m].id) {
            *out_movie_index = m;
            return 0;
        } else if (*movie_id > movies[m].id) {
            l = m + 1;
        } else if (*movie_id < movies[m].id) {
            r = m - 1;
        }
    }
    return -1;
}
