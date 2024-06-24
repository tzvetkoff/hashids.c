#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <getopt.h>

#include "hashids.h"

#ifndef lengthof
#define lengthof(x) ((size_t)(sizeof(x) / sizeof(x[0])))
#endif

struct testcase_t {
    const char *salt;
    size_t min_hash_length;
    const char *alphabet;
    const char *separators;
    size_t numbers_count;
    unsigned long long numbers[16];
    const char *expected_hash;
    int line;
};

struct testcase_t testcases[] = {
    {"", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        1, {1ull}, "jR", __LINE__},
    {"", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        1, {12345ull}, "j0gW", __LINE__},
    {"", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        1, {22ull}, "Lw", __LINE__},
    {"", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        1, {333ull}, "Z0E", __LINE__},
    {"", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        1, {9999ull}, "w0rR", __LINE__},
    {"", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        1, {18446744073709551615ull}, "AOo9Ql5nQR1VO", __LINE__},
    {"", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        4, {683ull, 94108ull, 123ull, 5ull}, "vJvi7On9cXGtD", __LINE__},
    {"", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        3, {1ull, 2ull, 3ull}, "o2fXhV", __LINE__},
    {"", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        3, {2ull, 4ull, 6ull}, "xGhmsW", __LINE__},
    {"", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        2, {99ull, 25ull}, "3lKfD", __LINE__},

    {"", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        1, {1ull}, "tP", __LINE__},
    {"", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        1, {12345ull}, "wv2s", __LINE__},
    {"", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        1, {22ull}, "QR", __LINE__},
    {"", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        1, {333ull}, "2WD", __LINE__},
    {"", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        1, {9999ull}, "JOs6", __LINE__},
    {"", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        1, {18446744073709551615ull}, "JNXLUzBMS8SXR", __LINE__},
    {"", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        4, {683ull, 94108ull, 123ull, 5ull}, "MH0ozwS3kMUjN", __LINE__},
    {"", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        3, {1ull, 2ull, 3ull}, "A3eGiY", __LINE__},
    {"", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        3, {2ull, 4ull, 6ull}, "GLi1ut", __LINE__},
    {"", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        2, {99ull, 25ull}, "9HYh2", __LINE__},

    {"Arbitrary string", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        4, {683ull, 94108ull, 123ull, 5ull}, "QWyf8yboH7KT2", __LINE__},
    {"Arbitrary string", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        3, {1ull, 2ull, 3ull}, "neHrCa", __LINE__},
    {"Arbitrary string", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        3, {2ull, 4ull, 6ull}, "LRCgf2", __LINE__},
    {"Arbitrary string", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        2, {99ull, 25ull}, "JOMh1", __LINE__},

    {"Arbitrary string", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        4, {683ull, 94108ull, 123ull, 5ull}, "1r0o7IOMjZtmI", __LINE__},
    {"Arbitrary string", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        3, {1ull, 2ull, 3ull}, "CYoMy0", __LINE__},
    {"Arbitrary string", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        3, {2ull, 4ull, 6ull}, "FUyniq", __LINE__},
    {"Arbitrary string", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        2, {99ull, 25ull}, "UNZa1", __LINE__},

    {"", 0, "!\"#%&',-/0123456789:;<=>ABCDEFGHIJKLMNOPQRSTUVWXYZ_`"
            "abcdefghijklmnopqrstuvwxyz~",
        HASHIDS_DEFAULT_SEPARATORS,
        4, {2839ull, 12ull, 32ull, 5ull}, "_nJUNTVU3", __LINE__},
    {"", 0, "!\"#%&',-/0123456789:;<=>ABCDEFGHIJKLMNOPQRSTUVWXYZ_`"
            "abcdefghijklmnopqrstuvwxyz~",
        HASHIDS_DEFAULT_SEPARATORS,
        3, {1ull, 2ull, 3ull}, "7xfYh2", __LINE__},
    {"", 0, "!\"#%&',-/0123456789:;<=>ABCDEFGHIJKLMNOPQRSTUVWXYZ_`"
            "abcdefghijklmnopqrstuvwxyz~",
        HASHIDS_DEFAULT_SEPARATORS,
        1, {23832ull}, "Z6R>", __LINE__},
    {"", 0, "!\"#%&',-/0123456789:;<=>ABCDEFGHIJKLMNOPQRSTUVWXYZ_`"
            "abcdefghijklmnopqrstuvwxyz~",
        HASHIDS_DEFAULT_SEPARATORS,
        2, {99ull, 25ull}, "AYyIB", __LINE__},

