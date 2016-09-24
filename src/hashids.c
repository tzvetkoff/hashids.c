#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

#include "hashids.h"

#ifndef __has_builtin
#   define __has_builtin(x) (0)
#endif
#if defined(__builtin_expect) || __has_builtin(__builtin_expect)
#   define HASHIDS_LIKELY(x)        (__builtin_expect(!!(x), 1))
#   define HASHIDS_UNLIKELY(x)      (__builtin_expect(!!(x), 0))
#else
#   define HASHIDS_LIKELY(x)        (x)
#   define HASHIDS_UNLIKELY(x)      (x)
#endif

/* exported hashids_errno */
int hashids_errno;

/* alloc/free */
static void *
hashids_alloc_f(size_t size)
{
    return calloc(size, 1);
}

static void *
hashids_realloc_f(void *ptr, size_t size)
{
    return realloc(ptr, size);
}

static void
hashids_free_f(void *ptr)
{
    free(ptr);
}

void *(*_hashids_alloc)(size_t size) = hashids_alloc_f;
void *(*_hashids_realloc)(void *ptr, size_t size) = hashids_realloc_f;
void (*_hashids_free)(void *ptr) = hashids_free_f;

/* shuffle loop step */
#define hashids_shuffle_step(iter) \
    if (i == 0) { break; }                                      \
    if (v == salt_length) { v = 0; }                            \
    p += salt[v]; j = (salt[v] + v + p) % (iter);               \
    temp = str[(iter)]; str[(iter)] = str[j]; str[j] = temp;    \
    --i; ++v;

/* consistent shuffle */
void
hashids_shuffle(char *str, size_t str_length, char *salt, size_t salt_length)
{
    ssize_t i;
    size_t j, v, p;
    char temp;

    if (!salt_length) {
        return;
    }

    for (i = str_length - 1, v = 0, p = 0; i > 0; /* empty */) {
        switch (i % 32) {
            case 31: hashids_shuffle_step(i);
            case 30: hashids_shuffle_step(i);
            case 29: hashids_shuffle_step(i);
            case 28: hashids_shuffle_step(i);
            case 27: hashids_shuffle_step(i);
            case 26: hashids_shuffle_step(i);
            case 25: hashids_shuffle_step(i);
            case 24: hashids_shuffle_step(i);
            case 23: hashids_shuffle_step(i);
            case 22: hashids_shuffle_step(i);
            case 21: hashids_shuffle_step(i);
            case 20: hashids_shuffle_step(i);
            case 19: hashids_shuffle_step(i);
            case 18: hashids_shuffle_step(i);
            case 17: hashids_shuffle_step(i);
            case 16: hashids_shuffle_step(i);
            case 15: hashids_shuffle_step(i);
            case 14: hashids_shuffle_step(i);
            case 13: hashids_shuffle_step(i);
            case 12: hashids_shuffle_step(i);
            case 11: hashids_shuffle_step(i);
            case 10: hashids_shuffle_step(i);
            case  9: hashids_shuffle_step(i);
            case  8: hashids_shuffle_step(i);
            case  7: hashids_shuffle_step(i);
            case  6: hashids_shuffle_step(i);
            case  5: hashids_shuffle_step(i);
            case  4: hashids_shuffle_step(i);
            case  3: hashids_shuffle_step(i);
            case  2: hashids_shuffle_step(i);
            case  1: hashids_shuffle_step(i);
            case  0: hashids_shuffle_step(i);
        }
    }
}

