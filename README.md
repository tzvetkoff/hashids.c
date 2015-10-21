# ![hashids](http://hashids.org/public/img/hashids-logo-normal.png "Hashids")

The [C](http://en.wikipedia.org/wiki/C_%28programming_language%29) port of the [Hashids](http://hashids.org/) library.

## Full Documentation

A small C library to generate Youtube-like IDs from one or many numbers.
Use __Hashids__ when you don't want to expose your database IDs to the user.

Read more about the library at [http://hashids.org/c/](http://hashids.org/c/).

## Usage

The C usage differs marginally than [JavaScript's](http://hashids.org/javascript/) or [Ruby's](http://hashids.org/ruby/) in the matter that nothing is done (auto-magically) for you.
You'll have to manually allocate and free all memory you need for encoding/decoding.
The library itself will only allocate the `hashids_t` structure (the handle) on its own, and even then [you've got the power](#memory-allocation).

### API

The C version of Hashids works (presumably, and only) with `unsigned long long` arguments.
Negative (signed) integers are, by design, treated as very big unsigned ones.

#### hashids_free

``` c
void
hashids_free(struct hashids_t *hashids);
```

The 'destructor'. This function disposes what you can allocate with the following 3 functions.
You'll definetely need to call this function when something goes wrong or you're finished (un)hashing your `ULONGLONG`s.

#### hashids_init3

``` c
struct hashids_t *
hashids_init3(const char *salt, unsigned int min_hash_length, const char *alphabet);
```

The most common initializer.

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

``` c
unsigned int
hashids_estimate_encoded_size(struct hashids_t *hashids, unsigned int numbers_count, unsigned long long *numbers);
```

Since we have no idea how much bytes an encoded `ULONGLONG` will take, there's this (pessimistic) function:

Example:

``` c
unsigned long long numbers[] = {1ull, 2ull, 3ull, 4ull, 5ull};
unsigned int bytes_needed;
bytes_needed = hashids_estimate_encoded_size(hashids, sizeof(numbers) / sizeof(unsigned long long), numbers);
/* bytes_needed => 12 */
```

#### hashids_estimate_encoded_size_v

``` c
unsigned int
hashids_estimate_encoded_size_v(struct hashids_t *hashids, unsigned int numbers_count, ...);
```

The variadic variant of the `hashids_estimate_encoded_size` function.

Example:

``` c
bytes_needed = hashids_estimate_encoded_size_v(hashids, 5, 1ull, 2ull, 3ull, 4ull, 5ull);
/* bytes_needed => 12 */
```

#### hashids_encode

``` c
unsigned int
hashids_encode(struct hashids_t *hashids, char *buffer, unsigned int numbers_count, unsigned long long *numbers);
```

The common encoding encoder.
Encodes an array of `ULONGLONG` numbers.
The returned value is the count of bytes encoded - if 0, something went south...

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

``` c
unsigned int
hashids_numbers_count(struct hashids_t *hashids, char *str);
```

If you thought that encoding is easy, think again.
Decoding is just as tough as encoding is - you'll have to manage the memory yourself.
Luckily, we thought about that too.

This nice function will tell you how much `ULONGLONG`s are hashed in the hash you'll be decoding.
If the function returns `0`, the hash is probably hashed with a different salt.
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
Will decode a string hash to an array of `ULONGLONG`s.
If the function returns `0`, the hash is probably hashed with a different salt.

Example:

``` c
unsigned long long numbers[5];
result = hashids_decode(hashids, "p2xkL3CK33JjcrrZ8vsw4YRZueZX9k", numbers);
/* numbers = {21979508, 35563591, 57543099, 93106690, 150649789}, result => 5 */
```

#### hashids_encode_hex

``` c
unsigned int
hashids_encode_hex(struct hashids_t *hashids, char *buffer, const char *hex_str);
```

Encodes a hex string rather than a number.

Example:

``` c
bytes_encoded = hashids_encode_hex(hashids, hash, "C0FFEE");
/* hash => "6N6LO4", result => 6 */
```

#### hashids_decode_hex

``` c
unsigned int
hashids_decode_hex(struct hashids_t *hashids, char *str, char *output);
```

Decodes a hash to a hex string rather than to a number.

Example:

``` c
char str[18];   /* sizeof(unsigned long long) * 2 + 2 */
result = hashids_decode_hex(hashids, "6N6LO4", str);
/* str => "C0FFEE", result => 1 */
```

## Error checking

The library uses its own `extern int hashids_errno` for error handling, thus it does not mangle the system-wide `errno`.
`hashids_errno` definitions:

| CONSTANT                              | Code  | Description                                                           |
| ------------------------------------- | ----- | --------------------------------------------------------------------- |
| __HASHIDS_ERROR_OK__                  |     0 | Happy, do nothing!                                                    |
| __HASHIDS_ERROR_ALLOC__               |    -1 | Memory allocation error                                               |
| __HASHIDS_ERROR_ALPHABET_LENGTH__     |    -2 | The alphabet is shorter than `HASHIDS_MIN_ALPHABET_LENGTH` (16 chars) |
| __HASHIDS_ERROR_ALPHABET_SPACE__      |    -3 | The alphabet contains a space (tab NOT included)                      |
| __HASHIDS_ERROR_INVALID_HASH__        |    -4 | An invalid hash has been passed to `hashids_decode()`                 |
| __HASHIDS_ERROR_INVALID_NUMBER__      |    -5 | An invalid hex string has been passed to `hashids_encode_hex()`       |

## Memory allocation

Since the `hashids_init*` (and some of the `*_v`) functions are memory-dependent, this library is trying to be allocator-agnostic.
If you roll your own allocator, and for some reason you don't like external libraries calling `malloc`/`calloc`, we've certainly got you covered.
Assuming that you know regular C syntax, we define 2 externals:

``` c
extern void *(*_hashids_alloc)(size_t size);
extern void (*_hashids_free)(void *ptr);
```

The library implementation is as simple as possible:

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

Oh, PLEASE __NOTE__: `hashids_init3` (most likely) relies on that memory allocated is zero-initialized.

## CLI

As you might've noticed, there's a command line utility utilizing the `hashids` library and providing almost all the functionality in the shell.

The usage is not much different. If you have any trouble, just run the command without any arguments and read the help lines.

``` bash
./hashids 1
# => jR
./hashids -d jR
# => 1
```

You can also consult the source (`main.c`) for further help on using `hashids`.

## Issues

[Oh, no!](https://github.com/tzvetkoff/hashids.c/issues/new)

## Hacking

Fork the repo, then send me a pull request.
Bonus points for topic branches.