    {"", 0, "!\"#%&',-/0123456789:;<=>ABCDEFGHIJKLMNOPQRSTUVWXYZ_`"
            "abcdefghijklmnopqrstuvwxyz~",
        "aeiouy",
        4, {2839ull, 12ull, 32ull, 5ull}, "fKt<e`,r'g", __LINE__},
    {"", 0, "!\"#%&',-/0123456789:;<=>ABCDEFGHIJKLMNOPQRSTUVWXYZ_`"
            "abcdefghijklmnopqrstuvwxyz~",
        "aeiouy",
        3, {1ull, 2ull, 3ull}, "C;eNij", __LINE__},
    {"", 0, "!\"#%&',-/0123456789:;<=>ABCDEFGHIJKLMNOPQRSTUVWXYZ_`"
            "abcdefghijklmnopqrstuvwxyz~",
        "aeiouy",
        1, {23832ull}, "b_=n", __LINE__},
    {"", 0, "!\"#%&',-/0123456789:;<=>ABCDEFGHIJKLMNOPQRSTUVWXYZ_`"
            "abcdefghijklmnopqrstuvwxyz~",
        "aeiouy",
        2, {99ull, 25ull}, "O;O4C", __LINE__},

    {"", 25, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        3, {7452ull, 2967ull, 21401ull}, "pO3K69b86jzc6krI416enr2B5", __LINE__},
    {"", 25, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        3, {1ull, 2ull, 3ull}, "gyOwl4B97bo2fXhVaDR0Znjrq", __LINE__},
    {"", 25, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        1, {6097ull}, "Nz7x3VXyMYerRmWeOBQn6LlRG", __LINE__},
    {"", 25, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        2, {99ull, 25ull}, "k91nqP3RBe3lKfDaLJrvy8XjV", __LINE__},

    {"", 25, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        3, {7452ull, 2967ull, 21401ull}, "R7Vs1Brz1PTuPSRoAX8qQN8MI", __LINE__},
    {"", 25, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        3, {1ull, 2ull, 3ull}, "xOSZPDHYXrA3eGiYpI3sC7JU0", __LINE__},
    {"", 25, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        1, {6097ull}, "JXOtT5IWQspH5FIqDLUH7GxBS", __LINE__},
    {"", 25, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        2, {99ull, 25ull}, "SAW5ILZwYp9HYh2nKvRtxME2U", __LINE__},

    {"arbitrary salt", 16, "abcdefghijklmnopqrstuvwxyz",
        HASHIDS_DEFAULT_SEPARATORS,
        3, {7452ull, 2967ull, 21401ull}, "wygqxeunkatjgkrw", __LINE__},
    {"arbitrary salt", 16, "abcdefghijklmnopqrstuvwxyz",
        HASHIDS_DEFAULT_SEPARATORS,
        3, {1ull, 2ull, 3ull}, "pnovxlaxuriowydb", __LINE__},
    {"arbitrary salt", 16, "abcdefghijklmnopqrstuvwxyz",
        HASHIDS_DEFAULT_SEPARATORS,
        1, {60125ull}, "jkbgxljrjxmlaonp", __LINE__},
    {"arbitrary salt", 16, "abcdefghijklmnopqrstuvwxyz",
        HASHIDS_DEFAULT_SEPARATORS,
        2, {99ull, 25ull}, "erdjpwrgouoxlvbx", __LINE__},

