#include <unistd.h>

#include "common.h"
#include "parser.h"

static unsigned long long movie_search(Movie_t* movies, unsigned long long movies_count, char** titles, unsigned char titles_count, unsigned short year, char** genres, unsigned char genres_count, char** tags, unsigned char tags_count, Movie_t** out_movies);

int main(int argc, char** argv) {
    unsigned long long movies_count = 0;
    (void) get_movies_count(&movies_count);
    Movie_t* movies = (Movie_t*) malloc(sizeof(Movie_t) * movies_count);
    switch (parse_all(movies)) {
        case PARSE_OK:
            // Everything went ok
            // for (int i = 0; i < 10; ++i) {
            //     printf("%s\n", movies[i].title);
            // }
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

    // char* opt;
    // while((opt = getopt(argc, argv, “:if:lrx”)) != -1)
    // {
    //     switch(opt)
    //     {
    //         case ‘i’:
    //         case ‘l’:
    //         case ‘r’:
    //             printf(“option: %c\n”, opt);
    //             break;
    //         case ‘f’:
    //             printf(“filename: %s\n”, optarg);
    //             break;
    //         case ‘:’:
    //             printf(“option needs a value\n”);
    //             break;
    //         case ‘?’:
    //             printf(“unknown option: %c\n”, optopt);
    //             break;
    //     }
    // }

    char** title_keywords = (char**) malloc(sizeof(char**) * 2); // Assume 2 keywords
    unsigned char title_keyword_size = 0U;
    unsigned char title_max_size = 2U;

    unsigned short* years = (unsigned short*) calloc(2, sizeof(unsigned short)); // Probably only one year but just in case
    unsigned char years_size = 0U;
    unsigned char years_max_size = 2;

    char** genres = (char**) malloc(sizeof(char**) * 2); // Assume 2 genres
    unsigned char genres_size = 0U;
    unsigned char genres_max_size = 2U;

    char** tags = (char**) malloc(sizeof(char**) * 2); // Assume 2 tags
    unsigned char tags_size = 0U;
    unsigned char tags_max_size = 2U;

    int i = 1; // Skip program name
    while (i < argc) {
        if (0 == strcmp(argv[i], "-title")) {
            ++i;
            while ((i < argc) && ('-' != argv[i][0])) {
                if (title_keyword_size + 1U == title_max_size) {
                    char** temp_keywords = (char**) malloc(sizeof(char**) * (title_max_size * 2));
                    for (unsigned char temp_index = 0U; temp_index < title_max_size; ++temp_index) {
                        temp_keywords[temp_index] = title_keywords[temp_index];
                    }
                    title_keywords = temp_keywords;
                    title_max_size <<= 1;
                }
                title_keywords[title_keyword_size] = argv[i];
                ++title_keyword_size;
                ++i;
            }
        } else if (0 == strcmp(argv[i], "-year")) {
            ++i;
            while ((i < argc) && ('-' != argv[i][0])) {
                if (years_size + 1U == years_max_size) {
                    unsigned short* temp_years = (unsigned short*) malloc(sizeof(unsigned short*) * (years_max_size * 2));
                    for (unsigned char temp_index = 0U; temp_index < years_max_size; ++temp_index) {
                        temp_years[temp_index] = years[temp_index];
                    }
                    years = temp_years;
                    years_max_size <<= 1;
                }
                years[years_size] = 0;
                unsigned char argv_index = 0U;
                while ('\0' != argv[i][argv_index]) {
                    years[years_size] = (years[years_size] * 10) + (argv[i][argv_index] - '0');
                    ++argv_index;
                }
                ++years_size;
                ++i;
            }
        } else if (0 == strcmp(argv[i], "-genre")) {
            ++i;
            while ((i < argc) && ('-' != argv[i][0])) {
                if (genres_size + 1U == genres_max_size) {
                    char** temp_genres = (char**) malloc(sizeof(char**) * (genres_max_size * 2));
                    for (unsigned char temp_index = 0U; temp_index < genres_max_size; ++temp_index) {
                        temp_genres[temp_index] = genres[temp_index];
                    }
                    genres = temp_genres;
                    genres_max_size <<= 1;
                }
                genres[genres_size] = argv[i];
                ++genres_size;
                ++i;
            }
        } else if (0 == strcmp(argv[i], "-tag")) {
            ++i;
            while ((i < argc) && ('-' != argv[i][0])) {
                if (tags_size + 1U == tags_max_size) {
                    char** temp_tags = (char**) malloc(sizeof(char**) * (tags_max_size * 2));
                    for (unsigned char temp_index = 0U; temp_index < tags_max_size; ++temp_index) {
                        temp_tags[temp_index] = tags[temp_index];
                    }
                    tags = temp_tags;
                    tags_max_size <<= 1;
                }
                tags[tags_size] = argv[i];
                ++tags_size;
                ++i;
            }
        }
    }

    Movie_t* result_movies;
    const unsigned long long movies_found_count = movie_search(movies, movies_count, title_keywords, title_keyword_size, years[0], genres, genres_size, tags, tags_size, &result_movies);

    for (unsigned long long movie_index = 0U; movie_index < movies_found_count; ++movie_index) {
        printf("%llu::%s::%s\n", result_movies[movie_index].id, result_movies[movie_index].title, result_movies[movie_index].genres);
    }

    free(result_movies);
    free(movies);
    return 0;
}

static unsigned long long movie_search(Movie_t* movies, unsigned long long movies_count, char** titles, unsigned char titles_count, unsigned short year, char** genres, unsigned char genres_count, char** tags, unsigned char tags_count, Movie_t** out_movies) {
    Movie_t* valid_movies = (Movie_t*) malloc(sizeof(Movie_t) * 4); // We will start will 4
    unsigned long long max_size = 4;
    unsigned long long found_movies_count = 0U;
    for (unsigned long long index = 0U; index < movies_count; ++index) {
        // Filter Year
        // TODO: Fix year parsing first
        // if ((0 != year) && (year != movies[index].year)) { // No movie released with Jesus
        //     continue;
        // }

        // Filter Title
        BOOL has_different = FALSE;
        for (unsigned char title_index = 0U; title_index < titles_count; ++title_index) {
            if (NULL == strstr(movies[index].title, titles[title_index])) {
                has_different = TRUE;
                break;
            }
        }
        if (TRUE == has_different) {
            continue;
        }
#if 0 // TODO: Fix genre in movie struct
        // Filter genre
        has_different = FALSE;
        for (unsigned char genre_index = 0U; genre_index < genres_count; ++genre_index) {
            for (unsigned char movie_genre_index = 0U; movie_genre_index < movies[index].genres_count; ++movie_genre_index) {
                if (0 != strcmp(genres[genre_index], movies[index].genres[movie_genre_index])) {
                    has_different = TRUE;
                    break;
                }
            }
            if (TRUE == has_different) {
                break;
            }
        }

        if (TRUE == has_different) {
            continue;
        }
#endif

        // Filter Tags
        has_different = FALSE;
        for (unsigned char tags_index = 0U; tags_index < tags_count; ++tags_index) {
            BOOL found_tag = FALSE;
            for (unsigned short movie_tags_index = 0U; movie_tags_index < movies[index].tags_count; ++movie_tags_index) {
                if (NULL != strstr(movies[index].tags[movie_tags_index], tags[tags_index])) {
                    found_tag = TRUE;
                    break;
                }
            }
            if (FALSE == found_tag) {
                has_different = TRUE;
                break;
            }
        }

        if (FALSE != has_different) {
            continue;
        }

        if (found_movies_count + 1 == max_size) {
            Movie_t* temp_movies = malloc(sizeof(Movie_t) * (max_size * 2));
            for (unsigned long long temp_index = 0U; temp_index < max_size; ++temp_index) {
                temp_movies[temp_index] = valid_movies[temp_index];
            }
            // free(valid_movies); // TODO: should this happen (prob not)
            valid_movies = temp_movies;
            max_size <<= 1;
        }
        valid_movies[found_movies_count] = movies[index];
        ++found_movies_count;
    }
    *out_movies = valid_movies;
    return found_movies_count;
}