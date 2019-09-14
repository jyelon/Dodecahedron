

#define ZIPPY_CARS 100
#define ZIPPY_LOOKAHEAD 3

struct ZippyCar {
    DirectedEdge edge;
    fpixels position;
    bool plan_left[3];
    int speed;
};

struct ZippyCarEffect {
    RGB color_[TOTAL_LEDS];
    ZippyCar car_[ZIPPY_CARS];
    int active_cars_;
    int background_phase_;
    
    ZippyCarEffect() {
        for (int i = 0; i < TOTAL_LEDS; i++) {
            color_[i] = RGB(0,0,0);
        }
        active_cars_ = 0;
        background_phase_ = 0;
    }
    
    void kill_one_car() {
        if (active_cars_ > 0) {
            int index = random(active_cars_);
            car_[index] = car_[active_cars_ - 1];
            active_cars_ -= 1;
        }
    }
    
    void start_new_car() {
        if (active_cars_ < ZIPPY_CARS) {
            ZippyCar p;
            p.edge.edge = random(TOTAL_EDGES);
            p.edge.backward = (random(2) == 0);
            p.position = 0;
            for (int i = 0; i < ZIPPY_LOOKAHEAD; i++) {
                p.plan_left[i] = (random(2) == 0);
            }
            p.speed = random(25) + 10;
            car_[active_cars_] = p;
            active_cars_ ++;
        }
    }
    
    void draw_car(const DirectedEdge &edge, fixed intensity, int start_fpixels, int end_fpixels) {
        int px_lo = clamp(0, LEDS_PER_EDGE - 1, fpixels_round_up(start_fpixels));
        int px_hi = clamp(0, LEDS_PER_EDGE - 1, fpixels_round_down(end_fpixels));
        for (int px = px_lo; px <= px_hi; px++) {
            fixed offset = fixed_unlerp(start_fpixels, end_fpixels, pixels_to_fpixels(px));
            fixed brite = spline8(offset, 0, FIXMAX * 3 / 8, FIXMAX * 5 / 8, FIXMAX * 6 / 8, FIXMAX * 7 / 8, FIXMAX, FIXMAX, FIXMAX, 0);
            brite = fixed_mul(brite, intensity);
            RGB acolor(brite, brite, brite);
            int index = edge.offset(px);
            color_[index] = color_[index].add(acolor);
        }
    }
    
    bool update() {
        int age = fixed_clamp(show_age * 2);
        int edge_len_fpixels = pixels_to_fpixels(LEDS_PER_EDGE);
        int car_intensity = spline8(age, 0, FIXMAX/5, FIXMAX/7, FIXMAX/10, FIXMAX/7, FIXMAX/2, FIXMAX/3, FIXMAX/2, FIXMAX);
        int car_length_fpixels = spline8(age, 80*64, 80*64, 40*64, 20*64, 80*64, 80*64, 80*64, 40*64, 20*64);
        int car_speed_mult = spline8(age, 100, 100, 50, 300, 200, 100, 30, 150, 200);
        int fade_black = spline8(age,  0, FIXMAX, FIXMAX, FIXMAX, FIXMAX, FIXMAX, FIXMAX, FIXMAX, 0);

        int car_count = 3;
        while (active_cars_ < car_count) start_new_car();
        while (active_cars_ > car_count) kill_one_car();

        background_phase_ += 20;
        for (int edge = 0; edge < TOTAL_EDGES; edge++) {
            fixed hue = ((edge * 5000) + background_phase_) & (FIXMAX - 1);
            RGB color = hue_sat(hue, FIXMAX).brighten();
            for (int i = 0; i < LEDS_PER_EDGE; i++) {
                int index = edge_forward(edge, i);
                color_[index] = color;
            }
        }

        for (int i = 0; i < active_cars_; i++) {
            ZippyCar &car = car_[i];
            DirectedEdge edge = car.edge;
            int start_fpixels = car.position;
            int end_fpixels = start_fpixels + car_length_fpixels;
            int next_plan_step = 0;
            while (true) {
                draw_car(edge, car_intensity, start_fpixels, end_fpixels);
                if (end_fpixels < edge_len_fpixels) break;
                if (next_plan_step == ZIPPY_LOOKAHEAD) break;
                start_fpixels -= edge_len_fpixels;
                end_fpixels -= edge_len_fpixels;
                edge = edge.successor(car.plan_left[next_plan_step]);
                next_plan_step++;
            }
            car.position += (car.speed * car_speed_mult / 100);
            if (car.position >= edge_len_fpixels) {
                car.position -= edge_len_fpixels;
                car.edge = car.edge.successor(car.plan_left[0]);
                for (int la = 1; la < ZIPPY_LOOKAHEAD; la++) {
                    car.plan_left[la - 1] = car.plan_left[la];
                }
                car.plan_left[ZIPPY_LOOKAHEAD - 1] = (random(2) == 0);
            }
        }
        for (int i = 0; i < TOTAL_LEDS; i++) {
            leds.setPixelColor(i, color_[i].scale(fade_black).neocolor_unsafe());
        }
        return age < FIXMAX;
    }
};

struct BurstEffect {
    EdgeData edge_;
    fixed base_hue_;
    fixed hue_range_;
    
    BurstEffect(fixed base_hue, fixed hue_range) :
        base_hue_(base_hue), hue_range_(hue_range) {
        }
        
