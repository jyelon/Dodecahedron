// Fixed range numbers.
//
// We represent many things as numbers in the range 0-32768.  Things
// expressed on this scale include R,G,B, hue, saturation, value.
// We define 'typedef fixed' to store numbers in the range 0-32768,
// and the constant FIXMAX to mean 32768, and FIXHALF to mean 16384.
//
// Using this scale is very convenient: scaling and interpolation are
// very fast, roundoff error is minimal, and integer overflow is not
// an issue.
// 

typedef uint16_t fixed;
typedef int32_t fpixels;

const fixed FIXMAX = 32768;
const fixed FIXHALF = 16384;

fixed fixed_mul(fixed a, fixed b) {
    int32_t ia = a;
    int32_t ib = b;
    return (ia * ib) >> 15;
}

fixed fixed_lerp_fast(fixed a, fixed b, fixed t) {
    int32_t ia = a;
    int32_t ib = b;
    int32_t it = t;
    return (ib * it + ia * (FIXMAX - it)) >> 15;
}

fixed fixed_clamp(int32_t x) {
    if (x < 0) { return 0; }
    if (x > FIXMAX) { return FIXMAX; }
    return x;
}

fixed fixed_sqr(fixed x) {
    return (x * x) >> 15;
}

int32_t pixels_to_fpixels(int32_t pixels) {
    return pixels * 64;
}

int32_t fpixels_round_up(fpixels x) {
    return (x + 63) >> 6;
}

int32_t fpixels_round_down(fpixels x) {
    return x >> 6;
}

// This version of fixed_lerp can handle full-sized ints.
//
int32_t fixed_lerp(int32_t a, int32_t b, fixed t) {
    int32_t divisor = FIXMAX;
    while ((a > FIXMAX) || (b > FIXMAX) || (a < -FIXMAX) || (b < -FIXMAX)) {
        if (divisor == 1) break;
        a>>=1;
        b>>=1;
        divisor>>=1;
    }
    return (b * t + a * (FIXMAX - t)) / divisor;
}

// This version of fixed_unlerp can handle full-sized ints.
//
fixed fixed_unlerp(int32_t a, int32_t b, int32_t t) {
    int32_t n = (t - a);
    int32_t d = (b - a);
    if (d < 0) {
        d = -d;
        n = -n;
    }
    if (n <= 0) {
        return 0;
    }
    if (n >= d) {
        return FIXMAX;
    }
    while (n > FIXMAX) {
        n >>= 1;
        d >>= 1;
    }
    return n * FIXMAX / d;
}

int32_t clamp(int32_t lo, int32_t hi, int32_t value) {
    if (value < lo) return lo;
    if (value > hi) return hi;
    return value;
}

// Multiply and divide.  The only reason to use this over, say,
// just multiplying and dividing is that it converts your values
// to ints first.
//
int32_t muldiv(int32_t a, int32_t n, int32_t d) {
    return (a * n) / d;
}

int pick_one(int p0, int p1, int p2, int p3, int p4, int p5, int p6, int p7, int p8, int p9) {
    switch(random(10)) {
        case 0: return p0;
        case 1: return p1;
        case 2: return p2;
        case 3: return p3;
        case 4: return p4;
        case 5: return p5;
        case 6: return p6;
        case 7: return p7;
        case 8: return p8;
        case 9: return p9;
    }
}

// Splining routines
//
// Interpolate between a sequence of values.

fixed spline2(fixed i, fixed p0, fixed p1, fixed p2) {
    uint32_t ix = i * 2;
    uint32_t offset = ix & 0x7FFF;
    switch (ix >> 15) {
        case 0: return fixed_lerp_fast(p0, p1, offset);
        case 1: return fixed_lerp_fast(p1, p2, offset);
        default: return p2;
    }
}

fixed spline4(fixed i, fixed p0, fixed p1, fixed p2, fixed p3, fixed p4) {
    uint32_t ix = i * 4;
    uint32_t offset = ix & 0x7FFF;
    switch (ix >> 15) {
        case 0: return fixed_lerp_fast(p0, p1, offset);
        case 1: return fixed_lerp_fast(p1, p2, offset);
        case 2: return fixed_lerp_fast(p2, p3, offset);
        case 3: return fixed_lerp_fast(p3, p4, offset);
        default: return p4;
    }
}

fixed spline6(fixed i, fixed p0, fixed p1, fixed p2, fixed p3, fixed p4, fixed p5, fixed p6) {
    uint32_t ix = i * 6;
    uint32_t offset = ix & 0x7FFF;
    switch (ix >> 15) {
        case 0: return fixed_lerp_fast(p0, p1, offset);
        case 1: return fixed_lerp_fast(p1, p2, offset);
        case 2: return fixed_lerp_fast(p2, p3, offset);
        case 3: return fixed_lerp_fast(p3, p4, offset);
        case 4: return fixed_lerp_fast(p4, p5, offset);
        case 5: return fixed_lerp_fast(p5, p6, offset);
        default: return p6;
    }
}

fixed spline8(fixed i, fixed p0, fixed p1, fixed p2, fixed p3, fixed p4, fixed p5, fixed p6, fixed p7, fixed p8) {
    uint32_t ix = i * 8;
    uint32_t offset = ix & 0x7FFF;
    switch (ix >> 15) {
        case 0: return fixed_lerp_fast(p0, p1, offset);
        case 1: return fixed_lerp_fast(p1, p2, offset);
        case 2: return fixed_lerp_fast(p2, p3, offset);
        case 3: return fixed_lerp_fast(p3, p4, offset);
        case 4: return fixed_lerp_fast(p4, p5, offset);
        case 5: return fixed_lerp_fast(p5, p6, offset);
        case 6: return fixed_lerp_fast(p6, p7, offset);
        case 7: return fixed_lerp_fast(p7, p8, offset);
        default: return p8;
    }
}

fixed a_ramp(fixed i, int narrow) {
    uint32_t offset;
    if (i > 16384) {
        offset = i - 16384;
    } else {
        offset = 16384 - i;
    }
    offset = offset << narrow;
    if (offset > 16384) {
        return 0;
    }
    return 32768 - (offset << 1);
}

