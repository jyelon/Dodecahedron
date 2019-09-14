#define MAXCOMETS 300

struct Comet {
    fixed edge;
    bool backward;
    fixed countdown; // Time until this starts running.
    fixed travel; // A number from 0 to FIXMAX.
    fixed speed; // If speed is zero, this comet is off.
    fixed size; // In LEDS
    fixed hue;
};

struct CometEffect {
    Comet comets_[MAXCOMETS];
    fixed decay_[TOTAL_LEDS];
    RGB color_[TOTAL_LEDS];
    int active_comets_;
    // These parameters persist for an entire show.
    int hue_base_;
    int hue_range_;
    int decay_multiplier_;
    int speed_multiplier_;
    int peak_comets_;
    
    void initialize_show() {
        hue_base_ = random(FIXMAX);
        hue_range_ = 2000 + random(10000);
        decay_multiplier_ = pick_one(20, 40, 80, 90, 100, 100, 100, 100, 110, 150);
        speed_multiplier_ = pick_one(40, 60, 80, 100, 100, 100, 100, 120, 150, 200);
        peak_comets_ = pick_one(30, 30, 40, 40, 50, 50, 60, 60, 150, 300);

        for (int i = 0; i < TOTAL_LEDS; i++) {
            color_[i] = RGB(0,0,0);
            decay_[i] = 150 + random(300);
            switch(random(3)) {
                case 0: break;
                case 1: decay_[i] *= 3; break;
                case 2: decay_[i] *= 9; break;
            }
        }
        active_comets_ = 0;
        for (int i = 0; i < MAXCOMETS; i++) {
            comets_[i].speed = 0;
        }
    }
    
    CometEffect() {
        initialize_show();
    }
        
    void kill_finished_comets() {
        for (int i = 0; i < MAXCOMETS; i++) {
            Comet &comet = comets_[i];
            if ((comet.speed > 0) && (comet.travel > FIXMAX)) {
                comet.speed = 0;
                active_comets_--;
            }
        }
    }

    void start_new_comets(int desired_comets, int move_speed) {
        for (int i = 0; i < MAXCOMETS; i++) {
            Comet &comet = comets_[i];
            if ((comet.speed == 0) && (active_comets_ < desired_comets)) {
                comet.edge = random(TOTAL_EDGES);
                comet.backward = (random(2) == 0);
                comet.travel = 0;
                comet.speed = move_speed + random(move_speed * 3);
                comet.size = 5 + random(5);
                comet.hue = (hue_base_ + random(hue_range_)) & 0x7FFF;
                comet.countdown = random(1000);
                active_comets_++;
            }
        }
    }
        
    bool update() {   
        int age = fixed_clamp(show_age * 2);

        int desired_comets = spline8(age,   2,   2,   3,   5,  10,  peak_comets_/2, peak_comets_,  2, 0);
        int move_speed =     spline8(age,  70,  85, 100, 150, 200, 250, 300, 70, 0);
        int decay_speed = 50 + (desired_comets / 2);
        decay_speed = decay_speed * decay_multiplier_ / 100;
        move_speed = move_speed * speed_multiplier_ / 100;
        
        start_new_comets(desired_comets, move_speed);
        
        for (int i = 0; i < TOTAL_LEDS; i++) {
            const int fixdecay = 10;
            int decay = decay_[i] * decay_speed / 100;
            color_[i] = color_[i].scale(FIXMAX - decay).sub(RGB(fixdecay, fixdecay, fixdecay));
        }
        
        for (int comet_index = 0; comet_index < MAXCOMETS; comet_index++) {
            Comet &comet = comets_[comet_index];
            if (comet.speed == 0) continue;
            if (comet.countdown > 0) {
                comet.countdown--;
                continue;
            }
            RGB color = hue_sat(comet.hue, FIXMAX);
            // Comet path constants
            const int comet_length_fpixels = pixels_to_fpixels(comet.size);
            const int travel_start_fpixels = -comet_length_fpixels;
            const int travel_end_fpixels = pixels_to_fpixels(LEDS_PER_EDGE);
            // These positions are specified in fpixels ("fractional pixels")
            fpixels comet_fpixels_lo = fixed_lerp(travel_start_fpixels, travel_end_fpixels, comet.travel);
            fpixels comet_fpixels_hi = comet_fpixels_lo + comet_length_fpixels;
            int comet_pixels_lo = clamp(0, LEDS_PER_EDGE-1, fpixels_round_up(comet_fpixels_lo));
            int comet_pixels_hi = clamp(0, LEDS_PER_EDGE-1, fpixels_round_down(comet_fpixels_hi));
            for (int i = comet_pixels_lo; i <= comet_pixels_hi; i++) {
                fixed offset = fixed_unlerp(comet_fpixels_lo, comet_fpixels_hi, pixels_to_fpixels(i));
                int hotness = spline4(offset, 0, FIXMAX*1/3, FIXMAX*2/3, FIXMAX*3/3, 0);
                int index;
                if (comet.backward) {
                    index = edge_backward(comet.edge, i);
                } else {
                    index = edge_forward(comet.edge, i);
                }
                color_[index] = color_[index].maxv(color.scale(hotness));
            }
            // Advance the comet.
            comet.travel += comet.speed;
        }
        kill_finished_comets();

        RGB white(FIXMAX, FIXMAX, FIXMAX);
        for (int i = 0; i < TOTAL_LEDS; i++) {
            int total = int(color_[i].R) + int(color_[i].G) + int(color_[i].B);
            total = fixed_clamp(total - 4000);
            RGB color = color_[i].lerp(white, total);
            leds.setPixelColor(i, color.neocolor_unsafe());
        }
        
        return age < FIXMAX;
    }
};
