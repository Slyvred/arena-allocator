#include <cstdio>
#include <iostream>
#include <memory>

class Arena {
private:
    char* buffer;
    std::size_t offset;
    std::size_t capacity;
public:
    Arena(std::size_t size) {
        this->buffer = static_cast<char*>(::operator new(size));
        this->offset = 0;
        this->capacity = size;

        printf("Created arena of capaticy %lu, at %p\n", size, &this->buffer);
    }

    ~Arena() {
        ::operator delete(buffer);
        printf("Destroyed Arena !\n");
    }

    void* allocate(std::size_t size, std::size_t alignment) {
        char* curr_ptr = this->buffer + this->offset;
        void* aligned_ptr = curr_ptr;
        std::size_t space_left = this->capacity - this->offset;

        if (std::align(alignment, size, aligned_ptr, space_left) == nullptr) {
            throw std::bad_alloc();
        }

        offset = (char*)aligned_ptr - this->buffer + size;
        return aligned_ptr;
    }

    void reset() {
        offset = 0;
    }

    template<typename T, typename ... Args>
    T* make(Args&&... args) {
        void* ptr = allocate(sizeof(T), alignof(T));
        return new (ptr) T(std::forward<Args> ...);
    }
};

// https://en.cppreference.com/w/cpp/named_req/Allocator.html
template<typename T>
class ArenaAllocator {
public:
    typedef T value_type;

    explicit ArenaAllocator(Arena* arena) noexcept : arena(arena) {}

    template<class U>
    constexpr ArenaAllocator(const ArenaAllocator <U>&other) noexcept : arena(other.arena) {}

    template <typename U>
    friend class ArenaAllocator;

    [[nodiscard]] T* allocate(std::size_t n) {
        return static_cast<T*>(arena->allocate(n * sizeof(T), alignof(T)));
    }

    void deallocate(T* /*p*/, std::size_t /*n*/) noexcept {
        // Do nothing, as it's managed by the Arena
    }

    bool operator==(const ArenaAllocator& other) const noexcept {
        return arena == other.arena;
    }

    bool operator!=(const ArenaAllocator& other) const noexcept {
        return !(*this == other);
    }

private:
    Arena* arena;
};