    {"arbitrary salt", 16, "abcdefghijklmnopqrstuvwxyz",
        "aeiouy",
        3, {7452ull, 2967ull, 21401ull}, "sqdxzzidlvarjhbp", __LINE__},
    {"arbitrary salt", 16, "abcdefghijklmnopqrstuvwxyz",
        "aeiouy",
        3, {1ull, 2ull, 3ull}, "jfbrsbjygalshcxg", __LINE__},
    {"arbitrary salt", 16, "abcdefghijklmnopqrstuvwxyz",
        "aeiouy",
        1, {60125ull}, "xqmzjpjqlqlslfnv", __LINE__},
    {"arbitrary salt", 16, "abcdefghijklmnopqrstuvwxyz",
        "aeiouy",
        2, {99ull, 25ull}, "rblfsdwqelmpgcqk", __LINE__},

    {"", 0, "abdegjklmnopqrvwxyzABDEGJKLMNOPQRVWXYZ1234567890",
        HASHIDS_DEFAULT_SEPARATORS,
        3, {7452ull, 2967ull, 21401ull}, "X50Yg6VPoAO4", __LINE__},
    {"", 0, "abdegjklmnopqrvwxyzABDEGJKLMNOPQRVWXYZ1234567890",
        HASHIDS_DEFAULT_SEPARATORS,
        3, {1ull, 2ull, 3ull}, "GAbDdR", __LINE__},
    {"", 0, "abdegjklmnopqrvwxyzABDEGJKLMNOPQRVWXYZ1234567890",
        HASHIDS_DEFAULT_SEPARATORS,
        1, {60125ull}, "5NMPD", __LINE__},
    {"", 0, "abdegjklmnopqrvwxyzABDEGJKLMNOPQRVWXYZ1234567890",
        HASHIDS_DEFAULT_SEPARATORS,
        2, {99ull, 25ull}, "yGya5", __LINE__},

    {"", 0, "abdegjklmnopqrvwxyzABDEGJKLMNOPQRVWXYZ1234567890",
        "aeiouy",
        3, {7452ull, 2967ull, 21401ull}, "OLQ8b7W3yROE", __LINE__},
    {"", 0, "abdegjklmnopqrvwxyzABDEGJKLMNOPQRVWXYZ1234567890",
        "aeiouy",
        3, {1ull, 2ull, 3ull}, "E5ewoA", __LINE__},
    {"", 0, "abdegjklmnopqrvwxyzABDEGJKLMNOPQRVWXYZ1234567890",
        "aeiouy",
        1, {60125ull}, "4XV6G", __LINE__},
    {"", 0, "abdegjklmnopqrvwxyzABDEGJKLMNOPQRVWXYZ1234567890",
        "aeiouy",
        2, {99ull, 25ull}, "7XXn9", __LINE__},

    {"", 0, "abdegjklmnopqrvwxyzABDEGJKLMNOPQRVWXYZ1234567890uC",
        HASHIDS_DEFAULT_SEPARATORS,
        3, {7452ull, 2967ull, 21401ull}, "GJNNmKYzbPBw", __LINE__},
    {"", 0, "abdegjklmnopqrvwxyzABDEGJKLMNOPQRVWXYZ1234567890uC",
        HASHIDS_DEFAULT_SEPARATORS,
        3, {1ull, 2ull, 3ull}, "DQCXa4", __LINE__},
    {"", 0, "abdegjklmnopqrvwxyzABDEGJKLMNOPQRVWXYZ1234567890uC",
        HASHIDS_DEFAULT_SEPARATORS,
        1, {60125ull}, "38V1D", __LINE__},
    {"", 0, "abdegjklmnopqrvwxyzABDEGJKLMNOPQRVWXYZ1234567890uC",
        HASHIDS_DEFAULT_SEPARATORS,
        2, {99ull, 25ull}, "373az", __LINE__},

