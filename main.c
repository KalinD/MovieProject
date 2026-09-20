#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "parser.h"

// TODO: add struct to shoten parameter list
static unsigned long long movie_search(Movie_t const * const movies, unsigned long long const * const movies_count, char** titles, const unsigned char titles_count, const unsigned short year, char** genres, const unsigned char genres_count, char** tags, const unsigned char tags_count, Movie_t** const out_movies);

int main(int argc, char** argv) {
    unsigned long long movies_count = 0U;
    (void) get_movies_count(&movies_count);
    Movie_t* movies = (Movie_t*) calloc(movies_count, sizeof(Movie_t));
    const int ret_val = parse_all(movies, &movies_count);
    switch (ret_val) {
        case PARSE_OK:
            // Everything went ok
            break;
        case MOVIE_PARSE_ERROR:
            printf("Error while parsing movie file.\n");
            break;
        case TAGS_PARSE_ERROR:
            printf("Error while parsing tags file.\n");
            break;
        default:
            // Should be unreachable
            break;
    }

    char** title_keywords = (char**) calloc(2U, sizeof(char**)); // Assume 2 keywords
    unsigned char title_keyword_size = 0U;
    unsigned char title_max_size = 2U;

    // Year should be a single value
    unsigned short year = 0U;

    char** genres = (char**) calloc(2U, sizeof(char**)); // Assume 2 genres
    unsigned char genres_size = 0U;
    unsigned char genres_max_size = 2U;

    char** tags = (char**) calloc(2U, sizeof(char**)); // Assume 2 tags
    unsigned char tags_size = 0U;
    unsigned char tags_max_size = 2U;

    int i = 1; // Skip program name
    // TODO: arguments parsing can be extracted into a method
    while (i < argc) {
        if (0U == strcmp(argv[i], "-title")) {
            ++i;
            while ((i < argc) && ('-' != argv[i][0U])) {
                if (title_keyword_size + 1U == title_max_size) {
                    char** temp_keywords = (char**) calloc((title_max_size << 1U), sizeof(char**));
                    for (unsigned char temp_index = 0U; temp_index < title_max_size; ++temp_index) {
                        temp_keywords[temp_index] = title_keywords[temp_index];
                    }
                    title_keywords = temp_keywords;
                    title_max_size <<= 1U;
                }
                title_keywords[title_keyword_size] = argv[i];
                ++title_keyword_size;
                ++i;
            }
        } else if (0U == strcmp(argv[i], "-year")) {
            ++i;
            while ((i < argc) && ('-' != argv[i][0U])) {
                unsigned char argv_index = 0U;
                while ('\0' != argv[i][argv_index]) {
                    year = (year * 10U) + (argv[i][argv_index] - '0');
                    ++argv_index;
                }
                ++i;
            }
        } else if (0U == strcmp(argv[i], "-genre")) {
            ++i;
            while ((i < argc) && ('-' != argv[i][0U])) {
                if (genres_size + 1U == genres_max_size) {
                    char** temp_genres = (char**) calloc((genres_max_size << 1), sizeof(char**));
                    for (unsigned char temp_index = 0U; temp_index < genres_max_size; ++temp_index) {
                        temp_genres[temp_index] = genres[temp_index];
                    }
                    genres = temp_genres;
                    genres_max_size <<= 1U;
                }
                genres[genres_size] = argv[i];
                ++genres_size;
                ++i;
            }
        } else if (0U == strcmp(argv[i], "-tag")) {
            ++i;
            while ((i < argc) && ('-' != argv[i][0U])) {
                if (tags_size + 1U == tags_max_size) {
                    char** temp_tags = (char**) calloc((tags_max_size << 1), sizeof(char**));
                    for (unsigned char temp_index = 0U; temp_index < tags_max_size; ++temp_index) {
                        temp_tags[temp_index] = tags[temp_index];
                    }
                    tags = temp_tags;
                    tags_max_size <<= 1U;
                }
                tags[tags_size] = argv[i];
                ++tags_size;
                ++i;
            }
        }
    }

    Movie_t* result_movies;
    const unsigned long long movies_found_count = movie_search(movies, &movies_count, title_keywords, title_keyword_size, year, genres, genres_size, tags, tags_size, &result_movies);

    free(title_keywords);
    free(genres);
    free(tags);

    for (unsigned long long movie_index = 0U; movie_index < movies_found_count; ++movie_index) {
        printf("%llu::%s (%u)::", result_movies[movie_index].id, result_movies[movie_index].title, result_movies[movie_index].year);
        for (unsigned char genre_index = 0U; genre_index < result_movies[movie_index].genres_count - 1U; ++genre_index) {
            printf("%s|", result_movies[movie_index].genres[genre_index]);
        }
        printf("%s", result_movies[movie_index].genres[result_movies[movie_index].genres_count - 1U]);
        printf(" %.2Lf\n", result_movies[movie_index].rating);
    }

    for (unsigned long long movie_index = 0U; movie_index < movies_count; ++movie_index) {
        for (unsigned short genre_index = 0U; genre_index < movies[movie_index].genres_count; ++genre_index) {
            free(movies[movie_index].genres[genre_index]);
        }
        free(movies[movie_index].genres);
        free(movies[movie_index].title);
        for (unsigned short tag_index = 0U; tag_index < movies[movie_index].tags_count; ++tag_index) {
            free(movies[movie_index].tags[tag_index]);
        }
        free(movies[movie_index].tags);
    }

    free(result_movies);
    free(movies);
    return 0;
}

