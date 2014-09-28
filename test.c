#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "hashids.h"

#ifndef lengthof
#define lengthof(x) ((unsigned int)(sizeof(x) / sizeof(x[0])))
#endif

struct testcase_t {
    const char *salt;
    unsigned int min_hash_length;
    const char *alphabet;
    unsigned int numbers_count;
    unsigned long long numbers[16];
    const char *expected_hash;
};

struct testcase_t testcases[] = {
    {"", 0, HASHIDS_DEFAULT_ALPHABET, 1, {1ull}, "jR"},
    {"", 0, HASHIDS_DEFAULT_ALPHABET, 1, {12345ull}, "j0gW"},
    {"", 0, HASHIDS_DEFAULT_ALPHABET, 1, {18446744073709551615ull}, "AOo9Ql5nQR1VO"},

    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET, 1, {1ull}, "NV"},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET, 1, {22ull}, "K4"},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET, 1, {333ull}, "OqM"},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET, 1, {9999ull}, "kQVg"},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET, 1, {9999ull}, "kQVg"},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET, 1, {12345ull}, "NkK9"},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET, 1, {123000ull}, "58LzD"},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET, 1, {456000000ull}, "5gn6mQP"},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET, 1, {987654321ull}, "oyjYvry"},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET, 1, {666555444333222ull}, "KVO9yy1oO5j"},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET, 1, {18446744073709551615ull}, "zXVjmzBamYlqX"},

    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET, 5, {1ull, 2ull, 3ull, 4ull, 5ull}, "zoHWuNhktp"},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET, 3, {1ull,2ull,3ull}, "laHquq"},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET, 3, {2ull,4ull,6ull}, "44uotN"},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET, 2, {99ull,25ull}, "97Jun"},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET, 3, {1337ull,42ull,314ull}, "7xKhrUxm"},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET, 4, {683ull,94108ull,123ull,5ull}, "aBMswoO2UB3Sj"},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET, 7, {547ull,31ull,241271ull,311ull,31397ull,1129ull,71129ull}, "3RoSDhelEyhxRsyWpCx5t1ZK"},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET, 5, {21979508ull,35563591ull,57543099ull,93106690ull,150649789ull}, "p2xkL3CK33JjcrrZ8vsw4YRZueZX9k"},

    {"this is my salt", 18, HASHIDS_DEFAULT_ALPHABET, 1, {1}, "aJEDngB0NV05ev1WwP"},
    {"this is my salt", 18, HASHIDS_DEFAULT_ALPHABET, 6, {4140ull,21147ull,115975ull,678570ull,4213597ull,27644437ull}, "pLMlCWnJSXr1BSpKgqUwbJ7oimr7l6"},

    {"this is my salt", 0, "ABCDEFGhijklmn34567890-", 5, {1ull,2ull,3ull,4ull,5ull}, "6nhmFDikA0"},

    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET, 4, {5ull,5ull,5ull,5ull}, "1Wc8cwcE"},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET, 10, {1ull,2ull,3ull,4ull,5ull,6ull,7ull,8ull,9ull,10ull}, "kRHnurhptKcjIDTWC3sx"},

    {"this is my salt", 0, "cfhistuCFHISTU+-", 1, {1337ull}, "+-+-++---++-"},

    {NULL, 0, NULL, 0, {0ull}, NULL}
};

char *failures[lengthof(testcases)];

char *
f(const char *fmt, ...)
{
    char *result = calloc(512, 1);
    va_list ap;

    if (!result) {
        printf("Fatal error: Cannot allocate memory for error description\n");
        exit(EXIT_FAILURE);
    }

    va_start(ap, fmt);
    vsprintf(result, fmt, ap);
    va_end(ap);

    return result;
}

int
main(int argc, char **argv)
{
    struct hashids_t *hashids;
    char buffer[256], *error = 0;
    unsigned int i, j, result;
    unsigned long long numbers[16];
    struct testcase_t testcase;

    /* walk test cases */
    for (i = 0, j = 0;; ++i) {
        if (i && i % 72 == 0) {
            printf("\n");
        }

        testcase = testcases[i];

        /* bail out */
        if (!testcase.salt || !testcase.alphabet || !testcase.expected_hash) {
            break;
        }

        /* initialize hashids */
        hashids = hashids_init3(testcase.salt, testcase.min_hash_length,
            testcase.alphabet);

        /* error upon initialization */
        if (!hashids) {
            printf("F");

            switch (hashids_errno) {
                case HASHIDS_ERROR_ALLOC:
                    error = "Hashids: Allocation failed";
                    break;
                case HASHIDS_ERROR_ALPHABET_LENGTH:
                    error = "Hashids: Alphabet is too short";
                    break;
                case HASHIDS_ERROR_ALPHABET_SPACE:
                    error = "Hashids: Alphabet contains whitespace characters";
                    break;
                default:
                    error = "Hashids: Unknown error";
                    break;
            }

            failures[j++] = f("#%d: %s", i + 1, error);
            continue;
        }

        /* encode */
        result = hashids_encode(hashids, buffer, testcase.numbers_count,
            testcase.numbers);

        /* encoding error */
        if (!result) {
            printf("F");
            failures[j++] = f("#%d: hashids_encode() returned 0", i + 1);
            hashids_free(hashids);
            continue;
        }

        /* decode */
        result = hashids_decode(hashids, buffer, numbers);

        /* decoding error */
        if (result != testcase.numbers_count) {
            printf("F");
            failures[j++] = f("#%d: hashids_decode() returned %u", i + 1, result);
            hashids_free(hashids);
            continue;
        }

        /* compare */
        if (memcmp(numbers, testcase.numbers,
                result * sizeof(unsigned long long))) {
            printf("F");
            failures[j++] = f("#%d: hashids_decode() decoding error", i + 1);
            hashids_free(hashids);
            continue;
        }

        printf(".");
        hashids_free(hashids);
    }

    printf("\n\n");

    for (i = 0; i < j; ++i) {
        printf("%s\n", failures[i]);
    }

    if (j) {
        printf("\n");
    }

    printf("%u samples, %u failures\n", lengthof(testcases) - 1, j);

    return j ? EXIT_FAILURE : EXIT_SUCCESS;
}
