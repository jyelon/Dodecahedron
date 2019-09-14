#define NEXUS_CLASSES 3
#define NEXUS_PHASES 8

struct NexusSpot {
    int class_index;
    int stage;
    int speed;
};

struct NexusClass {
    int hue;
    int quantity[NEXUS_PHASES];
    int active;
    int desire;
};

struct NexusEffect {
    NexusSpot spots_[TOTAL_LEDS];
    NexusClass classes_[NEXUS_CLASSES];
    int phase_color_[NEXUS_PHASES];
    int phase_intensity_[NEXUS_PHASES];
    int phase_speed_[NEXUS_PHASES];
    
    void initialize_show() {
        for (int i = 0; i < NEXUS_CLASSES; i++) {
            NexusClass &cls = classes_[i];
            cls.hue = random(FIXMAX);
            for (int i = 0; i < 7; i++) {
                cls.quantity[i] = FIXMAX;
            }
            cls.active = 0;
        }
        phase_intensity_[0] = 0;
        phase_intensity_[1] = 3000;
        phase_intensity_[2] = 5000;
        phase_intensity_[3] = 8000;
        phase_intensity_[4] = 10000;
        phase_intensity_[5] = 30000;
        phase_intensity_[6] = 3000;
        phase_intensity_[7] = 0;
        
        
        for (int i = 0; i < NEXUS_PHASES; i++) {
            for (int j = 0; j < NEXUS_CLASSES; j++) {
                classes_[j].quantity[i] = FIXMAX/20;
            }
            switch(random(2)) {
                case 0: break;
                case 1: phase_intensity_[i] /= 2; break;
                case 2: phase_intensity_[i] /= 4; break;
            }
            phase_color_[i] = random(NEXUS_CLASSES);
            classes_[phase_color_[i]].quantity[i] = FIXMAX;
            int speed_multiplier = random(100) + 50;
            phase_speed_[i] = fixed_lerp_fast(30, 100, phase_intensity_[i]);
            switch(random(2)) {
                case 0: break;
                case 1: phase_speed_[i] /= 2; break;
                case 2: phase_speed_[i] = phase_speed_[i] * 3 / 2; break;
            }
            Serial.printf("Phase %d speed=%d\n", i, phase_speed_[i]);
        }
        
        for (int i = 0; i < TOTAL_LEDS; i++) {
            NexusSpot &spot = spots_[i];
            spot.speed = 0;
            spot.stage = 0;
        }
    }

    NexusEffect() {
        initialize_show();
    }
    
    int pick_inactive_spot() {
        while (true) {
            int index = random(TOTAL_LEDS);
            if (spots_[index].speed == 0) {
                return index;
            }
        }
    }
    
    int total_active() {
        int result = 0;
        for (int i = 0; i < NEXUS_CLASSES; i++) {
            result += classes_[i].active;
        }
        return result;
    }
    
    void kill_finished_spots() {
        for (int i = 0; i < TOTAL_LEDS; i++) {
            NexusSpot &spot = spots_[i];
            if ((spot.speed > 0) && (spot.stage > FIXMAX)) {
                spot.speed = 0;
                classes_[spot.class_index].active --;
            }
        }
    }

    void start_new_spots(int age) {
        int phase = (age * (NEXUS_PHASES - 1)) / FIXMAX;
        fixed offset = (age * (NEXUS_PHASES - 1)) & 0x7FFF;
        offset = fixed_clamp(int(offset) * 3 / 2);
        fixed intensity0 = phase_intensity_[phase + 0];
        fixed intensity1 = phase_intensity_[phase + 1];
        fixed intensity = fixed_lerp_fast(intensity0, intensity1, offset);
        int speed0 = phase_speed_[phase + 0];
        int speed1 = phase_speed_[phase + 1];
        int speed = fixed_lerp_fast(speed0, speed1, offset);
        for (int i = 0; i < NEXUS_CLASSES; i++) {
            NexusClass &cls = classes_[i];
            fixed quantity0 = cls.quantity[phase + 0];
            fixed quantity1 = cls.quantity[phase + 1];
            fixed quantity = fixed_lerp_fast(quantity0, quantity1, offset);
            quantity = fixed_mul(quantity, intensity);
            cls.desire = (TOTAL_LEDS/2) * quantity / FIXMAX;
        }
        while (true) {
            // If too many spots are in use, we can't allocate more.
            if (total_active() > (TOTAL_LEDS / 2)) break;
            // Pick a random spot, and offer it to each class in turn.
            int spot_index = pick_inactive_spot();
            NexusSpot &spot = spots_[spot_index];
            int class_base = random(NEXUS_CLASSES);
            for (int class_offset = 0; class_offset < NEXUS_CLASSES; class_offset++) {
                int class_index = (class_base + class_offset) % NEXUS_CLASSES;
                NexusClass &cls = classes_[class_index];
                if (cls.active >= cls.desire) continue;
                spot.class_index = class_index;
                spot.speed = speed + random(speed);
                spot.stage = 0;
                cls.active += 1;
                break;
            }
            // If none of the classes wanted the spot, stop offering spots to classes.
            if (spot.speed == 0) break;
        }
    }
    
    bool update() {
        int age = fixed_clamp(show_age * 2);
        start_new_spots(age);
        clear_leds();
        for (int i = 0; i < TOTAL_LEDS; i++) {
            NexusSpot &spot = spots_[i];
            if (spot.speed == 0) continue;
            uint32_t ramp0 = a_ramp(spot.stage, 0);
            uint32_t ramp1 = a_ramp(spot.stage, 1);
            uint32_t ramp2 = a_ramp(spot.stage, 2);
            uint32_t pulse1 = fixed_sqr(ramp2);
            uint32_t pulse2 = fixed_sqr(pulse1);
            uint32_t bright = fixed_clamp((ramp0>>3) + (ramp1>>3) + pulse2);
            fixed sat = fixed_clamp(FIXMAX - (pulse1 * 2 / 3));
            fixed hue = classes_[spot.class_index].hue;
            RGB rgb = hue_sat(hue, sat).brighten().scale(bright);
            uint32_t neocolor = rgb.neocolor_unsafe();
            leds.setPixelColor(i, neocolor);
            spot.stage += spot.speed;
        }
        kill_finished_spots();
        
        return age < FIXMAX;
    }
};