static unsigned long long movie_search(Movie_t const * const movies, unsigned long long const * const movies_count, char** titles, const unsigned char titles_count, const unsigned short year, char** genres, const unsigned char genres_count, char** tags, const unsigned char tags_count, Movie_t** const out_movies) {
    Movie_t* valid_movies = (Movie_t*) calloc(4U, sizeof(Movie_t)); // We will start will 4
    unsigned long long max_size = 4U;
    unsigned long long found_movies_count = 0U;
    for (unsigned long long index = 0U; index < *movies_count; ++index) {
        // Filter Year
        if ((0 != year) && (year != movies[index].year)) { // No movie released with Jesus
            continue;
        }

        // Filter Title
        BOOL has_missing = FALSE;
        for (unsigned char title_index = 0U; title_index < titles_count; ++title_index) {
            if (NULL == strstr(movies[index].title, titles[title_index])) {
                // One of the required keywords is missing
                has_missing = TRUE;
                break;
            }
        }
        if (TRUE == has_missing) {
            continue;
        }

        // Filter genre
        has_missing = FALSE;
        for (unsigned char genre_index = 0U; genre_index < genres_count; ++genre_index) {
            BOOL has_genre = FALSE;
            for (unsigned char movie_genre_index = 0U; movie_genre_index < movies[index].genres_count; ++movie_genre_index) {
                if (0 == strcmp(genres[genre_index], movies[index].genres[movie_genre_index])) {
                    has_genre = TRUE;
                    break;
                }
            }
            if (FALSE == has_genre) {
                has_missing = TRUE;
                break;
            }
        }

        if (FALSE != has_missing) {
            continue;
        }

        // Filter Tags
        has_missing = FALSE;
        for (unsigned char tags_index = 0U; tags_index < tags_count; ++tags_index) {
            BOOL found_tag = FALSE;
            for (unsigned short movie_tags_index = 0U; movie_tags_index < movies[index].tags_count; ++movie_tags_index) {
                if (NULL != strstr(movies[index].tags[movie_tags_index], tags[tags_index])) {
                    found_tag = TRUE;
                    break;
                }
            }
            if (FALSE == found_tag) {
                has_missing = TRUE;
                break;
            }
        }

        if (FALSE != has_missing) {
            continue;
        }

        if (found_movies_count + 1 == max_size) {
            Movie_t* temp_movies = calloc((max_size << 1U), sizeof(Movie_t));
            // TODO: switch to realloc(); ?
            for (unsigned long long temp_index = 0U; temp_index < max_size; ++temp_index) {
                temp_movies[temp_index] = valid_movies[temp_index];
            }
            free(valid_movies);
            valid_movies = temp_movies;
            temp_movies = NULL;
            max_size <<= 1U;
        }
        valid_movies[found_movies_count] = movies[index];
        ++found_movies_count;
    }
    *out_movies = valid_movies;
    return found_movies_count;
}