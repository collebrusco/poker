/** 
 * util.h 
 * poker
 * created 03/17/25 by frank collebrusco
 */
#ifndef UTIL_H
#define UTIL_H
#include <cstddef>
#include <cstdint>
#include <stdio.h>
#include <vector>
#include <random>

#ifndef assert
    #define assert(expr) if (!(expr)) lg("ERROR: ASSERTION FAILED!!! %s\n", #expr)
#endif
#define tassert(expr) if (!(expr)) lg("ERROR: ASSERTION FAILED!!! %s\n", #expr); else lg("PASS: %s\n", #expr)

static size_t rand_int(size_t a, size_t b) {
    static std::random_device rd;  // Non-deterministic random source
    static std::mt19937 gen(rd()); // Mersenne Twister PRNG seeded with rd()
    std::uniform_int_distribution<size_t> dist(a, b);
    return dist(gen);
}


#define lg printf
static inline void nl() {lg("\n");}
static inline void nl(size_t n) {for (;n;n--) lg("\n");}

#endif /* UTIL_H */
