#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "hashids.h"

enum { COMMAND_ENCODE = 0, COMMAND_DECODE = 1 };

static void
usage(const char *program_invocation_name, FILE *out)
{
    fputs("\n", out);
    fputs("Usage:\n", out);
    fprintf(out, "  %s [options] <arguments>\n",
        program_invocation_name);

    fputs("\n", out);
    fputs("Options:\n", out);
    fputs("  -e, --encode      set command to encode (default)\n", out);
    fputs("  -d, --decode      set command to decode\n", out);
    fputs("  -s, --salt        set salt [" HASHIDS_DEFAULT_SALT "]\n", out);
    fputs("  -a, --alphabet    set alphabet [" HASHIDS_DEFAULT_ALPHABET "]\n",
        out);
    fprintf(out, "  -l, --min-length  set hash minimum length [%u]\n",
        HASHIDS_DEFAULT_MIN_HASH_LENGTH);
    fputs("  -x, --hex         encode / decode hex strings\n", out);
    fputs("  -h, --help        display this help and exit\n", out);
    fputs("  -v, --version     print version information and exit\n", out);

    fputs("\n", out);
    exit(out == stderr ? EXIT_FAILURE : EXIT_SUCCESS);
}

static unsigned long long
parse_number(const char *s, char **p)
{
    int radix = 10;

    if (*s == '0') {
        radix = 8;
        ++s;

        if (*s == 'x' || *s == 'X') {
            radix = 16;
            ++s;
        }
    }

    return strtoull(s, p, radix);
}

int
main(int argc, char **argv)
{
    hashids_t *hashids;
    char *salt = HASHIDS_DEFAULT_SALT, *alphabet = HASHIDS_DEFAULT_ALPHABET,
        *buffer, *p, str[18];
    unsigned int command = COMMAND_ENCODE, hex = 0;
    size_t min_hash_length = HASHIDS_DEFAULT_MIN_HASH_LENGTH, numbers_count;
    unsigned long long number, *numbers, *numbers_ptr;
    int ch, i, j;

    static const struct option longopts[] = {
        {"encode", no_argument, NULL, 'e'},
        {"decode", no_argument, NULL, 'd'},
        {"salt", required_argument, NULL, 's'},
        {"alphabet", required_argument, NULL, 'a'},
        {"min-length", required_argument, NULL, 'l'},
        {"hex", no_argument, NULL, 'x'},
        {"help", no_argument, NULL, 'h'},
        {"version", no_argument, NULL, 'v'},
        {NULL, 0, NULL, 0}
    };

    /* parse command line options */
    while ((ch = getopt_long(argc, argv, "+eds:a:l:xhv", longopts, NULL)) != -1) {
        switch (ch) {
            case 'e':
                command = COMMAND_ENCODE;
                break;
            case 'd':
                command = COMMAND_DECODE;
                break;
            case 's':
                salt = optarg;
                break;
            case 'a':
                alphabet = optarg;
                break;
            case 'l':
                min_hash_length = strtoul(optarg, &p, 10);
                if (p == optarg) {
                    fprintf(stderr, "Invalid minimum length: %s\n", optarg);
                    return EXIT_FAILURE;
                }
                break;
            case 'x':
                hex = 1;
                break;
            case 'h':
                usage(argv[0], stdout);
                break;
            case 'v':
                puts(HASHIDS_VERSION);
                return EXIT_SUCCESS;
            default:
                usage(argv[0], stderr);
        }
    }

    /* no arguments? */
    if (optind == argc) {
        usage(argv[0], stderr);
    }

    /* initialize hashids */
    hashids = hashids_init3(salt, min_hash_length, alphabet);

    /* error checking */
    if (!hashids) {
        switch (hashids_errno) {
            case HASHIDS_ERROR_ALLOC:
                printf("Hashids: Allocation failed\n");
                break;
            case HASHIDS_ERROR_ALPHABET_LENGTH:
                printf("Hashids: Alphabet is too short\n");
                break;
            case HASHIDS_ERROR_ALPHABET_SPACE:
                printf("Hashids: Alphabet contains whitespace characters\n");
                break;
            default:
                printf("Hashids: Unknown error\n");
                break;
        }

        return EXIT_FAILURE;
    }

    /* encode */
    if (command == COMMAND_ENCODE) {
        /* hex mode */
        if (hex) {
            number = (unsigned long long)-1;
            buffer = calloc(hashids_estimate_encoded_size(hashids, 1, &number),
                1);

            if (!buffer) {
                printf("Cannot allocate memory for buffer\n");
                hashids_free(hashids);
                return EXIT_FAILURE;
            }

            for (i = optind; i < argc; ++i) {
                hashids_encode_hex(hashids, buffer, argv[i]);
                printf("%s\n", buffer);
            }

            free(buffer);
            hashids_free(hashids);
            return EXIT_SUCCESS;
        }

        /* collect numbers */
        numbers_count = argc - optind;
        numbers = calloc(numbers_count, sizeof(unsigned long long));
        numbers_ptr = numbers;

        if (!numbers) {
            printf("Cannot allocate memory for numbers\n");
            hashids_free(hashids);
            return EXIT_FAILURE;
        }

        for (i = optind; i < argc; ++i) {
            *numbers_ptr++ = parse_number(argv[i], &p);
            if (p == argv[i]) {
                printf("Invalid number: %s\n", argv[i]);
                free(numbers);
                hashids_free(hashids);
                return EXIT_FAILURE;
            }
        }

        /* allocate output buffer */
        buffer = calloc(hashids_estimate_encoded_size(hashids, numbers_count,
            numbers), 1);

        if (!buffer) {
            printf("Cannot allocate memory for buffer\n");
            free(numbers);
            hashids_free(hashids);
            return EXIT_FAILURE;
        }

        /* encode, print, cleanup */
        hashids_encode(hashids, buffer, numbers_count, numbers);
        printf("%s\n", buffer);

        free(buffer);
        free(numbers);
        hashids_free(hashids);
        return EXIT_SUCCESS;
    }

    /* decode */
    if (hex) {
        for (i = optind; i < argc; ++i) {
            hashids_decode_hex(hashids, argv[i], str);
            printf("%s\n", str);
        }

        hashids_free(hashids);
        return EXIT_SUCCESS;
    }

    for (i = optind; i < argc; ++i) {

        numbers_count = hashids_numbers_count(hashids, argv[i]);

        if (!numbers_count) {
            switch (hashids_errno) {
                case HASHIDS_ERROR_INVALID_HASH:
                    printf("Hashids: Invalid hash: %s\n", argv[i]);
                    break;
                default:
                    printf("Hashids: Unknown error\n");
                    break;
            }

            hashids_free(hashids);
            return EXIT_FAILURE;
        }

        numbers = calloc(numbers_count, sizeof(unsigned long long));

        if (!numbers) {
            printf("Cannot allocate memory for numbers\n");
            hashids_free(hashids);
            return EXIT_FAILURE;
        }

        hashids_decode(hashids, argv[i], numbers);

        for (j = 0; j < numbers_count; ++j) {
            printf("%llu", numbers[j]);
            if (j + 1 < numbers_count) {
                printf(" ");
            }
        }
        printf("\n");

        free(numbers);
    }

    return EXIT_SUCCESS;
}
