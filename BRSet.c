//
//  BRSet.c
//
//  Created by Aaron Voisine on 9/11/15.
//  Copyright (c) 2015 breadwallet LLC
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

#include "BRSet.h"
#include <stdlib.h>
#include <string.h>

#define C1 0xcc9e2d51
#define C2 0x1b873593

// bitwise left rotation
#define rol32(a, b) (((a) << (b)) | ((a) >> (32 - (b))))

#define fmix32(h) (h ^= h >> 16, h *= 0x85ebca6b, h ^= h >> 13, h *= 0xc2b2ae35, h ^= h >> 16)

// murmurHash3 (x86_32): https://code.google.com/p/smhasher/
uint32_t BRMurmur3_32(const void *data, size_t len, uint32_t seed)
{
    uint32_t h = seed, k = 0;
    size_t i, count = len/4;
    
    for (i = 0; i < count; i++) {
        k = ((uint32_t *)data)[i]*C1;//le32(((uint32_t *)data)[i])*C1;
        k = rol32(k, 15)*C2;
        h ^= k;
        h = rol32(h, 13)*5 + 0xe6546b64;
    }
    
    k = 0;
    
    switch (len & 3) {
        case 3: k ^= ((uint8_t *)data)[i*4 + 2] << 16; // fall through
        case 2: k ^= ((uint8_t *)data)[i*4 + 1] << 8; // fall through
        case 1: k ^= ((uint8_t *)data)[i*4], k *= C1, h ^= rol32(k, 15)*C2;
    }
    
    h ^= len;
    fmix32(h);
    return h;
}

// linear probed hashtable for good cache performance, maximum load factor is 2/3

static const size_t tableSizes[] = { // starting with 1, multiply by 3/2, round up and find next largest prime
    1, 3, 7, 13, 23, 37, 59, 97, 149, 227, 347, 523, 787, 1187, 1783, 2677, 4019, 6037, 9059, 13591,
    20389, 30593, 45887, 68863, 103307, 154981, 232487, 348739, 523129, 784697, 1177067, 1765609,
    2648419, 3972643, 5958971, 8938469, 13407707, 20111563, 30167359, 45251077, 67876637, 101814991,
    152722489, 229083739, 343625629, 515438447, 773157683, 1159736527, 1739604799, 2609407319, 3914111041
};

#define TABLE_SIZES_LEN (sizeof(tableSizes)/sizeof(*tableSizes))

struct _BRSet {
    void **table; // hashtable
    size_t size; // number of buckets in table
    size_t itemCount; // number of items in set
    size_t (*hash)(const void *); // hash function
    int (*eq)(const void *, const void *); // equality function
};

void BRSetInit(BRSet *set, size_t (*hash)(const void *), int (*eq)(const void *, const void *), size_t capacity)
{
    size_t i = 0;
    
    while (i < TABLE_SIZES_LEN && tableSizes[i] < capacity) i++;

    if (i + 1 < TABLE_SIZES_LEN) {
        set->table = calloc(tableSizes[i + 1], sizeof(void *));
        set->size = tableSizes[i + 1];
    }
    
    set->itemCount = 0;
    set->hash = (hash) ? hash : BRPairHash;
    set->eq = (eq) ? eq : BRPairEq;
}

// retruns a newly allocated empty set that must be freed by calling BRSetFree(), hash is a function that returns a hash
// value for a given set item, eq is a function that tests if two item elements are equal, capacity is the expected
// number of items the set will hold
BRSet *BRSetNew(size_t (*hash)(const void *), int (*eq)(const void *, const void *), size_t capacity)
{
    BRSet *set = malloc(sizeof(BRSet));
    
    BRSetInit(set, hash, eq, capacity);
    return set;
}

// rebuilds hashtable to hold up to capacity items
static void BRSetGrow(BRSet *set, size_t capacity)
{
    BRSet newSet;
    
    BRSetInit(&newSet, set->hash, set->eq, capacity);
    BRSetUnion(&newSet, set);
    free(set->table);
    set->table = newSet.table;
    set->size = newSet.size;
}

// adds given item to set or replaces an equivalent existing item
void BRSetAdd(BRSet *set, void *item)
{
    size_t size = set->size;
    size_t i = set->hash(item) % size;
    void *t = set->table[i];

    while (t && t != item && ! set->eq(t, item)) {
        i = (i + 1) % size;
        t = set->table[i];
    }

    if (! t) set->itemCount++;
    set->table[i] = item;
    if (set->itemCount > ((size + 2)/3)*2) BRSetGrow(set, size);
}

// removes given item from set
void BRSetRemove(BRSet *set, const void *item)
{
    size_t size = set->size;
    size_t i = set->hash(item) % size;
    void *t = set->table[i];

    while (t != item && t && ! set->eq(t, item)) {
        i = (i + 1) % size;
        t = set->table[i];
    }
    
    if (t) {
        set->itemCount--;
        set->table[i] = NULL;
        i = (i + 1) % size;
        t = set->table[i];
        
        while (t) { // hashtable cleanup
            set->itemCount--;
            set->table[i] = NULL;
            BRSetAdd(set, t);
            i = (i + 1) % size;
            t = set->table[i];
        }
    }
}

// removes all items from set
void BRSetClear(BRSet *set)
{
    memset(set->table, 0, set->size);
    set->itemCount = 0;
}

// returns the number of items in set
inline size_t BRSetCount(BRSet *set)
{
    return set->itemCount;
}

// true if item is contained in set
int BRSetContains(BRSet *set, const void *item)
{
    return (BRSetGet(set, item) != NULL);
}

// returns member item from set equivalent to given item
void *BRSetGet(BRSet *set, const void *item)
{
    size_t size = set->size;
    size_t i = set->hash(item) % size;
    void *r = set->table[i];

    while (r != item && r && ! set->eq(r, item)) {
        i = (i + 1) % size;
        r = set->table[i];
    }
    
    return r;
}

// returns an initial random item from set for use when iterating, or NULL if set is empty
void *BRSetFirst(BRSet *set)
{
    size_t i = 0, size = set->size;
    void *r = NULL;

    while (! r && i < size) r = set->table[i++];
    return r;
}

// returns the next item after given item when iterating, or NULL if no more items are available
void *BRSetNext(BRSet *set, const void *item)
{
    size_t size = set->size;
    size_t i = set->hash(item) % size;
    void *t = set->table[i], *r = NULL;
    
    while (t != item && t && ! set->eq(t, item)) {
        i = (i + 1) % size;
        t = set->table[i];
    }
    
    i++;
    while (! r && i < size) r = set->table[i++];
    return r;
}

// adds or replaces items from otherSet into set
void BRSetUnion(BRSet *set, const BRSet *otherSet)
{
    size_t i = 0, size = otherSet->size;
    void *t;
    
    if (otherSet->itemCount > ((set->size + 2)/3)*2) BRSetGrow(set, otherSet->itemCount);
    
    while (i < size) {
        t = otherSet->table[i++];
        if (t) BRSetAdd(set, t);
    }
}

// removes items contained in otherSet from set
void BRSetMinus(BRSet *set, const BRSet *otherSet)
{
    size_t i = 0, size = otherSet->size;
    void *t;
    
    while (i < size) {
        t = otherSet->table[i++];
        if (t) BRSetRemove(set, t);
    }
}

// frees memory allocated for set
void BRSetFree(BRSet *set)
{
    free(set->table);
    free(set);
}
