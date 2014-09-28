#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

#include "hashids.h"

/* exported hashids_errno */
int hashids_errno;

/* alloc/free */
static void *
hashids_alloc_f(size_t size)
{
    return calloc(size, 1);
}

static void
hashids_free_f(void *ptr)
{
    free(ptr);
}

void *(*_hashids_alloc)(size_t size) = hashids_alloc_f;
void (*_hashids_free)(void *ptr) = hashids_free_f;

/* shuffle */
void
hashids_shuffle(char *str, int str_length, char *salt, int salt_length)
{
    int i, j, v, p, temp;

    if (!salt_length) {
        return;
    }

    for (i = str_length - 1, v = 0, p = 0; i > 0; --i, ++v) {
        v %= salt_length;
        p += salt[v];
        j = (salt[v] + v + p) % i;

        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
    }
}

/* "destructor" */
void
hashids_free(struct hashids_t *hashids)
{
    if (hashids) {
        if (hashids->alphabet) {
            _hashids_free(hashids->alphabet);
        }
        if (hashids->alphabet_copy_1) {
            _hashids_free(hashids->alphabet_copy_1);
        }
        if (hashids->alphabet_copy_2) {
            _hashids_free(hashids->alphabet_copy_2);
        }
        if (hashids->salt) {
            _hashids_free(hashids->salt);
        }
        if (hashids->separators) {
            _hashids_free(hashids->separators);
        }
        if (hashids->guards) {
            _hashids_free(hashids->guards);
        }

        _hashids_free(hashids);
    }
}

