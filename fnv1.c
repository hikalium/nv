#include <stdio.h>
#include <stdint.h>

// from http://wowdev.jp/?p=873
// http://www.isthe.com/chongo/tech/comp/fnv/

/*
 * FNV Constants
 */
static const uint32_t FNV_OFFSET_BASIS_32 = 2166136261U;
static const uint64_t FNV_OFFSET_BASIS_64 = 14695981039346656037U;

static const uint32_t FNV_PRIME_32 = 16777619U;
static const uint64_t FNV_PRIME_64 = 1099511628211LLU;

/*
 * FNV Hash Algorithm
 */
uint32_t fnv_1_hash_32(uint8_t *bytes, size_t length)
{
	uint32_t hash;
	size_t i;

	hash = FNV_OFFSET_BASIS_32;
	for( i = 0 ; i < length ; ++i) {
		hash = (FNV_PRIME_32 * hash) ^ (bytes[i]);
	}

	return hash;
}
uint64_t fnv_1_hash_64(uint8_t *bytes, size_t length)
{
	uint64_t hash;
	size_t i;

	hash = FNV_OFFSET_BASIS_64;
	for( i = 0 ; i < length ; ++i) {
		hash = (FNV_PRIME_64 * hash) ^ (bytes[i]);
	}

	return hash;
}
