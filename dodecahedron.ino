// WARNING ABOUT POWER CONSUMPTION
//
// The circuitry driving our LED strips will overheat if you set the LEDs above
// 1/2 brightness.  The circuit has fuses to prevent disaster.  To avoid blowing
// the fuses, we have to restrict power consumption in software.  To have a little
// safety margin, this code limits power consumption to 1/3 of the theoretical maximum.
//
// Each individual effect must be designed to limit power consumption to 1/3!  One way
// to accomplish this is to turn on fewer than 1/3 of the LEDs.  The other is to limit
// the brightness of each individual LED to 1/3.  We provide libraries to make either
// approach easy.
//

#include <Adafruit_NeoPXL8.h>
#include <Bounce2.h>

#include "basic-math.hpp"
#include "colors.hpp"
#include "pool-alloc.hpp"
#include "led-buffer.hpp"
#include "vector.hpp"
#include "geometry.hpp"
#include "random-seeding.hpp"

// Show management.
//
PoolAlloc pool;
uint32_t show_age = 0;
uint32_t show_counter = 0;
void *show_effect = NULL;
uint32_t clicks = 0;

#include "nexus-effect.hpp"
#include "comet-effect.hpp"
#include "rug-effect.hpp"
#include "half-baked-effects.hpp"

// The pushbutton is connected to pin 4.  We use the debouncing library
// to read the pushbutton.
//
const int BUTTON_PIN = 4;
Bounce debouncer = Bounce();


// Initial setup.
//
void setup() {
    Serial.begin(9600);
    delay(5000);
    populate_successor_edges();
    leds.begin();
    leds.setBrightness(255);
    debouncer.attach(BUTTON_PIN, INPUT_PULLUP); // Attach the debouncer to a pin with INPUT_PULLUP mode
    debouncer.interval(25); // Use a debounce interval of 25 milliseconds
    randomSeed(seed_from_analog_noise(A0, A1, A5));
    Serial.printf("Starting up.\n");
}

bool update_show() {
    switch(show_counter) {
    case 0:
        if (show_effect == NULL) {
            show_effect = new(pool) ZippyCarEffect();
        }
        return ((ZippyCarEffect *)show_effect)->update();
    case 1:
        if (show_effect == NULL) {
            show_effect = new(pool) NexusEffect();
        }
        return ((NexusEffect *)show_effect)->update();
    case 2:
        if (show_effect == NULL) {
            show_effect = new(pool) WaterFallEffect();
        }
        return ((WaterFallEffect *)show_effect)->update();

    case 3:
        if (show_effect == NULL) {
            show_effect = new(pool) CometEffect();
        }
        return ((CometEffect*)show_effect)->update();
    case 4:
        if (show_effect == NULL) {
            show_effect = new(pool) RugEffect();
        }
        return ((RugEffect *)show_effect)->update();
    default:
        return false;
    }
}

//     if (effect == NULL) {
//          effect = new(pool) BurstEffect(20000, 9000);
//     }
//     ((BurstEffect *)effect)->update();

//     if (effect == NULL) {
//         Rainbow *rainbow = new(pool) Rainbow(Rainbow::BOW_STANDARD_RAINBOW);
//         effect = new(pool) RugOneEffect(rainbow);
//     }
//     ((RugOneEffect *)effect)->update();

//     if (effect == NULL) {
//         effect = new(pool) BoundariesEffect();
//     }
//     ((BoundariesEffect *)effect)->update();

//     if (effect == NULL) {
//         Rainbow *rainbow = new(pool) Rainbow(Rainbow::BOW_STANDARD_RAINBOW);
//         effect = new(pool) WaterFallEffect(rainbow);
//     }
//     ((WaterFallEffect*)effect)->update();
    
//     if (effect == NULL) {
//         effect = new(pool) BrightnessEffect(FIXMAX);
//     }
//     ((BrightnessEffect*)effect)->update();

//     if (effect == NULL) {
//         Rainbow *rainbow = new(pool) Rainbow("R Y, Y G, G C, C B, B M, M R");
//         effect = new(pool) StaticRainbowEffect(rainbow);
//     }
//     ((StaticRainbowEffect*)effect)->update();

//     if (effect == NULL) {
//          effect = new(pool) FullWhiteEffect();
//     }
//     ((FullWhiteEffect *)effect)->update();


void loop() {
    show_age++;
    // Keep incrementing the show counter until you succeed
    // in starting an effect.
    while (true) {
        debouncer.update();
        bool running = update_show();
        bool dbf = debouncer.fell();
        if (dbf) Serial.printf("Debouncer fell.\n");
        if (running && !dbf) break;
        pool.clear();
        show_effect = NULL;
        show_age = 0;
        show_counter = (show_counter + 1) & 255;
    }
    leds.show();
}