/* common init */
struct hashids_t *
hashids_init3(const char *salt, unsigned int min_hash_length,
    const char *alphabet)
{
    struct hashids_t *result;
    int i, j;
    char ch, *p;

    hashids_errno = HASHIDS_ERROR_OK;

    /* allocate the structure */
    result = _hashids_alloc(sizeof(struct hashids_t));
    if (!result) {
        hashids_errno = HASHIDS_ERROR_ALLOC;
        return NULL;
    }

    /* allocate enough space for the alphabet and its copies */
    result->alphabet = _hashids_alloc(strlen(alphabet) + 1);
    result->alphabet_copy_1 = _hashids_alloc(strlen(alphabet) + 1);
    result->alphabet_copy_2 = _hashids_alloc(strlen(alphabet) + 1);
    if (!result->alphabet || !result->alphabet_copy_1
        || !result->alphabet_copy_2) {
        hashids_free(result);
        hashids_errno = HASHIDS_ERROR_ALLOC;
        return NULL;
    }

    /* extract only the unique characters */
    result->alphabet[0] = '\0';
    for (i = 0, j = 0; i < strlen(alphabet); ++i) {
        ch = alphabet[i];
        if (!strchr(result->alphabet, ch)) {
            result->alphabet[j++] = ch;
        }
    }
    result->alphabet[j] = '\0';

    /* store alphabet length */
    result->alphabet_length = j;

    /* check length and whitespace */
    if (result->alphabet_length < HASHIDS_MIN_ALPHABET_LENGTH) {
        hashids_free(result);
        hashids_errno = HASHIDS_ERROR_ALPHABET_LENGTH;
        return NULL;
    }
    if (strchr(result->alphabet, ' ')) {
        hashids_free(result);
        hashids_errno = HASHIDS_ERROR_ALPHABET_SPACE;
        return NULL;
    }

    /* copy salt */
    result->salt = strdup(salt ? salt : HASHIDS_DEFAULT_SALT);
    result->salt_length = strlen(result->salt);

    /* allocate enough space for separators */
    result->separators = _hashids_alloc(ceil((float)result->alphabet_length
        / HASHIDS_SEPARATOR_DIVISOR) + 1);
    if (!result->separators) {
        hashids_free(result);
        hashids_errno = HASHIDS_ERROR_ALLOC;
        return NULL;
    }

    /* non-alphabet characters cannot be separators */
    for (i = 0, j = 0; i < strlen(HASHIDS_DEFAULT_SEPARATORS); ++i) {
        ch = HASHIDS_DEFAULT_SEPARATORS[i];
        if ((p = strchr(result->alphabet, ch))) {
            result->separators[j++] = ch;

            /* also remove separators from alphabet */
            memmove(p, p + 1,
                strlen(result->alphabet) - (p - result->alphabet));
        }
    }

    /* store separators length */
    result->separators_count = j;

    /* subtract separators count from alphabet length */
    result->alphabet_length -= result->separators_count;

    /* shuffle the separators */
    hashids_shuffle(result->separators, result->separators_count,
        result->salt, result->salt_length);

    /* check if we have any/enough separators */
    if (!result->separators_count
        || (((float)result->alphabet_length / (float)result->separators_count)
                > HASHIDS_SEPARATOR_DIVISOR)) {
        int separators_count = ceil(
            (float)result->alphabet_length / HASHIDS_SEPARATOR_DIVISOR);

        if (separators_count == 1) {
            separators_count = 2;
        }

        if (separators_count > result->separators_count) {
            /* we need more separators - get some from alphabet */
            int diff = separators_count - result->separators_count;
            strncat(result->separators, result->alphabet, diff);
            memmove(result->alphabet, result->alphabet + diff,
                result->alphabet_length - diff + 1);

            result->separators_count += diff;
            result->alphabet_length -= diff;
        } else {
            /* we have more than enough - truncate */
            result->separators[separators_count] = '\0';
            result->separators_count = separators_count;
        }
    }

    /* shuffle alphabet */
    hashids_shuffle(result->alphabet, result->alphabet_length,
        result->salt, result->salt_length);

    /* allocate guards */
    result->guards_count = ceil((float)result->alphabet_length
        / HASHIDS_GUARD_DIVISOR);
    result->guards = _hashids_alloc(result->guards_count + 1);
    if (!result->guards) {
        hashids_free(result);
        hashids_errno = HASHIDS_ERROR_ALLOC;
        return NULL;
    }

    if (result->alphabet_length < 3) {
        /* take some from separators */
        strncpy(result->guards, result->separators, result->guards_count);
        memmove(result->separators, result->separators + result->guards_count,
            result->separators_count - result->guards_count + 1);

        result->separators_count -= result->guards_count;
    } else {
        /* take them from alphabet */
        strncpy(result->guards, result->alphabet, result->guards_count);
        memmove(result->alphabet, result->alphabet + result->guards_count,
            result->alphabet_length - result->guards_count + 1);

        result->alphabet_length -= result->guards_count;
    }

    /* set min hash length */
    result->min_hash_length = min_hash_length;

    /* return result happily */
    return result;
}

/* init with salt and minimum hash length */
struct hashids_t *
hashids_init2(const char *salt, unsigned int min_hash_length)
{
    return hashids_init3(salt, min_hash_length, HASHIDS_DEFAULT_ALPHABET);
}

/* init with hash only */
struct hashids_t *
hashids_init(const char *salt)
{
    return hashids_init2(salt, HASHIDS_DEFAULT_MIN_HASH_LENGTH);
}

/* estimate buffer size (generic) */
unsigned int
hashids_estimate_encoded_size(struct hashids_t *hashids,
    unsigned int numbers_count, unsigned long long *numbers)
{
    unsigned int result_len, i;
    unsigned long long number;

    /* start from 1 - the lottery character */
    result_len = 1;

    for (i = 0; i < numbers_count; ++i) {
        number = numbers[i];

        /* how long is the hash */
        do {
            ++result_len;
            number /= hashids->alphabet_length;
        } while (number);

        /* more than 1 number - separator */
        if (i + 1 < numbers_count) {
            ++result_len;
        }
    }

    /* minimum length checks */
    if (result_len++ < hashids->min_hash_length) {
        if (result_len++ < hashids->min_hash_length) {
            while (result_len < hashids->min_hash_length) {
                result_len += hashids->alphabet_length;
            }
        }
    }

    return result_len + 1;
}

