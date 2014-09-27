# ![hashids](http://www.hashids.org.s3.amazonaws.com/public/img/hashids.png "Hashids")

The [C](http://en.wikipedia.org/wiki/C_%28programming_language%29) port of the [Hashids](http://hashids.org/) library.

## Full Documentation

A small C library to generate Youtube-like IDs from one or many numbers.
Use __Hashids__ when you don't want to expose your database ids to the user.

Read the full documentation at [http://hashids.org/c](http://hashids.org/c).

## Usage

The C usage differs marginally than [JavaScript's](http://hashids.org/javascript/) or [Ruby's](http://hashids.org/ruby/) in the matter that everything is left for the user.
You'll have to manually allocate and free all memory you need for encoding/decoding.
The library itself will only allocate the `hashids_t` structure (the handle) on its own, and even then [you've got the power](#memory-allocation).

### API

The C version of Hashids works (presumably, and only) with `unsigned long long` arguments.
Negative (signed) integers are, by design, treated as very big unsigned ones.

#### hashids_init3

``` c
struct hashids_t *
hashids_init3(const char *salt, unsigned int min_hash_length, const char *alphabet);
```

The most common initializer.
Takes 3 arguments:

* const char *salt
* unsigned int min_hash_length
* const char *alpabet

Example:

``` c
struct hashids_t *hashids;
hashids = hashids_init3("this is my salt", 0, HASHIDS_DEFAULT_ALPHABET);
```

#### hashids_init2

``` c
struct hashids_t *
hashids_init2(const char *salt, unsigned int min_hash_length);
```

The same as `hashids_init3` but without the `alpabet` parameter.
Will use `HASHIDS_DEFAULT_ALPHABET` as alphabet.

#### hashids_init

``` c
struct hashids_t *
hashids_init(const char *salt);
```

The same as `hashids_init2` but using `0` as `min_hash_length`.
If you pass `NULL` for `salt` the `HASHIDS_DEFAULT_SALT` will be used (currently `""`).

#### hashids_estimate_encoded_size

Since we have no idea how much bytes an encoded `ULONGLONG` will take, there's this (pessimistic) function:

``` c
unsigned int
hashids_estimate_encoded_size(struct hashids_t *hashids, unsigned int numbers_count, unsigned long long *numbers);
```

Example:

``` c
unsigned long long numbers[] = {1ull, 2ull, 3ull, 4ull, 5ull};
unsigned int bytes_needed;
bytes_needed = hashids_estimate_encoded_size(hashids, sizeof(numbers) / sizeof(unsigned long long), numbers);
```

#### hashids_estimate_encoded_size_v

The variadic variant of the `hashids_estimate_encoded_size` function.

``` c
unsigned int
hashids_estimate_encoded_size_v(struct hashids_t *hashids, unsigned int numbers_count, ...);
```

Example:

``` c
bytes_needed = hashids_estimate_encoded_size_v(hashids, 5, 1ull, 2ull, 3ull, 4ull, 5ull);
```

#### hashids_encode

``` c
unsigned int
hashids_encode(struct hashids_t *hashids, char *buffer, unsigned int numbers_count, unsigned long long *numbers);
```

The common encoding encoder.
Encodes an array of `ULONGLONG` numbers.
Takes 4 arguments:

* struct hashids_t *hashids - the hashids_t handle
* char *buffer - the string buffer where the encoded hash will be put
* unsigned int numbers_count - the count of numbers we'll be encoding
* unsigned long long *numbers - the array of numbers

Example:

``` c
unsigned int bytes_encoded;
char hash[512];
unsigned long long numbers[] = {1ull};
bytes_encoded = hashids_encode(hashids, hash, sizeof(numbers) / sizeof(unsigned long long), numbers);
/* hash => "NV", bytes_encoded => 2 */
unsigned long long numbers2[] = {1ull, 2ull, 3ull, 4ull, 5ull};
bytes_encoded = hashids_encode(hashids, hash, sizeof(numbers2) / sizeof(unsigned long long), numbers2);
/* hash => "zoHWuNhktp", bytes_encoded => 10 */
```

#### hashids_encode_v

``` c
unsigned int
hashids_encode_v(struct hashids_t *hashids, char *buffer, unsigned int numbers_count, ...);
```

The variadic variant of `hashids_encode`.

Example:

``` c
bytes_encoded = hashids_encode_v(hashids, hash, 1, 1ull);
/* hash => "NV", bytes_encoded => 2 */
bytes_encoded = hashids_encode_v(hashids, hash, 5, 1ull, 2ull, 3ull, 4ull, 5ull);
/* hash => "zoHWuNhktp", bytes_encoded => 10 */
```

#### hashids_encode_one

``` c
unsigned int
hashids_encode_one(struct hashids_t *hashids, char *buffer, unsigned long long number);
```

A shorthand function encoding just one `ULONGLONG`.

Example:

``` c
bytes_encoded = hashids_encode_one(hashids, hash, 12345);
/* hash => "NkK9", bytes_encoded => 4 */
```

#### hashids_numbers_count

If you thought that encoding is easy, think again.
Decoding is just as tough as encoding is - you'll have to manage the memory yourself.
Luckily, we thought about that too:

``` c
unsigned int
hashids_numbers_count(struct hashids_t *hashids, char *str);
```

That nice function will tell you how much `ULONGLONG`s are hashed in the hash you'll be decoding.
It's up to you to allocate `result * sizeof(unsigned long long)` enough memory yourself.

Example:

``` c
unsigned int numbers_count = hashids_numbers_count(hashids, "zoHWuNhktp");
/* numbers_count => 5 */
```

#### hashids_decode

``` c
unsigned int
hashids_decode(struct hashids_t *hashids, char *str, unsigned long long *numbers);
```

The common decoding decoder.
Takes 3 parameters:

* struct hashids_t *hashids - the hashids_t handle
* char *str - the hash we'll be decoding
* the output `ULONGLONG` array

Example:

``` c
unsigned long long numbers[5];
result = hashids_decode(hashids, "p2xkL3CK33JjcrrZ8vsw4YRZueZX9k", numbers);
/* numbers = {21979508, 35563591, 57543099, 93106690, 150649789 }, result => 5 */
```

## Error checking

The library uses its own `extern int hashids_errno` for error handling, thus it does not mangle system-wide `errno`.
`hashids_errno` definitions:

* HASHIDS_ERROR_OK => 0 -- happy
* HASHIDS_ERROR_ALLOC => -1 -- memory allocation error
* HASHIDS_ERROR_ALPHABET_LENGTH => -2 -- alphabet is shorter than `HASHIDS_MIN_ALPHABET_LENGTH` (16 chars)
* HASHIDS_ERROR_ALPHABET_SPACE => -3 -- alpabet contains a space (`' '`)
* HASHIDS_ERROR_INVALID_HASH => -4 -- an invalid hash has been passed to hashids_decode()

## Memory allocation

Since the `hashids_init*` (and some of the `*_v`) functions are memory-dependent, this library is trying to be allocator-agnostic.
If you roll your own allocator, and for some reason you don't like external libraries calling `malloc`/`calloc`, we've certainly got you covered.
Assuming that you know regular C syntax, we define 2 externals:

``` c
extern void *(*_hashids_alloc)(size_t size);
extern void (*_hashids_free)(void *ptr);
```

The library implementation is simple:

``` c
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
```

Feel free to reassign those at your will.

## Issues

[Oh, no!](https://github.com/tzvetkoff/hashids.c/issues/new)

## Hacking

Fork teh repo, then send me a pull request.
