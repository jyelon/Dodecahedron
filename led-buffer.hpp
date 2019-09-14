// These parameters describe the LED buffer.
//
#define LEDS_PER_HALF 15
#define LEDS_PER_EDGE (LEDS_PER_HALF*2)
#define EDGES_PER_STRAND 6
#define TOTAL_STRANDS 5
#define LEDS_PER_STRAND (LEDS_PER_EDGE*EDGES_PER_STRAND)
#define TOTAL_EDGES (EDGES_PER_STRAND*TOTAL_STRANDS)
#define TOTAL_LEDS (TOTAL_EDGES*LEDS_PER_EDGE)

// NeoPXL8 declaration.
//

int8_t neopxl8_pins[8] = { 0, 1, 7, 9, 10, 11, 12, 13 };
const uint32_t NEOPXL8_CHANNELS = NEO_BGR; // Use for GS8208
// const uint32_t NEOPXL8_CHANNELS = NEO_GRB; // Use for WS2818b
Adafruit_NeoPXL8 leds(LEDS_PER_STRAND, neopxl8_pins, NEOPXL8_CHANNELS);


// Just clear all the LEDS to black.
//

void clear_leds() {
    for (int i = 0; i < TOTAL_LEDS; i++) {
        leds.setPixelColor(i, 0);
    }
}

// Get offsets of pixels.
//
// These allow you to find the index of an LED given its coordinates.
//

int strand_edge_forward(int strand, int edge, int offset) {
    return (strand * LEDS_PER_STRAND) + (edge * LEDS_PER_EDGE) + offset;
}

int strand_edge_backward(int strand, int edge, int offset) {
    return (strand * LEDS_PER_STRAND) + (edge * LEDS_PER_EDGE) + (LEDS_PER_EDGE - offset - 1);
}

int strand_edge_middle_backward(int strand, int edge, int offset) {
    return (strand * LEDS_PER_STRAND) + (edge * LEDS_PER_EDGE) + (LEDS_PER_HALF - offset - 1);
}

int strand_edge_middle_forward(int strand, int edge, int offset) {
    return (strand * LEDS_PER_STRAND) + (edge * LEDS_PER_EDGE) + (LEDS_PER_HALF + offset);
}

int edge_forward(int edge, int offset) {
    return (edge * LEDS_PER_EDGE) + offset;
}

int edge_backward(int edge, int offset) {
    return (edge * LEDS_PER_EDGE) + (LEDS_PER_EDGE - offset - 1);
}

#define MAINCHAIN_LENGTH (5*LEDS_PER_EDGE)

int mainchain_forward(int strand, int offset) {
    return (strand * LEDS_PER_STRAND) + offset;
}

int mainchain_backward(int strand, int offset) {
    return (strand * LEDS_PER_STRAND) + (MAINCHAIN_LENGTH - offset - 1);
}        

// Store colors on the "waterfall" that goes from the top to the bottom.
//
// The following routine treats the dodecahedron as if it were a
// single line of pixels, running from the top to the bottom of the
// dodecahedron.  In reality, the "line" starts in the middle of every
// top-edge, and it works its way down to the middle of every bottom
// edge.
//

const int SPC_WATERFALL_LENGTH (LEDS_PER_HALF * 8);

void spc_waterfall(int i, uint32_t neocolor) {
    for (int strand = 0; strand < TOTAL_STRANDS; strand++) {
        int segment = i / LEDS_PER_HALF;
        int offset = i - (segment * LEDS_PER_HALF);
        switch (segment) {
        case 0:
            leds.setPixelColor(strand_edge_middle_forward(strand, 0, offset), neocolor);
            leds.setPixelColor(strand_edge_middle_backward(strand, 0, offset), neocolor);
            break;
        case 1:
            leds.setPixelColor(strand_edge_forward(strand, 1, offset), neocolor);
            break;
        case 2:
            leds.setPixelColor(strand_edge_middle_forward(strand, 1, offset), neocolor);
            break;
        case 3:
            leds.setPixelColor(strand_edge_forward(strand, 2, offset), neocolor);
            leds.setPixelColor(strand_edge_backward(strand, 5, offset), neocolor);
            break;
        case 4:
            leds.setPixelColor(strand_edge_middle_forward(strand, 2, offset), neocolor);
            leds.setPixelColor(strand_edge_middle_backward(strand, 5, offset), neocolor);
            break;
        case 5:
            leds.setPixelColor(strand_edge_forward(strand, 3, offset), neocolor);
            break;
        case 6:
            leds.setPixelColor(strand_edge_middle_forward(strand, 3, offset), neocolor);
            break;
        case 7:
            leds.setPixelColor(strand_edge_forward(strand, 4, offset), neocolor);
            leds.setPixelColor(strand_edge_backward(strand, 4, offset), neocolor);
            break;
        }
    }
}

// A tool to store the same values in all edges.
//
struct EdgeData {
    uint32_t data[LEDS_PER_EDGE];
    
    void write_all() {
        for (int i = 0; i < TOTAL_EDGES; i++) {
            for (int j = 0; j < LEDS_PER_EDGE; j++) {
                leds.setPixelColor(j + i * LEDS_PER_EDGE, data[j]);
            }
        }
    }
};