/* estimate buffer size (variadic) */
unsigned int
hashids_estimate_encoded_size_v(struct hashids_t *hashids,
    unsigned int numbers_count, ...)
{
    int i;
    unsigned int result;
    unsigned long long *numbers;
    va_list ap;

    numbers = _hashids_alloc(numbers_count * sizeof(unsigned long long));

    if (!numbers) {
        hashids_errno = HASHIDS_ERROR_ALLOC;
        return 0;
    }

    va_start(ap, numbers_count);
    for (i = 0; i < numbers_count; ++i) {
        numbers[i] = va_arg(ap, unsigned long long);
    }
    va_end(ap);

    result = hashids_estimate_encoded_size(hashids, numbers_count, numbers);
    _hashids_free(numbers);

    return result;
}

/* encode many (generic) */
unsigned int
hashids_encode(struct hashids_t *hashids, char *buffer,
    unsigned int numbers_count, unsigned long long *numbers)
{
    /* bail out if no numbers */
    if (!numbers_count) {
        buffer[0] = '\0';

        return 0;
    }

    unsigned int i, j, result_len, guard_index, half_length_floor,
        half_length_ceil;
    unsigned long long number, number_copy, numbers_hash;
    int p_max, excess;
    char lottery, ch, temp_ch, *p, *buffer_end, *buffer_temp;

    /* return an estimation if no buffer */
    if (!buffer) {
        return hashids_estimate_encoded_size(hashids, numbers_count, numbers);
    }

    /* copy the alphabet into internal buffer 1 */
    strncpy(hashids->alphabet_copy_1, hashids->alphabet,
        hashids->alphabet_length);

    /* walk arguments once and generate a hash */
    for (i = 0, numbers_hash = 0; i < numbers_count; ++i) {
        number = numbers[i];
        numbers_hash += number % (i + 100);
    }

    /* lottery character */
    lottery = hashids->alphabet[numbers_hash % hashids->alphabet_length];

    /* start output buffer with it (or don't) */
    buffer[0] = lottery;
    buffer_end = buffer + 1;

    /* alphabet-like buffer used for salt at each iteration */
    hashids->alphabet_copy_2[0] = lottery;
    hashids->alphabet_copy_2[1] = '\0';
    strncat(hashids->alphabet_copy_2, hashids->salt,
        hashids->alphabet_length - 1);
    p = hashids->alphabet_copy_2 + hashids->salt_length + 1;
    p_max = hashids->alphabet_length - 1 - hashids->salt_length;
    if (p_max > 0) {
        strncat(hashids->alphabet_copy_2, hashids->alphabet,
            p_max);
    } else {
        hashids->alphabet_copy_2[hashids->alphabet_length] = '\0';
    }

    for (i = 0; i < numbers_count; ++i) {
        /* take number */
        number = number_copy = numbers[i];

        /* create a salt for this iteration */
        if (p_max > 0) {
            strncpy(p, hashids->alphabet_copy_1, p_max);
        }

        /* shuffle the alphabet */
        hashids_shuffle(hashids->alphabet_copy_1, hashids->alphabet_length,
            hashids->alphabet_copy_2, hashids->alphabet_length);

        /* hash the number */
        buffer_temp = buffer_end;
        do {
            ch = hashids->alphabet_copy_1[number % hashids->alphabet_length];
            *buffer_end++ = ch;

            number /= hashids->alphabet_length;
        } while (number);

        /* reverse the hash we got */
        for (j = 0; j < (buffer_end - buffer_temp) / 2; ++j) {
            temp_ch = *(buffer_temp + j);
            *(buffer_temp + j) = *(buffer_end - 1 - j);
            *(buffer_end - 1 - j) = temp_ch;
        }

        if (i + 1 < numbers_count) {
            number_copy %= ch + i;
            *buffer_end = hashids->separators[number_copy %
                hashids->separators_count];
            ++buffer_end;
        }
    }

    /* intermediate string length */
    result_len = buffer_end - buffer;

    /* add guards before start padding with alphabet */
    if (result_len < hashids->min_hash_length) {
        guard_index = (numbers_hash + buffer[0]) % hashids->guards_count;
        memmove(buffer + 1, buffer, result_len);
        buffer[0] = hashids->guards[guard_index];
        ++result_len;

        if (result_len < hashids->min_hash_length) {
            guard_index = (numbers_hash + buffer[2]) % hashids->guards_count;
            buffer[result_len] = hashids->guards[guard_index];
            ++result_len;

            /* pad with half alphabet before and after */
            half_length_floor = floor((float)hashids->alphabet_length / 2);
            half_length_ceil = ceil((float)hashids->alphabet_length / 2);

            /* pad, pad, pad */
            while (result_len < hashids->min_hash_length) {
                strncpy(hashids->alphabet_copy_2, hashids->alphabet_copy_1,
                    hashids->alphabet_length);
                hashids_shuffle(hashids->alphabet_copy_1,
                    hashids->alphabet_length, hashids->alphabet_copy_2,
                    hashids->alphabet_length);

                memmove(buffer + half_length_ceil, buffer, result_len);
                memmove(buffer, hashids->alphabet_copy_1 + half_length_floor,
                    half_length_ceil);
                memmove(buffer + result_len + half_length_ceil,
                    hashids->alphabet_copy_1, half_length_floor);

                result_len += hashids->alphabet_length;
                int excess = result_len - hashids->min_hash_length;

                if (excess > 0) {
                    memmove(buffer, buffer + excess / 2,
                        hashids->min_hash_length);
                    result_len = hashids->min_hash_length;
                }
            }
        }
    }

    buffer[result_len] = '\0';
    return result_len;
}

