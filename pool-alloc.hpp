
// Pool Allocator.
//
// Many effects need memory to store their data structures.  But when the
// effect isn't running, the memory can be freed.  Rather than use malloc
// and free, which can fragment over time, I've created a pool allocator.
// The idea is that an effect can allocate from the pool, then the pool
// can be cleared when switching effects.  I've provided a version of operator
// new that can take a pool as a parameter.
//

#define POOL_ALLOC_SIZE 65536

class PoolAlloc {
private:
    double base_[(POOL_ALLOC_SIZE >> 3)];
    uint32_t used_;

public:
    PoolAlloc() : used_(0) {}
    
    // alloc
    //
    // Allocate the specified number of bytes.  The memory is always
    // aligned for doubles.
    
    unsigned char *alloc(uint32_t nbytes) {
        nbytes = (nbytes + 7) & (~7);
        if (used_ + nbytes > POOL_ALLOC_SIZE) {
            Serial.printf("PoolAlloc::alloc failed.\n");
            return NULL;
        }
        unsigned char *result = ((unsigned char *)base_) + used_;
        used_ += nbytes;
        return result;
    }
    
    // clear
    //
    // Deallocate everything in the pool.
    
    void clear() { used_ = 0; }
};

inline void *operator new(size_t sz, class PoolAlloc &pool) {
    return pool.alloc(sz);
}
