#include "parser.h"

static int parse_movie(const char* line, Movie_t* movie);
static int parse_tag(const char* line, unsigned long long* out_movie_id, char** out_tag);
// static int add_tag_to_movie(char* tag, unsigned char tag_size, unsigned long long movie_id, Movie_t* movies, unsigned long long movies_count);
// static int get_movie_by_id(unsigned long long movie_id, Movie_t* movies, Movie_t** out_movie);
static int get_movie_by_id(unsigned long long movie_id, Movie_t* movies, unsigned long long* out_movie_index);


int get_movies_count(unsigned long long *count) {
    FILE* file = fopen("./movies.dat", "r");

    if (NULL == file) {
        printf("File 'movies.dat' could not be opened.\n");
        return MOVIE_PARSE_ERROR;
    }

    unsigned long long movies_count = 0;
    while (TRUE != feof(file)) {
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

int parse_all(Movie_t* movies) {
    FILE* file = fopen("./movies.dat", "r");

    if (NULL == file) {
        printf("File 'movies.dat' could not be opened.\n");
        return MOVIE_PARSE_ERROR;
    }

    unsigned long long movie_index = 0;
    char line[300] = {'\0'};
    while (NULL != fgets(line, 300, file)) {
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
        return MOVIE_PARSE_ERROR;
    }

    while (NULL != fgets(line, 300, file)) { // Longest Tag line was around 111 characters
        unsigned long long movie_id = 0;
        char* tag;
        parse_tag(line, &movie_id, &tag);
        // Movie_t* current_movie = NULL;
        unsigned long long current_movie_index = 0;
        const int ret_val = get_movie_by_id(movie_id, movies, &current_movie_index);
        if (0 != ret_val) {
            // Error getting the movie!
            printf("No movie with id: %llu\n", movie_id);
            continue;
        }
        char** old_tags = movies[current_movie_index].tags;
        char** new_tags = (char**) malloc(sizeof(char*) * (movies[current_movie_index].tags_count + 1));
        unsigned short i = 0;
        if (0 != movies[current_movie_index].tags_count) {
            for (i = 0U; i < movies[current_movie_index].tags_count; ++i) {
                // strcpy(new_tags[i], current_movie->tags[i]);
                new_tags[i] = movies[current_movie_index].tags[i];
            }
        } else {
            movies[current_movie_index].tags = malloc((sizeof(char*)));
        }
        // strcpy(new_tags[i], tag)
        new_tags[i] = tag;
        if (NULL != movies[current_movie_index].tags) {
            free(movies[current_movie_index].tags);
        }
        movies[current_movie_index].tags = new_tags;
        ++movies[current_movie_index].tags_count;
        ++movie_index;
    }

    if (0 != fclose(file)) {
        printf("File 'movies.dat' could not be closed.\n");
        return MOVIE_PARSE_ERROR;
    }

    return PARSE_OK;
}

static int parse_tag(const char* line, unsigned long long* out_movie_id, char** out_tag) {
    unsigned long user_id = 0;
    unsigned short index = 0U;

    // Working on User ID - currently ignored
    while ((index < 300) && (':' != line[index])) {
        user_id = (user_id * 10) + (line[index] - '0');
        ++index;
    }
    if (index >= 300) {
        return TAGS_PARSE_ERROR;
    }
    index += 2; // Skip "::"

    // Working on Movie ID
    unsigned long long movie_id = 0;
    while ((index < 300) && (':' != line[index])) {
        movie_id = (movie_id * 10) + (line[index] - '0');
        ++index;
    }
    if (index >= 300) {
        return TAGS_PARSE_ERROR;
    }
    *out_movie_id = movie_id;
    index += 2; // Skip "::"

    // Working on Movie Tag
    const unsigned char tag_start = index;
    while (index < 299) {
        ++index;
        if ((':' == line[index]) && (':' == line[index + 1])) {
            break;
        }
    }

    if (index >= 299) {
        return TAGS_PARSE_ERROR;
    }

    char* tag = (char*) malloc(sizeof(char) * (index - tag_start + 1));
    for (unsigned short position = tag_start; position < index; ++position) {
        tag[position - tag_start] = line[position];
    }
    tag[index - tag_start] = '\0';  // Add terminating 0
    *out_tag = tag;
    index += 2; // Skip "::"

    // Working on Timestamp - Not used yet
    unsigned long long timestamp = 0;
    while ((index < 300) && ('\0' != line[index])) {
        timestamp = (timestamp * 10) + (line[index] - '0');
        ++index;
    }
    if (index >= 300) {
        return TAGS_PARSE_ERROR;
    }

    return PARSE_OK;
}

static int parse_movie(const char* line, Movie_t* movie) {
    unsigned long id = 0;
    unsigned short index = 0U;

    // Working on Movie ID
    while ((index < 300) && (':' != line[index])) {
        id = (id * 10) + (line[index] - '0');
        ++index;
    }

    if (index >= 300) {
        return MOVIE_PARSE_ERROR;
    }

    if (id == 363) {
        asm("nop");
    }
    movie->id = id;
    index += 2; // Skip "::"

    // Working on Movie Title
    // TODO: get year from title "Into the Wild (2007)"
    const unsigned char title_start = index;
    unsigned short year = 0;
    BOOL is_year_part = FALSE;
    while (index < 300) {
        ++index;
        // TODO: Fix year parsing
        // if (')' == line[index]) {
        //     is_year_part = FALSE;
        // }
        // if (FALSE != is_year_part) {
        //     year = (year * 10) + (line[index] - '0');
        // }
        // if ('(' == line[index]) {
        //     is_year_part = TRUE;
        // }
        if ((':' == line[index]) && (':' == line[index + 1])) {
            break;
        }
    }

    if (index >= 300) {
        return MOVIE_PARSE_ERROR;
    }

    char* title = (char*) malloc(sizeof(char) * (index - title_start + 1));
    for (unsigned char position = title_start; position < index; ++position) {
        title[position - title_start] = line[position];
    }
    title[index - title_start] = '\0';  // Add terminating 0
    movie->title = title;
    // movie->year = year;
    index += 2; // Skip "::"

    // Working on Movie Genres
    // TODO: Split genres to separate strings
    const unsigned char current_place = index;
    while ((index < 300) && ('\0' != line[index])) {
        ++index;
    }
    if (index >= 300) {
        return MOVIE_PARSE_ERROR;
    }

    char* genres = (char*) malloc(sizeof(char) * (index - current_place + 1));
    for (unsigned short position = current_place; position < index; ++position) {
        genres[position - current_place] = line[position];
    }
    genres[index - current_place] = '\0'; // Add terminating 0
    movie->genres = genres;

    // Init rest of params
    movie->genres_count = 0U;
    movie->tags_count = 0U;

    return PARSE_OK;
}

// static int get_movie_by_id(unsigned long long movie_id, Movie_t* movies, Movie_t** out_movie) {
static int get_movie_by_id(unsigned long long movie_id, Movie_t* movies, unsigned long long* out_movie_index) {
    unsigned long long i = 0;
    while (true) {
        // TODO: Optimize to use binary search
        Movie_t* temp = &movies[i];
        if (NULL == temp) {
            return -1;
        }
        if (movie_id == temp->id) {
            *out_movie_index = i;
            return 0;
        }
        ++i;
    }
    return -1;
}


// static int add_tag_to_movie(char* tag, unsigned char tag_size, unsigned long long movie_id, Movie_t* movies, unsigned long long movies_count) {
//     for (unsigned long long movie)
// }