    {"", 0, "abdegjklmnopqrvwxyzABDEGJKLMNOPQRVWXYZ1234567890uC",
        "aeiouy",
        3, {7452ull, 2967ull, 21401ull}, "JGRRuwERkN3A", __LINE__},
    {"", 0, "abdegjklmnopqrvwxyzABDEGJKLMNOPQRVWXYZ1234567890uC",
        "aeiouy",
        3, {1ull, 2ull, 3ull}, "EYeMoB", __LINE__},
    {"", 0, "abdegjklmnopqrvwxyzABDEGJKLMNOPQRVWXYZ1234567890uC",
        "aeiouy",
        1, {60125ull}, "4VWzJ", __LINE__},
    {"", 0, "abdegjklmnopqrvwxyzABDEGJKLMNOPQRVWXYZ1234567890uC",
        "aeiouy",
        2, {99ull, 25ull}, "4XDmN", __LINE__},

    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        1, {1ull}, "NV", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        1, {22ull}, "K4", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        1, {333ull}, "OqM", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        1, {9999ull}, "kQVg", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        1, {12345ull}, "NkK9", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        1, {123000ull}, "58LzD", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        1, {456000000ull}, "5gn6mQP", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        1, {987654321ull}, "oyjYvry", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        1, {666555444333222ull}, "KVO9yy1oO5j", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        1, {18446744073709551615ull}, "zXVjmzBamYlqX", __LINE__},

    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        1, {1ull}, "Vt", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        1, {22ull}, "Sp", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        1, {333ull}, "PYA", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        1, {9999ull}, "xC2s", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        1, {12345ull}, "ZwA1", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        1, {123000ull}, "2FYKW", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        1, {456000000ull}, "2Vw2HOn", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        1, {987654321ull}, "RHpJGYQ", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        1, {666555444333222ull}, "SYvVJ8tGtWH", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        1, {18446744073709551615ull}, "xQNnVWzEB0BNU", __LINE__},

    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        5, {1ull, 2ull, 3ull, 4ull, 5ull}, "zoHWuNhktp", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        3, {1ull,2ull,3ull}, "laHquq", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        3, {2ull,4ull,6ull}, "44uotN", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        2, {99ull,25ull}, "97Jun", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        3, {1337ull,42ull,314ull}, "7xKhrUxm", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        4, {683ull,94108ull,123ull,5ull}, "aBMswoO2UB3Sj", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        7, {547ull,31ull,241271ull,311ull,31397ull,1129ull,71129ull},
        "3RoSDhelEyhxRsyWpCx5t1ZK", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        5, {21979508ull,35563591ull,57543099ull,93106690ull,150649789ull},
        "p2xkL3CK33JjcrrZ8vsw4YRZueZX9k", __LINE__},

    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        5, {1ull, 2ull, 3ull, 4ull, 5ull}, "xIyKoAaZeC", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        3, {1ull,2ull,3ull}, "FWyGoN", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        3, {2ull,4ull,6ull}, "E5o3ep", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        2, {99ull,25ull}, "HEBl3", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        3, {1337ull,42ull,314ull}, "ZJQiZ0gGv", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        4, {683ull,94108ull,123ull,5ull}, "IPOh3zLNdrFcz", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        7, {547ull,31ull,241271ull,311ull,31397ull,1129ull,71129ull},
        "UONfBmVZrQmpZy3EBmPFdVSq", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        5, {21979508ull,35563591ull,57543099ull,93106690ull,150649789ull},
        "XXwqUZj1pQ2xfNZ0LLhPRPDHkwVPZHO", __LINE__},

    {"this is my salt", 18, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        1, {1}, "aJEDngB0NV05ev1WwP", __LINE__},
    {"this is my salt", 18, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        6, {4140ull,21147ull,115975ull,678570ull,4213597ull,27644437ull},
        "pLMlCWnJSXr1BSpKgqUwbJ7oimr7l6", __LINE__},

    {"this is my salt", 18, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        1, {1}, "xAQrG2p6VtMHCVEZNU", __LINE__},
    {"this is my salt", 18, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        6, {4140ull,21147ull,115975ull,678570ull,4213597ull,27644437ull},
        "PxO0jEtJc4B1NiIGz2gBPC42yCwOsw", __LINE__},

    {"this is my salt", 0, "ABCDEFGhijklmn34567890-",
        HASHIDS_DEFAULT_SEPARATORS,
        5, {1ull,2ull,3ull,4ull,5ull}, "D4h3F7i5Al", __LINE__},

    {"this is my salt", 0, "ABCDEFGhijklmn34567890-",
        "aeiouy",
        5, {1ull,2ull,3ull,4ull,5ull}, "73AkB0CkD6", __LINE__},

    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        4, {5ull,5ull,5ull,5ull}, "1Wc8cwcE", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        16, {1ull,2ull,3ull,4ull,5ull,6ull,7ull,8ull,
            9ull,10ull,11ull,12ull,13ull,14ull,15ull,16ull},
        "2bHPuPh5tycXIRTLCos9FaiXfoSEUQHB", __LINE__},

    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        4, {5ull,5ull,5ull,5ull}, "Y1iti2iB", __LINE__},
    {"this is my salt", 0, HASHIDS_DEFAULT_ALPHABET,
        "aeiouy",
        16, {1ull,2ull,3ull,4ull,5ull,6ull,7ull,8ull,
            9ull,10ull,11ull,12ull,13ull,14ull,15ull,16ull},
        "tAy5onaFepi4bWcwdUfHgRhXjxkKlHmW", __LINE__},

    {"this is my salt", 0, "cfhistuCFHISTU+-",
        HASHIDS_DEFAULT_SEPARATORS,
        1, {1337ull}, "+-+-++---++-", __LINE__},

    {"this is my salt", 0, "cfhistuCFHISTU+-",
        "aeiouy",
        1, {1337ull}, "TsCCU", __LINE__},

    {"this is my salt", 0, "abdegjkomnopqrvwxyzABDEGJKLMNOPQRVWXYZ1234567890"
        "!@#$%^&*()-_=+;:,.<>/?`~[]{}",
        HASHIDS_DEFAULT_SEPARATORS,
        1, {1ull}, "V7", __LINE__},

    {"this is my salt", 0, "abdegjkomnopqrvwxyzABDEGJKLMNOPQRVWXYZ1234567890"
        "!@#$%^&*()-_=+;:,.<>/?`~[]{}",
        "aeiouy",
        1, {1ull}, "6J", __LINE__},

    {"", 22, "abdegjk0123456789",
        HASHIDS_DEFAULT_SEPARATORS,
        1, {190126ull}, "070683k2j3771430j49157", __LINE__},
    {"\\7ULC'", 22, "@'l*p9n]);+7>Ar(\\",
        HASHIDS_DEFAULT_SEPARATORS,
        1, {190126ull}, "9];r(An97\\]]\\()>7>\\)+]", __LINE__},

    {"", 22, "abdegjk0123456789",
        "aeiouy",
        1, {190126ull}, "070683k2j3771430j49157", __LINE__},
    {"\\7ULC'", 22, "@'l*p9n]);+7>Ar(\\",
        "aeiouy",
        1, {190126ull}, "9];r(An97\\]]\\()>7>\\)+]", __LINE__},

    {"", 1337, HASHIDS_DEFAULT_ALPHABET,
        HASHIDS_DEFAULT_SEPARATORS,
        1, {1337ull},
        "lzALO5xqNEXWL8DOJ0wPk21RmoyQKMYlZ38Gzr95O6jyokL7BVXx8J2oQ17YvqAw"
        "56KLpMNm8PKwXlkojOA01ZBVy6YgpDvRV2E3OxnPgA6orY4ZG1QzBDZVA1mLP5Ng"
        "YG3Q2WqKO6rmDgB5x3lZkMLQ2EVpjwnRJlN6vyOEnxpZRrXKMAoG52YOl8g4D6KX"
        "7x9ARmyzv1krGAzNWwQyPB89Lk2gvjnqoOB6Vn1oGqzYkl7O0p23KQmEpZxMGNPy"
        "g1W5L7Bz3vrqwYq8gPWR4Xk0wyKMmL6AvJ7Q1oD2EZVpPYM4WRN0GgK9XlNJvAzX"
        "kw079qLQZ8WxnrRKVpL5ND0l91vMgE7JAZOxP6ERPGw4xDqMklnymZ9VKQp331QE"
        "RDYzALx5PlwgKk8p4r9qL2KQ86gJ7RDBnxwMr3ZGmQVMGwAvEK6N7X5yDP4pJnP4"
        "o3RKxDXZyBNvrJmjWqLwWm1VER9X5B78G6ZNkpL4O2EknRx4qMK9NZ7j53WyOzQg"
        "qrW1YMB5AnkRpO37DV2jG6YLmBQ6J2E70wnAPDZXOplxj349lD0YZLngkOG2Ro7z"
        "yJ6PqJ30oKGn9OgNzRyW25m77pnOg298EvljGD1YWRVrk32yp8z6nWqx0LXVE9jP"
        "K5DQEvX37WpmMkqno2j50r4KGRb4Z2eBLZQxgy91DlPzONVwA68JYYRkM7o3OJwB"
        "ZNlAm41rgGv6XNzwJy0AB4Mq5LoxZPQKmMjEAkYVZXDw8rxlQpv41LB5pQqvNBmK"
        "w1rMAXV8EP6xWy1RkKzVMjrG3g8vN9oW45qmwvgZxoLXyPJQKNzE0498lA2wX8Vr"
        "0mpvP6LJ1GoBYlDYzDnyMwP0rQj3KgvlJAxoq2Mg7Q9Op6G8AY5nV01Elkz8Bolq"
        "2g0YOW1rxj9zR3kLZj45Wmo1VAE0XvkzPyYNOpl6M0XyOVGjN7nBoq2Zvm9WJ50j"
        "vXBOzWo2Lg1JNr7AY68ryk8o4X2jQW3GKqzYnwBmRBj4lpDYyVm1MP5gGo2EO63v"
        "6OqJy8nzAwLBxmjk7r35Q35zolOjx9VnrpBG2YNED1ZXl6KE2RAnoDmj9O0V4JQ8"
        "kRM9gNX48v5LjyrZwJWPxDA4KV3pZX0JRD5Ex17lYrm6MWE2jJPpw3qBnGZLQ5NV"
        "M0ogDkzWj0Y84QPmB3L7qw1V9Yo49PW6XAqy807z1NJOKvG4vp780lEoRryjXM9x"
        "wzkJnB79DWXLlNp0jk8mJMK5qyw4QMLGNrJRvWx792Ezqm35nVEyPz9RGl0jrgW3"
        "BnOZkD4pXRxnmvN0QWP21DEwgq4JABlZ59MzV74nKGAjp6rgvY3p8ywGk", __LINE__},


    {NULL, 0, NULL, NULL, 0, {0ull}, NULL, 0}
};

