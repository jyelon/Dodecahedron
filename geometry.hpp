// Dodecahedron Vertices
//
// The following table "dodecahedron_vertices" reports the spatial
// position of the vertices, and also effectively assigns an order
// to the vertices.  The vertex ordering defined by this table is
// used consistently throughout this program as the "canonical" way to
// order the vertices of a dodecahedron.
//
// If you put a dodecahedron on a table, you can imagine that the top
// pentagon is the "north pole" and the bottom, touching the table,
// is the "south pole."  You can then draw "latitude lines" around the
// dodecahedron.
//
// The vertices of the dodecahedron touch exactly four of these latitude
// lines: there's a ring of five vertices near the south pole, a ring
// of five just below the equator, a ring of five just slightly above
// the equator, and a ring of five near the north pole.
//
// The lookup table below lists the spatial positions of the vertices.
// It is organized into four rings consisting of five vertices each.
// The four rings start near the south pole and work their way northward.
// Each ring is clockwise around the dodecahedron.  The first two rings
// start at longitude zero, the next two start at longitude 36 degrees.
//
// Coordinates range from -16384 to 16384.
//

Vector dodecahedron_vertex[20] = {
    {     0,-10125,-13254}, { -9630, -3129,-13254}, { -5951,  8192,-13254}, {  5951,  8192,-13254}, {  9630, -3129,-13254},
    {     0,-16384, -3129}, {-15582, -5062, -3129}, { -9630, 13254, -3129}, {  9630, 13254, -3129}, { 15582, -5062, -3129},
    { -9630,-13254,  3129}, {-15582,  5062,  3129}, {     0, 16384,  3129}, { 15582,  5062,  3129}, {  9630,-13254,  3129},
    { -5951, -8192, 13254}, { -9630,  3129, 13254}, {     0, 10125, 13254}, {  9630,  3129, 13254}, {  5951, -8192, 13254},
};

// Dodecahedron Edges
//
// The following table lists the edges in the dodecahedron.  It
// also effectively assigns an order to the edges.  The edge ordering
// defined by this table is used consistently throughout this program
// as the "canonical" way to order the edges of a dodecahedron.
//
// The order in which these edges appear in this table is the same order
// in which they are physically wired.  In other words, the edges appear
// in this table in the exact same order that they appear
// in the neopxl8 buffer.
//

struct DodecahedronEdge {
    int vertex1;
    int vertex2;
};

DodecahedronEdge dodecahedron_edge[] = {
    { 0,  1}, { 1,  6}, { 6, 11}, {11, 16}, {16, 17}, {11,  7},
    { 1,  2}, { 2,  7}, { 7, 12}, {12, 17}, {17, 18}, {12,  8},
    { 2,  3}, { 3,  8}, { 8, 13}, {13, 18}, {18, 19}, {13,  9},
    { 3,  4}, { 4,  9}, { 9, 14}, {14, 19}, {19, 15}, {14,  5},
    { 4,  0}, { 0,  5}, { 5, 10}, {10, 15}, {15, 16}, {10,  6},
};

Vector dod_get_edge_vertex1(int edge) {
    return dodecahedron_vertex[dodecahedron_edge[edge].vertex1];
}

Vector dod_get_edge_vertex2(int edge) {
    return dodecahedron_vertex[dodecahedron_edge[edge].vertex2];
}

// DirectedEdge
//
// Stores an edge and a direction.
// 

struct DirectedEdge {
    int edge;
    bool backward;
    
    // Constructors.
    DirectedEdge() : edge(-1), backward(false) {}
    DirectedEdge(int e, bool b) : edge(e), backward(b) {}
    
    // Return the LED index of the Nth LED.
    int offset(int i) const {
        if (backward) {
            return edge_backward(edge, i);
        } else {
            return edge_forward(edge, i);
        }
    }
    
    // Return the opposite of this DirectedEdge.
    DirectedEdge uturn() const {
        return DirectedEdge(edge, !backward);
    }
    
    // Get the vertices at the start and end of the edge.
    Vector vertex1() const {
        if (backward) {
            return dodecahedron_vertex[dodecahedron_edge[edge].vertex2];
        } else {
            return dodecahedron_vertex[dodecahedron_edge[edge].vertex1];
        }
    }
    Vector vertex2() const {
        if (backward) {
            return dodecahedron_vertex[dodecahedron_edge[edge].vertex1];
        } else {
            return dodecahedron_vertex[dodecahedron_edge[edge].vertex2];
        }
    }
    
    // Return the forward and backward vectors.
    Vector forward_vector() const {
        return vertex2().sub(vertex1());
    }
    Vector backward_vector() const {
        return vertex1().sub(vertex2());
    }
    
