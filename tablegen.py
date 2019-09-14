FIXMAX=32768
import sys
import math

def hue_curve(offset):
    return 32768 - (offset>>1)

def constant_hue(hue):
        sector = (hue >> 2) % 6
        offset1 = (hue & 3) * 8192
        offset2 = FIXMAX - offset1
        if sector==0 :
            r = hue_curve(offset1)
            g = FIXMAX - r
            b = 0
        elif sector==1 :
            g = hue_curve(offset2);
            r = FIXMAX - g
            b = 0
        elif sector==2 :
            g = hue_curve(offset1)
            b = FIXMAX - g
            r = 0
        elif sector==3 :
            b = hue_curve(offset2)
            g = FIXMAX - b
            r = 0
        elif sector==4 :
            b = hue_curve(offset1)
            r = FIXMAX - b
            g = 0
        elif sector==5 :
            r = hue_curve(offset2)
            b = FIXMAX - r
            g = 0
        return r,g,b
        
def bright_hue(hue):
        sector = (hue >> 2) % 6
        offset1 = (hue & 3) * 8192
        offset2 = FIXMAX - offset1
        if sector==0 :
            r = FIXMAX
            g = offset1
            b = 0
        elif sector==1 :
            g = FIXMAX
            r = offset2
            b = 0
        elif sector==2 :
            g = FIXMAX
            b = offset1
            r = 0
        elif sector==3 :
            b = FIXMAX
            g = offset2
            r = 0
        elif sector==4 :
            b = FIXMAX
            r = offset1
            g = 0
        elif sector==5 :
            r = FIXMAX
            b = offset2
            g = 0
        return r,g,b    
    
def gen_constant_table():
    print "RGB hue_constant_table[] = {"
    for i in range(25):
        r,g,b = constant_hue(i)
        sys.stdout.write("  {%d, %d, %d},\n" % (r,g,b))
    print "};"

def gen_bright_table():
    print "RGB hue_bright_table[] = {"
    for i in range(25):
        r,g,b = bright_hue(i)
        sys.stdout.write("  {%d, %d, %d},\n" % (r,g,b))
    print "};"

def fixed_lerp(a, b, t):
    return (b * t + a * (32768 - t)) >> 15


def fixed_mul(a, b):
    return (b * a) >> 15

def spline2(i, p0, p1, p2):
    ix = i * 2
    offset = ix & 0x7FFF
    sector = ix >> 15
    if sector == 0:
        return fixed_lerp(p0, p1, offset)
    elif sector == 1:
        return fixed_lerp(p1, p2, offset)
    else: return p2

    
def spline6(i, p0, p1, p2, p3, p4, p5, p6):
    ix = i * 6
    offset = ix & 0x7FFF
    sector = ix >> 15
    if sector == 0:
        return fixed_lerp(p0, p1, offset)
    elif sector == 1:
        return fixed_lerp(p1, p2, offset)
    elif sector == 2:
        return fixed_lerp(p2, p3, offset)
    elif sector == 3:
        return fixed_lerp(p3, p4, offset)
    elif sector == 4:
        return fixed_lerp(p4, p5, offset)
    elif sector == 5:
        return fixed_lerp(p5, p6, offset)
    else: return p6
 
 
def rotate(vtx):
    x, y, z = vtx
    ny = cos*y - sin*z
    nz = sin*y + cos*z
    return x, ny, nz

def generate_dodecahedron_vertices():
    vertices = []
    for x in [-1, 1]:
        for y in [-1, 1]:
            vertices.append((x, y, 1))
            vertices.append((x, y, -1))
            vertices.append((0, x*va, y*vb))
            vertices.append((y*vb, 0, x*va))
            vertices.append((x*va, y*vb, 0))
    rotated = []
    for v in vertices:
        rotated.append(rotate(v))
    return rotated
    

