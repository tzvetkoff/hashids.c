# ![hashids](http://www.hashids.org.s3.amazonaws.com/public/img/hashids.png "Hashids")

The [C](http://en.wikipedia.org/wiki/C_%28programming_language%29) port of the [Hashids](http://hashids.org/) library.

## Full Documentation

A small C library to generate YouTube-like ids from one or many numbers.
Use hashids when you do not want to expose your database ids to the user.
Read full documentation at: [http://hashids.org/c](http://hashids.org/c).

## Usage

The C usage differs marginally than [JavaScript's](http://hashids.org/javascript/) or [Ruby's](http://hashids.org/ruby/) in the matter that everything is left for the user.
You'll have to manually allocate and free all memory you need for encoding/decoding.
The library itself will only allocate the `hashids_t` structure (the handle) on its own.

#### API

##### hashids_init3

The most common initializer.
Takes 3 arguments:

* const char *salt
* unsigned int min_hash_length
* const char *alpabet