/* "destructor" */
void
hashids_free(hashids_t *hashids)
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
hashids_t *
hashids_init3(const char *salt, size_t min_hash_length, const char *alphabet)
{
    hashids_t *result;
    size_t i, j, len;
    char ch, *p;

    hashids_errno = HASHIDS_ERROR_OK;

    /* allocate the structure */
    result = _hashids_alloc(sizeof(hashids_t));
    if (HASHIDS_UNLIKELY(!result)) {
        hashids_errno = HASHIDS_ERROR_ALLOC;
        return NULL;
    }

    /* allocate enough space for the alphabet and its copies */
    len = strlen(alphabet) + 1;
    result->alphabet = _hashids_alloc(len);
    result->alphabet_copy_1 = _hashids_alloc(len);
    result->alphabet_copy_2 = _hashids_alloc(len);
    if (HASHIDS_UNLIKELY(!result->alphabet || !result->alphabet_copy_1
        || !result->alphabet_copy_2)) {
        hashids_free(result);
        hashids_errno = HASHIDS_ERROR_ALLOC;
        return NULL;
    }

    /* extract only the unique characters */
    result->alphabet[0] = '\0';
    for (i = 0, j = 0; i < len; ++i) {
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
    if (strchr(result->alphabet, 0x20) || strchr(result->alphabet, 0x09)) {
        hashids_free(result);
        hashids_errno = HASHIDS_ERROR_ALPHABET_SPACE;
        return NULL;
    }

    /* copy salt */
    result->salt_length = salt ? strlen(salt) : 0;
    result->salt = _hashids_alloc(result->salt_length + 1);
    if (HASHIDS_UNLIKELY(!result->salt)) {
        hashids_free(result);
        hashids_errno = HASHIDS_ERROR_ALLOC;
        return NULL;
    }
    strncpy(result->salt, salt, result->salt_length);

    /* allocate enough space for separators */
    len = strlen(HASHIDS_DEFAULT_SEPARATORS);
    j = (size_t)
        (ceil((float)result->alphabet_length / HASHIDS_SEPARATOR_DIVISOR) + 1);
    if (j < len + 1) {
        j = len + 1;
    }

    result->separators = _hashids_alloc(j);
    if (HASHIDS_UNLIKELY(!result->separators)) {
        hashids_free(result);
        hashids_errno = HASHIDS_ERROR_ALLOC;
        return NULL;
    }

    /* take default separators out of the alphabet */
    for (i = 0, j = 0; i < strlen(HASHIDS_DEFAULT_SEPARATORS); ++i) {
        ch = HASHIDS_DEFAULT_SEPARATORS[i];

        /* check if separator is actually in the used alphabet */
        if ((p = strchr(result->alphabet, ch))) {
            result->separators[j++] = ch;

            /* remove that separator */
            memmove(p, p + 1,
                strlen(result->alphabet) - (p - result->alphabet));
        }
    }

    /* store separators length */
    result->separators_count = j;

    /* subtract separators count from alphabet length */
    result->alphabet_length -= result->separators_count;

    /* shuffle the separators */
    if (result->separators_count) {
        hashids_shuffle(result->separators, result->separators_count,
            result->salt, result->salt_length);
    }

    /* check if we have any/enough separators */
    if (!result->separators_count
        || (((float)result->alphabet_length / (float)result->separators_count)
            > HASHIDS_SEPARATOR_DIVISOR)) {
        size_t separators_count = (size_t)ceil(
            (float)result->alphabet_length / HASHIDS_SEPARATOR_DIVISOR);

        if (separators_count == 1) {
            separators_count = 2;
        }

        if (separators_count > result->separators_count) {
            /* we need more separators - get some from alphabet */
            size_t diff = separators_count - result->separators_count;
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
    result->guards_count = (size_t)ceil((float)result->alphabet_length
                                               / HASHIDS_GUARD_DIVISOR);
    result->guards = _hashids_alloc(result->guards_count + 1);
    if (HASHIDS_UNLIKELY(!result->guards)) {
        hashids_free(result);
        hashids_errno = HASHIDS_ERROR_ALLOC;
        return NULL;
    }

    if (HASHIDS_UNLIKELY(result->alphabet_length < 3)) {
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
hashids_t *
hashids_init2(const char *salt, size_t min_hash_length)
{
    return hashids_init3(salt, min_hash_length, HASHIDS_DEFAULT_ALPHABET);
}

/* init with hash only */
hashids_t *
hashids_init(const char *salt)
{
    return hashids_init2(salt, HASHIDS_DEFAULT_MIN_HASH_LENGTH);
}

/* frees a hashids arena */
void hashids_arena_free(hashids_arena_t *hashids_arena) {
  if (hashids_arena) {
    if (hashids_arena->buffer) {
      _hashids_free(hashids_arena->buffer);
    }

    if (hashids_arena->numbers) {
      _hashids_free(hashids_arena->numbers);
    }

    _hashids_free(hashids_arena);
  }
}

/* init an arena */
hashids_arena_t *hashids_new_arena_init(size_t buffer_size, size_t numbers_count) {
  if (HASHIDS_UNLIKELY(buffer_size == 0 || numbers_count == 0)) {
    hashids_errno = HASHIDS_ERROR_ALLOC;
    return NULL;
  }
  hashids_arena_t *result;
  hashids_errno = HASHIDS_ERROR_OK;

  /* allocate the structure */
  result = _hashids_alloc(sizeof(hashids_arena_t));
  if (HASHIDS_UNLIKELY(!result)) {
      hashids_errno = HASHIDS_ERROR_ALLOC;
      return NULL;
  }

  result->buffer = _hashids_alloc(buffer_size);

  if (HASHIDS_UNLIKELY(!result->buffer)) {
    hashids_errno = HASHIDS_ERROR_ALLOC;
    hashids_arena_free(result);
    return NULL;
  }

  result->buffer_size = buffer_size;

  result->numbers = _hashids_alloc(numbers_count * sizeof(unsigned long long));

  if (HASHIDS_UNLIKELY(!result->numbers)) {
    hashids_errno = HASHIDS_ERROR_ALLOC;
    hashids_arena_free(result);
    return NULL;
  }

  result->numbers_count = numbers_count;

  return result;
}

/* init an arena for a specific hashids instance */
hashids_arena_t *hashids_arena_init(hashids_t *hashids) {
  if (HASHIDS_UNLIKELY(!hashids)) {
    hashids_errno = HASHIDS_ERROR_ALLOC;
    return NULL;
  }

  size_t minimal_buffer_size;
  if (hashids->min_hash_length > 0) {
    minimal_buffer_size = hashids->min_hash_length + 1;
  }
  else {
    minimal_buffer_size = 32;
  }

  return hashids_new_arena_init(minimal_buffer_size, 16);
}

/* estimate buffer size (generic) */
size_t
hashids_estimate_encoded_size(hashids_t *hashids,
    size_t numbers_count, unsigned long long *numbers)
{
    int i, result_len;

    for (i = 0, result_len = 1; i < numbers_count; ++i) {
        if (numbers[i] == 0) {
            result_len += 2;
        } else if (numbers[i] == 0xFFFFFFFFFFFFFFFFull) {
            result_len += ceil(log2(numbers[i]) / log2(hashids->alphabet_length)) + 1;
        } else {
            result_len += ceil(log2(numbers[i] + 1) / log2(hashids->alphabet_length)) + 1;
        }
    }

    if (result_len <= hashids->min_hash_length) {
        result_len = hashids->min_hash_length + 1;
    }

    return result_len;
}

/* estimate buffer size and realloc the arena in case it's too small */
size_t
hashids_estimate_encoded_size_arena(hashids_t *hashids, hashids_arena_t *hashids_arena,
    size_t numbers_count, unsigned long long *numbers) {
    size_t estimated_encoded_size = hashids_estimate_encoded_size(hashids, numbers_count, numbers);
    if (hashids_arena->buffer_size < estimated_encoded_size) {
      hashids_arena->buffer = _hashids_realloc(hashids_arena->buffer, estimated_encoded_size);
      if (HASHIDS_UNLIKELY(!hashids_arena->buffer)) {
          hashids_errno = HASHIDS_ERROR_ALLOC;
          return 0;
      }
      hashids_arena->buffer_size = estimated_encoded_size;
    }

    return estimated_encoded_size;
}

/* estimate buffer size (variadic) */
size_t
hashids_estimate_encoded_size_v(hashids_t *hashids,
    size_t numbers_count, ...)
{
    size_t i, result;
    unsigned long long *numbers;
    va_list ap;

    numbers = _hashids_alloc(numbers_count * sizeof(unsigned long long));

    if (HASHIDS_UNLIKELY(!numbers)) {
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
size_t
hashids_encode(hashids_t *hashids, char *buffer,
    size_t numbers_count, unsigned long long *numbers)
{
    /* bail out if no numbers */
    if (HASHIDS_UNLIKELY(!numbers_count)) {
        buffer[0] = '\0';

        return 0;
    }

    size_t i, j, result_len, guard_index, half_length_ceil, half_length_floor;
    unsigned long long number, number_copy, numbers_hash;
    int p_max;
    char lottery, ch, temp_ch, *p, *buffer_end, *buffer_temp;

    /* return an estimation if no buffer */
    if (HASHIDS_UNLIKELY(!buffer)) {
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

    if (result_len < hashids->min_hash_length) {
        /* add a guard before the encoded numbers */
        guard_index = (numbers_hash + buffer[0]) % hashids->guards_count;
        memmove(buffer + 1, buffer, result_len);
        buffer[0] = hashids->guards[guard_index];
        ++result_len;

        if (result_len < hashids->min_hash_length) {
            /* add a guard after the encoded numbers */
            guard_index = (numbers_hash + buffer[2]) % hashids->guards_count;
            buffer[result_len] = hashids->guards[guard_index];
            ++result_len;

            /* pad with half alphabet before and after */
            half_length_ceil = ceil((float)hashids->alphabet_length / 2);
            half_length_floor = floor((float)hashids->alphabet_length / 2);

            /* pad, pad, pad */
            while (result_len < hashids->min_hash_length) {
                /* shuffle the alphabet */
                strncpy(hashids->alphabet_copy_2, hashids->alphabet_copy_1,
                    hashids->alphabet_length);
                hashids_shuffle(hashids->alphabet_copy_1,
                    hashids->alphabet_length, hashids->alphabet_copy_2,
                    hashids->alphabet_length);

                /* left pad from the end of the alphabet */
                i = ceil((float)(hashids->min_hash_length - result_len) / 2);
                /* right pad from the beginning */
                j = floor((float)(hashids->min_hash_length - result_len) / 2);

                /* check bounds */
                if (i > half_length_ceil) {
                    i = half_length_ceil;
                }
                if (j > half_length_floor) {
                    j = half_length_floor;
                }

                /* edge case - only pad left */
                if (i == 1 && j == i) {
                    i = 2; j = 0;
                }

                /* move the current result to "center" */
                memmove(buffer + i, buffer, result_len);
                /* pad left */
                memmove(buffer,
                    hashids->alphabet_copy_1 + hashids->alphabet_length - i, i);
                /* pad right */
                memmove(buffer + i + result_len, hashids->alphabet_copy_1, j);

                /* increment result_len */
                result_len += i + j;
            }
        }
    }

    buffer[result_len] = '\0';
    return result_len;
}

/* encode many (variadic) */
size_t
hashids_encode_v(hashids_t *hashids, char *buffer,
    size_t numbers_count, ...)
{
    int i;
    size_t result;
    unsigned long long *numbers;
    va_list ap;

    numbers = _hashids_alloc(numbers_count * sizeof(unsigned long long));

    if (HASHIDS_UNLIKELY(!numbers)) {
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
size_t
hashids_encode_one(hashids_t *hashids, char *buffer,
    unsigned long long number)
{
    return hashids_encode(hashids, buffer, 1, &number);
}

/* numbers count */
size_t
hashids_numbers_count(hashids_t *hashids, char *str)
{
    size_t numbers_count;
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

size_t
hashids_numbers_count_arena(hashids_t *hashids, hashids_arena_t *hashids_arena, char *str) {
  size_t numbers_count = hashids_numbers_count(hashids, str);
  if (hashids_arena->numbers_count < numbers_count) {
    hashids_arena->numbers = _hashids_realloc(hashids_arena->numbers, numbers_count * sizeof(unsigned long long));
    if (HASHIDS_UNLIKELY(!hashids_arena->buffer)) {
        hashids_errno = HASHIDS_ERROR_ALLOC;
        return 0;
    }
    hashids_arena->numbers_count = numbers_count;
  }
  return numbers_count;
}

/* decode */
size_t
hashids_decode(hashids_t *hashids, char *str,
    unsigned long long *numbers)
{
    size_t numbers_count;
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
size_t
hashids_encode_hex(hashids_t *hashids, char *buffer,
    const char *hex_str)
{
    int len;
    char *temp, *p;
    size_t result;
    unsigned long long number;

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
size_t
hashids_decode_hex(hashids_t *hashids, char *str, char *output)
{
    size_t result, i;
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