def gendo():
    va = (1.0 + math.sqrt(5)) / 2.0
    ve = 2.0 - va
    vg = va + va - 2.0
    
    elevation = [-va, -ve, ve, va]
    radius = [vg, 2.0, 2.0, vg]
    result = []
    for ring in range(4):
        for point in range(5):
            degrees = 270 - 72*point
            if ring > 1: degrees -= 36
            x = math.cos(degrees * math.pi / 180.0) * radius[ring]
            y = math.sin(degrees * math.pi / 180.0) * radius[ring]
            z = elevation[ring]
            result.append((x,y,z))
    return result
    

def gendodec():
    va = (1.0 + math.sqrt(5)) / 2.0
    vb = math.sqrt(va * va - va - va + 2.0)
    vc = (va - 1.0) * vb
    vd = va - 1.0
    ve = 2.0 - va
    vf = va * vb
    vg = va + va - 2.0
    
    result = []
    
    result.append((0.0, -vg, -va))
    result.append((-vb, -ve, -va))
    result.append((-vc, 1.0, -va))
    result.append((vc, 1.0, -va))
    result.append((vb, -ve, -va))
    
    result.append((0.0, -2.0, -ve))
    result.append((-vf, -vd, -ve))
    result.append((-vb, va, -ve))
    result.append((vb, va, -ve))
    result.append((vf, -vd, -ve))
    
    result.append((-vb, -va, ve))
    result.append((-vf, vd, ve))
    result.append((0.0, 2.0, ve))
    result.append((vf, vd, ve))
    result.append((vb, -va, ve))
    
    result.append((-vc, -1.0, va))
    result.append((-vb, ve, va))
    result.append((0.0, vg, va))
    result.append((vb, ve, va))
    result.append((vc, -1.0, va))
    
    return result
    
    
    
def printvertices(vtx):
    for ring in range(4):
        sys.stdout.write("    // Ring %d\n" % ring)
        sys.stdout.write("    ")
        for cyc in range(5):
            x, y, z = vtx[cyc + ring * 5]
            sys.stdout.write("{%6d,%6d,%6d}, " % (x, y, z))
        sys.stdout.write("\n")


def compare(vlist1, vlist2):
    scaling = vlist1[0][1] / vlist2[0][1]
    for i in range(20):
        x1, y1, z1 = vlist1[i]
        x2, y2, z2 = vlist2[i]
        xa, ya, za = x2*scaling, y2*scaling, z2*scaling
        xd, yd, zd = abs(x1-xa), abs(y1-ya), abs(z1-za)
        if xd > 0.0001 or yd > 0.0001 or zd > 0.0001:
            return False
    return True
    
    
def getscale(vertices):
    high = 0
    for x, y, z in vertices:
        high = max(abs(x), abs(y), abs(z), high)
    return high
    
def convert32k(vertices):
    high = getscale(vertices)
    scaled = []
    for vtx in vertices:
        x, y, z = vtx
        sx, sy, sz = x/high, y/high, z/high
        scaled.append((int(sx*16384), int(sy*16384), int(sz*16384)))
    return scaled

def dv(ring, offset):
    return ring * 5 + (offset % 5)

def dodecahedron_edges():
    result = []
    for strip in range(5):
        v1 = dv(0, strip)
        v2 = dv(0, strip+1)
        v3 = dv(1, strip+1)
        v4 = dv(2, strip+1)
        v5 = dv(3, strip+1)
        v6 = dv(3, strip+2)
        v7 = dv(1, strip+2)
        result.append((v1, v2))
        result.append((v2, v3))
        result.append((v3, v4))
        result.append((v4, v5))
        result.append((v5, v6))
        result.append((v4, v7))
    return result

def print_edges(e):
    for strand in range(5):
        sys.stdout.write("    // Strand %d\n" % strand)
        sys.stdout.write("    ")
        for offset in range(6):
            v1, v2 = e[strand * 6 + offset]
            sys.stdout.write("{%2d, %2d}, " % (v1, v2))
        sys.stdout.write("\n")
        