    void update() {
        for (int i = 0; i < LEDS_PER_HALF; i++) {
            uint32_t index1 = (show_age * 200 + i * 1200);
            uint32_t bright1 = spline8(index1 & 0x7FFF, 0, FIXMAX/3, FIXMAX, FIXMAX/2, FIXMAX/4, FIXMAX/8, FIXMAX/12, FIXMAX/16, 0);
            uint32_t index2 = (show_age * 931 + i * 7000);
            uint32_t bright2 = spline4(index2 & 0x7FFF, 0, FIXMAX/4, FIXMAX, FIXMAX/4, 0);
            fixed bright = fixed_clamp(bright1 + (bright2/8));
            fixed sat = 32768 - (bright2 >> 2);
            fixed hue_offset = spline2((show_age * 15) & 0x7FFF, 0, hue_range_, 0);
            fixed hue = (base_hue_ + hue_offset + (bright1 >> 2)) & 0x7FFF;
            RGB rgb = hue_sat(hue, sat).scale(bright);
            uint32_t neocolor = rgb.neocolor_unsafe();
            edge_.data[i] = neocolor;
            edge_.data[LEDS_PER_EDGE - 1 - i] = neocolor;
        }
        edge_.data[0] = 0;
        edge_.data[LEDS_PER_EDGE - 1] = 0;
        edge_.write_all();
    }
};


struct WaterFallEffect {
    WaterFallEffect() {
    }
        
    bool update() {
        int age = show_age * 2;
        int ripple_brightness = spline8(age, 12000, 12000, 8000, 0, 20000, 2000, 0, 8000, 12000);
        int fade_black =     spline8(age,  0, FIXMAX, FIXMAX, FIXMAX, FIXMAX, FIXMAX, FIXMAX, FIXMAX, 0);
        int rainbow_pack = spline8(age, FIXMAX/8, FIXMAX/8, FIXMAX/2, FIXMAX/8, FIXMAX/8, FIXMAX/3, FIXMAX, FIXMAX/8, FIXMAX/8);
        int ripple_desat = spline6(age, FIXHALF, FIXHALF/2, 0, 0, FIXHALF, FIXHALF, 0);
        for (int i = 0; i < SPC_WATERFALL_LENGTH; i++) {
            int boffset = ((show_age * 150) + (i * 4500)) & 0x7FFF;
            int brite = spline2(boffset, ripple_brightness, 32768, ripple_brightness);
            int pack = fixed_mul(800, rainbow_pack);
            int offset = ((show_age * 20) + (i * pack)) & 0x7FFF;
            RGB rgb = hue_sat(offset, FIXMAX).brighten().scale(brite);
            RGB white(FIXMAX, FIXMAX, FIXMAX);
            int soffset = ((show_age * 13) + (i * 1023)) & 0x7FFF;
            int saturation = spline2(soffset, ripple_desat, FIXMAX, ripple_desat);
            rgb = white.lerp(rgb, saturation);
            rgb = rgb.scale(fade_black);
            spc_waterfall(i, rgb.neocolor_unsafe());
        }
        return age < FIXMAX;
    }
};

struct FullWhiteEffect {

    FullWhiteEffect() {
    }
        
    void update() {        
        RGB rgb(FIXMAX, FIXMAX, FIXMAX);
        for (int x = 0; x < TOTAL_LEDS; x++) {
            uint32_t neocolor = rgb.neocolor_unsafe();
            leds.setPixelColor(x, neocolor);
        }
    }
};

struct BoundariesEffect {
    BoundariesEffect() {
    }
    
    void update() {
        for (int edge = 0; edge < TOTAL_EDGES; edge++) {
            fixed hue = ((edge * 5000) + (show_age * 20)) & (FIXMAX - 1);
            uint32_t color1 = hue_sat(hue, FIXMAX).neocolor_unsafe();
            uint32_t color2 = hue_sat(hue, 5000).neocolor_unsafe();
            int offset = (edge * LEDS_PER_EDGE);
            int last = (LEDS_PER_EDGE - 1);
            for (int i = 0; i < LEDS_PER_EDGE; i++) {
                leds.setPixelColor(i + offset, color1);
            }
            leds.setPixelColor(0 + offset, color2);
            leds.setPixelColor(last + offset, color2);
        }
    }
};


struct LittleCarEffect {
    DirectedEdge edge_;
    int offset_;
    bool next_left_;
    
    LittleCarEffect() {
        edge_ = DirectedEdge(0, false);
        offset_ = 0;
        next_left_ = false;
    }
    bool update() {
        uint32_t red = RGB(FIXMAX, 0, 0).neocolor_unsafe();
        uint32_t blue = RGB(0, 0, FIXMAX).neocolor_unsafe();
        uint32_t white = RGB(FIXMAX, FIXMAX, FIXMAX).neocolor_unsafe();
        
        if (show_age*2 > FIXMAX) return false;
        if ((show_age & 0x3F) == 0) {
            if (offset_ == LEDS_PER_EDGE - 1) {
                DirectedEdge next = edge_.successor(next_left_);
                offset_ = 0;
                edge_ = next;
                next_left_ = (random(2) == 0);
            } else {
                offset_ ++;
            }
        }
        clear_leds();
        int index = edge_.offset(offset_);
        if (offset_ < LEDS_PER_HALF) {
            leds.setPixelColor(index, white);
        } else {
            if (next_left_) {
                leds.setPixelColor(index, red);
            } else {
                leds.setPixelColor(index, blue);
            }
        }
        return true;
    }
};




