

// Make a random seed using analog noise on three analog pins.
//
uint32_t seed_from_analog_noise(int pin0, int pin1, int pin2) {
    uint32_t a, b, c;
    a = 0;
    b = 0;
    c = 0;
    for (int i = 0; i < 1000; i++) {
        a ^= analogRead(pin0);
        b ^= analogRead(pin1);
        c ^= analogRead(pin2);
        // This is bob jenkins' old mixing function.
        a -= b; a -= c; a ^= (c>>13);
        b -= c; b -= a; b ^= (a<<8);
        c -= a; c -= b; c ^= (b>>13);
        a -= b; a -= c; a ^= (c>>12);
        b -= c; b -= a; b ^= (a<<16);
        c -= a; c -= b; c ^= (b>>5);
        a -= b; a -= c; a ^= (c>>3);
        b -= c; b -= a; b ^= (a<<10);
        c -= a; c -= b; c ^= (b>>15);
    }
    return a;
}
