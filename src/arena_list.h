#pragma once

#include <cstdio>
#include <limits>
#include <memory>
#include <list>
#include "logger.h"

/* This file contains the arena implementation, it uses std::list to store the data and supports automatic expansion */

typedef struct {
    char* data;
    std::size_t capacity;
}Buffer;

extern Logger logger;

class Arena {
private:
    std::list<Buffer> buffers;
    std::size_t offset;
    std::size_t buffer_idx;
public:
    Arena(std::size_t capacity) {
        this->buffers.push_back(Buffer{static_cast<char*>(::operator new(capacity)), capacity});
        this->offset = 0;
        this->buffer_idx = 0;

        logger.log(LOG_LEVEL::DEBUG, "Created arena of capaticy %lu, at %p", capacity, &this->buffers.back().data);
    }

    ~Arena() {
        for (auto& buffer: this->buffers){
            ::operator delete(buffer.data);
        }
        this->buffers.clear();
        logger.log(LOG_LEVEL::DEBUG, "Destroyed Arena !");
    }

    void* allocate(std::size_t size, std::size_t alignment) {
        // Get current buffer infos
        Buffer &current_buffer = *std::next(this->buffers.begin(), this->buffer_idx); // Is usually buffer.back(), unless we did a reset and are left with a bunch of free buffers
        char* curr_ptr = current_buffer.data + this->offset;
        void* aligned_ptr = curr_ptr;
        std::size_t space_left = current_buffer.capacity - this->offset;

        // If we fail to fit our bytes, we either allocate a new buffer or move on to the next one
        if (std::align(alignment, size, aligned_ptr, space_left) == nullptr) {
            try {
                // If there isn't any buffer available, we create a new one
                if (std::next(this->buffers.begin(), this->buffer_idx + 1) == this->buffers.end()) {

                    // We want to heap allocate as little as possible, so we double the capacity of each new buffer
                    std::size_t new_capacity = current_buffer.capacity * 2;
                    this->buffers.push_back(Buffer{static_cast<char*>(::operator new(new_capacity)), new_capacity});
                    logger.log(LOG_LEVEL::DEBUG, "Created new buffer of capacity %lu at %p", new_capacity, &this->buffers.back().data);
                }

                // If next buffer is availble, we just increment the buffer_idx to use this one for our allocations
                this->offset = 0;                       // We'll work in a new buffer so we reset the offset;
                this->buffer_idx++;
                return this->allocate(size, alignment); // Retry allocation in new buffer

            } catch (const std::exception&) {
                logger.log(LOG_LEVEL::ERROR, "Arena.allocate(): Failed to align, memory is likely full");
                throw std::bad_alloc();
            }
        }

        this->offset = (char*)aligned_ptr - current_buffer.data + size;
        return aligned_ptr;
    }

    void reset() {
        this->offset = 0;
        this->buffer_idx = 0;
        logger.log(LOG_LEVEL::DEBUG, "Arena was reset");
    }

    template <typename T, typename... Args>
    T* make(Args&&... args) {
        static_assert(std::is_trivially_destructible_v<T>, "Arena<T>::make() requires trivially destructible types");
        void* ptr = this->allocate(sizeof(T), alignof(T));
        logger.log(LOG_LEVEL::DEBUG, "Allocated new object of size %lu and align %lu", sizeof(T), alignof(T));
        return new (ptr) T(std::forward<Args>(args)...);
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
