struct Vector {
    int32_t X;
    int32_t Y;
    int32_t Z;
    
    Vector(int32_t x, int32_t y, int32_t z) : X(x), Y(y), Z(z) {}
    
    Vector negate() const {
        return Vector(-X, -Y, -Z);
    }
    
    Vector div(int n) const {
        return Vector(X / n, Y / n, Z / n);
    }
    
    Vector add(const Vector &other) const {
        return Vector(X + other.X, Y + other.Y, Z + other.Z);
    }
    
    Vector sub(const Vector &other) const {
        return Vector(X - other.X, Y - other.Y, Z - other.Z);
    }
    
    Vector cross(const Vector &other) const {
        return Vector(Y*other.Z - Z*other.Y, Z*other.X - X*other.Z, X*other.Y - Y*other.X);
    }
    
    Vector fixed_lerp(const Vector &other, fixed offset) const {
        return Vector(::fixed_lerp(X, other.X, offset),
                      ::fixed_lerp(Y, other.Y, offset),
                      ::fixed_lerp(Z, other.Z, offset));
    }
    
    int32_t dot(const Vector &other) const {
        return X*other.X + Y*other.Y + Z*other.Z;
    }
};