/* encode many (variadic) */
unsigned int
hashids_encode_v(struct hashids_t *hashids, char *buffer,
    unsigned int numbers_count, ...)
{
    int i;
    unsigned int result;
    unsigned long long *numbers;
    va_list ap;

    numbers = _hashids_alloc(numbers_count * sizeof(unsigned long long));

    if (!numbers) {
        hashids_errno = HASHIDS_ERROR_ALLOC;
        return 0;
    }

    va_start(ap, numbers_count);
    for (i = 0; i < numbers_count; ++i) {
        numbers[i] = va_arg(ap, unsigned long long);
    }
    va_end(ap);

    result = hashids_encode(hashids, buffer, numbers_count, numbers);
    _hashids_free(numbers);

    return result;
}

/* encode one */
unsigned int
hashids_encode_one(struct hashids_t *hashids, char *buffer,
    unsigned long long number)
{
    return hashids_encode(hashids, buffer, 1, &number);
}

/* numbers count */
unsigned int
hashids_numbers_count(struct hashids_t *hashids, char *str)
{
    unsigned int numbers_count;
    char ch, *p;

    /* skip characters until we find a guard */
    if (hashids->min_hash_length) {
        p = str;
        while ((ch = *p)) {
            if (strchr(hashids->guards, ch)) {
                str = p + 1;
                break;
            }

            p++;
        }
    }

    /* parse */
    numbers_count = 0;
    while ((ch = *str)) {
        if (strchr(hashids->guards, ch)) {
            break;
        }
        if (strchr(hashids->separators, ch)) {
            numbers_count++;
            str++;
            continue;
        }
        if (!strchr(hashids->alphabet, ch)) {
            hashids_errno = HASHIDS_ERROR_INVALID_HASH;
            return 0;
        }

        str++;
    }

    /* account for the last number */
    return numbers_count + 1;
}

