struct RugEffect {
    Rainbow rainbow_;
    fixed data_[TOTAL_LEDS];
    fixed next_[TOTAL_LEDS];
    int peak_aggressiveness_;
    int focal_edge_;
    
    RugEffect() {
        for (int x = 0; x < TOTAL_LEDS; x++) {
            data_[x] = 1000;
        }
        peak_aggressiveness_ = 5 << random(4);
        Serial.printf("Peak agg = %d\n", peak_aggressiveness_);
        int hue_gap = 4000 + random(8000);
        int hue1 = random(FIXMAX);
        int hue2 = (hue1 + hue_gap) & 0x7FFF;
        int hue3 = (hue1 + (hue_gap >> 1) + FIXHALF) & 0x7FFF;
        if (random(2) == 0) {
            int t=hue1; hue1=hue2; hue2=t;
        }
        RGB color1 = hue_sat(hue1, FIXMAX).brighten();
        RGB color2 = hue_sat(hue2, FIXMAX).brighten();
        RGB color3 = hue_sat(hue3, FIXMAX).brighten();
        switch(random(5)) {
        case 0: break; // No desaturation.
        case 1: color1 = RGB(FIXMAX, FIXMAX, FIXMAX); break;
        case 2: color2 = RGB(FIXMAX, FIXMAX, FIXMAX); break;
        case 3: color3 = RGB(FIXMAX, FIXMAX, FIXMAX); break;
        case 4:
            color1 = color1.desaturate(FIXHALF);
            color2 = color2.desaturate(FIXHALF);
            break;
        }
        focal_edge_ = (random(5) * EDGES_PER_STRAND) + 4;
           
        RGB black(0,0,0);
        rainbow_.clear();
        rainbow_.add_range(3, black, black);
        rainbow_.add_range(2, black, color1);
        rainbow_.add_range(random(4) + 1, color1, color1);
        rainbow_.add_range(random(10) + 1, color1, color2);
        rainbow_.add_range(random(4) + 1, color2, color2);
        if (random(2) == 0) {
            rainbow_.add_range(1, color2, color3);
        } else {
            rainbow_.add_range(1, color2, black);
            rainbow_.add_range(1, black, color3);
        }
        rainbow_.add_range(random(3) + 1, color3, color3);
        rainbow_.add_range(1, color3, black);
    }
    
    bool update() {
        int age = fixed_clamp(show_age * 2);
        fixed agg_ramp = spline8(age, 0, FIXMAX>>5, FIXMAX>>4, FIXMAX>>3, FIXMAX>>2, FIXMAX>>1, FIXMAX, FIXMAX>>2, 0);
        int aggressiveness = 3 + fixed_mul(peak_aggressiveness_, agg_ramp);
        int forcing =        spline8(age,  20, 10,   5,   2,   0,   0,   0,  0, 0);
        int fade_black =     spline8(age,  0, FIXMAX, FIXMAX, FIXMAX, FIXMAX, FIXMAX, FIXMAX, FIXMAX, 0);
        for (int edge = 0; edge < TOTAL_EDGES; edge++) {
            for (int offset = 0; offset < LEDS_PER_EDGE; offset++) {
                AdjacentLEDs adj(edge, offset);
                int self_index = edge_forward(edge, offset);
                int self_data = data_[self_index];
                int average;
                if (adj.have_three()) {
                    average = data_[adj.led[0]] + data_[adj.led[1]] + data_[adj.led[2]] + self_data;
                } else {
                    average = data_[adj.led[0]] + data_[adj.led[1]] + self_data + self_data;
                }
                average += self_data * 4;
                average = average >> 3;
                next_[self_index] = (average - aggressiveness) & 0x7FFF;
            }
        }
        DirectedEdge hotspot(focal_edge_, 0);
        for (int i = 0; i < 5; i ++) {
            next_[hotspot.offset(14)] -= forcing;
            next_[hotspot.offset(15)] -= forcing;
            hotspot = hotspot.successor(false);
        }
                
        for (int x = 0; x < TOTAL_LEDS; x++) {
            data_[x] = next_[x];
            int rain = (data_[x] << 2) & 0x7FFF;
            RGB rgb = rainbow_.get(rain);
            uint32_t neocolor = rgb.scale(fade_black).neocolor_unsafe();
            leds.setPixelColor(x, neocolor);
        }

        return (age < FIXMAX);
    }
};