char *failures[lengthof(testcases)];

char *
f(const char *fmt, ...)
{
    char *result = calloc(512, 1);
    va_list ap;

    if (!result) {
        fputs("Fatal error: Cannot allocate memory for error description\n",
            stdout);
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
    hashids_t *hashids = NULL;
    size_t i = 0, j = 1, k = 0, result = 0;
    char *buffer = NULL;
    unsigned long long numbers[16];
    struct testcase_t testcase = {NULL, 0, NULL, NULL, 0, {0}, NULL, 0};
    int fail = 0, fail_fast = 0, ch = 0;

    static const struct option longopts[] = {
        {"fail-fast", no_argument, NULL, 'f'},
        {NULL, 0, NULL, 0}
    };

    /* parse command line options */
    while ((ch = getopt_long(argc, argv, "+f", longopts, NULL)) != -1) {
        switch (ch) {
            case 'f':
                fail_fast = 1;
                break;
            default:
                /* do nothing (and avoid warnings) */
                (void)NULL;
        }
    }

    /* walk test cases */
    for (;; ++i, ++j) {
        fail = 0;

        if (i && i % 72 == 0) {
            fputs("\n", stdout);
        }

        testcase = testcases[i];

        /* bail out */
        if (!testcase.salt || !testcase.alphabet || !testcase.expected_hash) {
            break;
        }

        /* initialize hashids */
        hashids = hashids_init4(testcase.salt, testcase.min_hash_length,
            testcase.alphabet, testcase.separators);

        /* error upon initialization */
        if (!hashids) {
            fail = 1;

            switch (hashids_errno) {
                case HASHIDS_ERROR_ALLOC:
                    failures[k++] = f("%s:%d: hashids_init3(): "
                        "memory allocation failed",
                        __FILE__, testcase.line);
                    break;
                case HASHIDS_ERROR_ALPHABET_LENGTH:
                    failures[k++] = f("%s:%d: hashids_init3(): "
                        "alphabet too short",
                        __FILE__, testcase.line);
                    break;
                case HASHIDS_ERROR_ALPHABET_SPACE:
                    failures[k++] = f("%s:%d: hashids_init3(): "
                        "alphabet contains whitespace character",
                        __FILE__, testcase.line);
                    break;
                default:
                    failures[k++] = f("%s:%d: hashids_init3(): "
                        "unknown error",
                        __FILE__, testcase.line);
                    break;
            }

            goto test_end;
        }

        /* allocate buffer */
        size_t
            estimated_encoded_size = hashids_estimate_encoded_size(hashids,
                testcase.numbers_count, testcase.numbers),
            expected_encoded_size = strlen(testcase.expected_hash) + 1;
        if (estimated_encoded_size < expected_encoded_size) {
            fail = 1;
            failures[k++] = f("%s:%d: hashids_estimate_encoded_size() "
                "returned %u, expected >=%u",
                __FILE__, testcase.line,
                estimated_encoded_size, expected_encoded_size);
            goto test_end;
        }

        buffer = calloc(estimated_encoded_size, 1);
        if (!buffer) {
            fail = 1;
            failures[k++] = f("%s:%d: cannot allocate buffer",
                __FILE__, testcase.line);
            goto test_end;
        }

        /* encode */
        result = hashids_encode(hashids, buffer, testcase.numbers_count,
            testcase.numbers);

        /* encoding error */
        if (result < testcase.min_hash_length) {
            fail = 1;
            failures[k++] = f("%s:%d: hashids_encode() "
                "returned %u, expected >=%u",
                __FILE__, testcase.line,
                result, testcase.min_hash_length);
            goto test_end;
        }

        /* compare encoded string */
        if (strcmp(buffer, testcase.expected_hash) != 0) {
            fail = 1;
            failures[k++] = f("%s:%d: hashids_encode() "
                "returned \"%s\", expected \"%s\"",
                __FILE__, testcase.line,
                buffer, testcase.expected_hash);
            goto test_end;
        }

        /* decode */
        result = hashids_decode(hashids, buffer, numbers, 16);

        /* decoding error */
        if (result != testcase.numbers_count) {
            fail = 1;
            failures[k++] = f("%s:%d: hashids_decode() "
                "returned %u, expected %u",
                __FILE__, testcase.line,
                result, testcase.numbers_count);
            goto test_end;
        }

        /* compare decoded numbers */
        if (memcmp(numbers, testcase.numbers,
                result * sizeof(unsigned long long))) {
            fail = 1;
            failures[k++] = f("%s:%d: hashids_decode() decoding error",
                __FILE__, testcase.line);
            goto test_end;  /* nop? */
        }

test_end:
        fputc(fail ? 'F' : '.', stdout);

        if (hashids) {
            hashids_free(hashids);
            hashids = NULL;
        }
        if (buffer) {
            free(buffer);
            buffer = NULL;
        }

        if (fail && fail_fast) {
            fputs("\n\n", stdout);
            printf("test            : %s:%d\n", __FILE__, testcase.line);
            printf("salt            : \"%s\"\n", testcase.salt);
            printf("min_hash_length : %lu\n", testcase.min_hash_length);
            printf("alphabet        : \"%s\"\n", testcase.alphabet);
            fputs( "numbers         : ", stdout);
            for (i = 0; i < testcase.numbers_count; ++i) {
                printf("%llu", testcase.numbers[i]);
                if (i < testcase.numbers_count - 1) {
                    fputs(" ", stdout);
                }
            }
            fputs("\n", stdout);
            printf("expected_hash   : %s", testcase.expected_hash);
            break;
        }
    }

    if (failures[0]) {
        fputs("\n\n", stdout);
        for (i = 0; failures[i]; ++i) {
            printf("%s\n", failures[i]);
            free(failures[i]);
        }
    }

    printf("\n%lu samples, %lu failures\n", j, k);

    return k ? EXIT_FAILURE : EXIT_SUCCESS;
}
