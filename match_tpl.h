
#include "zbuild.h"
#include "deflate.h"
#include "functable.h"

#ifndef BESTCMP_TYPE
#define BESTCMP_TYPE

#ifdef UNALIGNED_OK
#  ifdef UNALIGNED64_OK
typedef uint64_t        bestcmp_t;
#  else
typedef uint32_t        bestcmp_t;
#  endif
#else
typedef uint8_t         bestcmp_t;
#endif

#endif

/*
 * Set match_start to the longest match starting at the given string and
 * return its length. Matches shorter or equal to prev_length are discarded,
 * in which case the result is equal to prev_length and match_start is garbage.
 *
 * IN assertions: cur_match is the head of the hash chain for the current
 * string (strstart) and its distance is <= MAX_DIST, and prev_length >=1
 * OUT assertion: the match length is not greater than s->lookahead
 */
ZLIB_INTERNAL int32_t LONGEST_MATCH(deflate_state *const s, Pos cur_match) {
    unsigned int strstart = s->strstart;
    const unsigned wmask = s->w_mask;
    ZLIB_REGISTER unsigned char *window = s->window;
    ZLIB_REGISTER unsigned char *scan = window + strstart;
    const Pos *prev = s->prev;
    Pos limit;
    uint32_t chain_length, nice_match, best_len, offset;
    bestcmp_t scan_end;
#ifndef UNALIGNED_OK
    bestcmp_t scan_end0;
#else
    bestcmp_t scan_start;
#endif

#define EARLY_EXIT_TRIGGER_LEVEL 5

#define RETURN_BEST_LEN \
    if (best_len <= s->lookahead) \
        return best_len; \
    return s->lookahead;

#define GOTO_NEXT_CHAIN \
    if (--chain_length && (cur_match = prev[cur_match & wmask]) > limit) \
        continue; \
    RETURN_BEST_LEN;

    /*
     * The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple
     * of 16. It is easy to get rid of this optimization if necessary.
     */
    Assert(s->hash_bits >= 8 && MAX_MATCH == 258, "Code too clever");

    best_len = s->prev_length ? s->prev_length : 1;

    /* 
     * Calculate read offset which should only extend an extra byte 
     * to find the next best match length.
     */
    offset = best_len-1;
#ifdef UNALIGNED_OK
    if (best_len >= sizeof(uint32_t)) {
        offset -= 2;
#ifdef UNALIGNED64_OK
        if (best_len >= sizeof(uint64_t))
            offset -= 4;
#endif
    }
#endif

    scan_end   = *(bestcmp_t *)(scan+offset);
#ifndef UNALIGNED_OK
    scan_end0  = *(bestcmp_t *)(scan+offset+1);
#else
    scan_start = *(bestcmp_t *)(scan);
#endif

    /*
     * Do not waste too much time if we already have a good match
     */
    chain_length = s->max_chain_length;
    if (best_len >= s->good_match)
        chain_length >>= 2;

    /*
     * Do not look for matches beyond the end of the input. This is
     * necessary to make deflate deterministic
     */
    nice_match = (uint32_t)s->nice_match > s->lookahead ? s->lookahead : (uint32_t)s->nice_match;

    /*
     * Stop when cur_match becomes <= limit. To simplify the code,
     * we prevent matches with the string of window index 0
     */
    limit = strstart > MAX_DIST(s) ? (Pos)(strstart - MAX_DIST(s)) : 0;

    Assert((unsigned long)strstart <= s->window_size - MIN_LOOKAHEAD, "need lookahead");
    for (;;) {
        ZLIB_REGISTER unsigned char *match;
        ZLIB_REGISTER unsigned int len;
        if (cur_match >= strstart)
            break;

        /*
         * Skip to next match if the match length cannot increase
         * or if the match length is less than 2. Note that the checks
         * below for insufficient lookahead only occur occasionally
         * for performance reasons. Therefore uninitialized memory
         * will be accessed and conditional jumps will be made that
         * depend on those values. However the length of the match
         * is limited to the lookahead, so the output of deflate is not
         * affected by the uninitialized values.
         */
#ifdef UNALIGNED_OK
        if (best_len < sizeof(uint32_t)) {
            for (;;) {
                match = window + cur_match;
                if (UNLIKELY(*(uint16_t *)(match+offset) == (uint16_t)scan_end &&
                             *(uint16_t *)(match) == (uint16_t)scan_start))
                    break;
                GOTO_NEXT_CHAIN;
            }
#  ifdef UNALIGNED64_OK
        } else if (best_len >= sizeof(uint64_t)) {
            for (;;) {
                match = window + cur_match;
                if (UNLIKELY(*(uint64_t *)(match+offset) == (uint64_t)scan_end &&
                             *(uint64_t *)(match) == (uint64_t)scan_start))
                    break;
                GOTO_NEXT_CHAIN;
            }
#  endif
        } else {
            for (;;) {
                match = window + cur_match;
                if (UNLIKELY(*(uint32_t *)(match+offset) == (uint32_t)scan_end &&
                             *(uint32_t *)(match) == (uint32_t)scan_start))
                    break;
                GOTO_NEXT_CHAIN;
            }
        }
#else
        for (;;) {
            match = window + cur_match;
            if (UNLIKELY(match[offset] == scan_end && match[offset+1] == scan_end0 &&
                         match[0] == scan[0] && match[1] == scan[1]))
                break;
            GOTO_NEXT_CHAIN;
        }
#endif
        len = COMPARE256(scan+2, match+2) + 2;
        Assert(scan+len <= window+(unsigned)(s->window_size-1), "wild scan");

        if (len > best_len) {
            s->match_start = cur_match;
            best_len = len;
            if (best_len >= nice_match)
                break;
            offset = best_len-1;
#ifdef UNALIGNED_OK
            if (best_len >= sizeof(uint32_t)) {
                offset -= 2;
#ifdef UNALIGNED64_OK
                if (best_len >= sizeof(uint64_t))
                    offset -= 4;
#endif
            }
#endif
            scan_end = *(bestcmp_t *)(scan+offset);
#ifndef UNALIGNED_OK
            scan_end0 = *(bestcmp_t *)(scan+offset+1);
#endif
        } else {
            /*
             * The probability of finding a match later if we here
             * is pretty low, so for performance it's best to
             * outright stop here for the lower compression levels
             */
            if (s->level < EARLY_EXIT_TRIGGER_LEVEL)
                break;
        }
        GOTO_NEXT_CHAIN;
    }

    RETURN_BEST_LEN;
}

#undef LONGEST_MATCH
#undef COMPARE256
#undef COMPARE258