/* decode */
unsigned int
hashids_decode(struct hashids_t *hashids, char *str,
    unsigned long long *numbers)
{
    unsigned int numbers_count;
    unsigned long long number;
    char lottery, ch, *p, *c;
    int p_max;

    numbers_count = hashids_numbers_count(hashids, str);

    if (!numbers) {
        return numbers_count;
    }

    /* skip characters until we find a guard */
    if (hashids->min_hash_length) {
        p = str;
        while ((ch = *p)) {
            if (strchr(hashids->guards, ch)) {
                str = p + 1;
                break;
            }

            p++;
        }
    }

    /* get the lottery character */
    lottery = *str++;

    /* copy the alphabet into internal buffer 1 */
    strncpy(hashids->alphabet_copy_1, hashids->alphabet,
        hashids->alphabet_length);

    /* alphabet-like buffer used for salt at each iteration */
    hashids->alphabet_copy_2[0] = lottery;
    hashids->alphabet_copy_2[1] = '\0';
    strncat(hashids->alphabet_copy_2, hashids->salt,
        hashids->alphabet_length - 1);
    p = hashids->alphabet_copy_2 + hashids->salt_length + 1;
    p_max = hashids->alphabet_length - 1 - hashids->salt_length;
    if (p_max > 0) {
        strncat(hashids->alphabet_copy_2, hashids->alphabet,
            p_max);
    } else {
        hashids->alphabet_copy_2[hashids->alphabet_length] = '\0';
    }

    /* first shuffle */
    hashids_shuffle(hashids->alphabet_copy_1, hashids->alphabet_length,
        hashids->alphabet_copy_2, hashids->alphabet_length);

    /* parse */
    number = 0;
    while ((ch = *str)) {
        if (strchr(hashids->guards, ch)) {
            break;
        }
        if (strchr(hashids->separators, ch)) {
            *numbers++ = number;
            number = 0;

            /* resalt the alphabet */
            if (p_max > 0) {
                strncpy(p, hashids->alphabet_copy_1, p_max);
            }
            hashids_shuffle(hashids->alphabet_copy_1, hashids->alphabet_length,
                hashids->alphabet_copy_2, hashids->alphabet_length);

            str++;
            continue;
        }
        if (!(c = strchr(hashids->alphabet_copy_1, ch))) {
            hashids_errno = HASHIDS_ERROR_INVALID_HASH;
            return 0;
        }

        number *= hashids->alphabet_length;
        number += c - hashids->alphabet_copy_1;

        str++;
    }

    /* store last number */
    *numbers = number;

    return numbers_count;
}

/* encode hex */
unsigned int
hashids_encode_hex(struct hashids_t *hashids, char *buffer,
    const char *hex_str)
{
    int len;
    char *temp, *p;
    unsigned long long number;
    unsigned int result;

    len = strlen(hex_str);
    temp = _hashids_alloc(len + 2);

    if (!temp) {
        hashids_errno = HASHIDS_ERROR_ALLOC;
        return 0;
    }

    temp[0] = '1';
    strncpy(temp + 1, hex_str, len);

    number = strtoull(temp, &p, 16);

    if (p == temp) {
        _hashids_free(temp);
        hashids_errno = HASHIDS_ERROR_INVALID_NUMBER;
        return 0;
    }

    result = hashids_encode(hashids, buffer, 1, &number);
    _hashids_free(temp);

    return result;
}

/* decode hex */
unsigned int
hashids_decode_hex(struct hashids_t *hashids, char *str, char *output)
{
    unsigned int result, i;
    unsigned long long number;
    char ch, *temp;

    result = hashids_numbers_count(hashids, str);

    if (result != 1) {
        return 0;
    }

    result = hashids_decode(hashids, str, &number);

    if (result != 1) {
        return 0;
    }

    temp = output;

    do {
        ch = number % 16;
        if (ch > 9) {
            ch += 'A' - 10;
        } else {
            ch += '0';
        }

        *temp++ = (char)ch;

        number /= 16;
    } while (number);

    temp--;
    *temp = 0;

    for (i = 0; i < (temp - output) / 2; ++i) {
        ch = *(output + i);
        *(output + i) = *(temp - 1 - i);
        *(temp - 1 - i) = ch;
    }

    return 1;
}