    // Return one successor of this DirectedEdge.
    DirectedEdge successor(bool left) const;
};

// Successor edge table.
//

struct SuccessorEdges {
    DirectedEdge left_forward;
    DirectedEdge right_forward;
    DirectedEdge left_backward;
    DirectedEdge right_backward;
};

SuccessorEdges successor_edges[TOTAL_EDGES];

DirectedEdge DirectedEdge::successor(bool left) const {
    const SuccessorEdges &successors = successor_edges[edge];
    if (backward) {
        if (left) {
            return successors.left_backward;
        } else {
            return successors.right_backward;
        }
    } else {
        if (left) {
            return successors.left_forward;
        } else {
            return successors.right_forward;
        }
    }
}

void populate_successor_edges() {
    for (int edge = 0; edge < TOTAL_EDGES; edge++) {
        SuccessorEdges &se = successor_edges[edge];
        int v1 = dodecahedron_edge[edge].vertex1;
        int v2 = dodecahedron_edge[edge].vertex2;
        Vector vtx1 = dodecahedron_vertex[v1];
        Vector vtx2 = dodecahedron_vertex[v2];
        Vector center = vtx1.add(vtx2).div(2);
        Vector forward = vtx2.sub(vtx1);
        Vector backward = vtx1.sub(vtx2);
        Vector right_forward = center.cross(forward).div(32768);
        Vector right_backward = center.cross(backward).div(32768);
        //Serial.printf("edge %d: v1=%d v2=%d\n", edge, v1, v2);
        for (int other = 0; other < TOTAL_EDGES; other++) {
            if (other == edge) continue;
            int q1 = dodecahedron_edge[other].vertex1;
            int q2 = dodecahedron_edge[other].vertex2;
            if ((q1 == v1) || (q2 == v1)) {
                DirectedEdge successor(other, (q2 == v1));
                if (right_backward.dot(successor.forward_vector()) > 0) {
                    //Serial.printf(" -- right backward: %d %d   (v1:%d v2:%d   q1:%d q2:%d)\n", successor.edge, int(successor.backward), v1, v2, q1, q2);
                    se.right_backward = successor;
                } else {
                    //Serial.printf(" -- left backward : %d %d   (v1:%d v2:%d   q1:%d q2:%d)\n", successor.edge, int(successor.backward), v1, v2, q1, q2);
                    se.left_backward = successor;
                }
            } else if ((q1 == v2) || (q2 == v2)) {
                DirectedEdge successor(other, (q2 == v2));
                if (right_forward.dot(successor.forward_vector()) > 0) {
                    //Serial.printf(" -- right forward : %d %d   (v1:%d v2:%d   q1:%d q2:%d)\n", successor.edge, int(successor.backward), v1, v2, q1, q2);
                    se.right_forward = successor;
                } else {
                    //Serial.printf(" -- left forward  : %d %d   (v1:%d v2:%d   q1:%d q2:%d)\n", successor.edge, int(successor.backward), v1, v2, q1, q2);
                    se.left_forward = successor;
                }
            }
        }
    }
}

// Any given LED can have either two or three adjacent LEDS.

struct AdjacentLEDs {
    int led[3];
    
    bool have_three() const {
        return led[2] >= 0;
    }
    
    int count() const {
        return have_three() ? 3 : 2;
    }
    
    AdjacentLEDs(int edge, int offset) {
        if (offset == 0) {
            const SuccessorEdges &successors = successor_edges[edge];
            int e1 = successors.left_backward.edge;
            int b1 = successors.left_backward.backward;
            int e2 = successors.right_backward.edge;
            int b2 = successors.right_backward.backward;
            led[0] = b1 ? edge_backward(e1, 0) : edge_forward(e1, 0);
            led[1] = b2 ? edge_backward(e2, 0) : edge_forward(e2, 0);
            led[2] = edge_forward(edge, 1);
            return;
        }
        if (offset == (LEDS_PER_EDGE - 1)) {
            const SuccessorEdges &successors = successor_edges[edge];
            int e1 = successors.left_forward.edge;
            int b1 = successors.left_forward.backward;
            int e2 = successors.right_forward.edge;
            int b2 = successors.right_forward.backward;
            led[0] = b1 ? edge_backward(e1, 0) : edge_forward(e1, 0);
            led[1] = b2 ? edge_backward(e2, 0) : edge_forward(e2, 0);
            led[2] = edge_backward(edge, 1);
            return;
        }
        led[0] = edge_forward(edge, offset - 1);
        led[1] = edge_forward(edge, offset + 1);
        led[2] = -1;
    }
};



