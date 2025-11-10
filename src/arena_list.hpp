#pragma once

#include "logger.hpp"
#include <cassert>
#include <cstdio>
#include <list>
#include <memory>

/* This file contains the arena implementation, it uses std::list to store the
 * data and supports automatic expansion */

typedef struct {
  char *data;
  std::size_t capacity;
} Buffer;

extern Logger logger;

class Arena {
private:
  std::list<Buffer> buffers_;
  std::size_t offset_;
  std::size_t bufferIdx_;

public:
  Arena(std::size_t capacity) {
    this->buffers_.push_back(
        Buffer{static_cast<char *>(::operator new(capacity)), capacity});
    this->offset_ = 0;
    this->bufferIdx_ = 0;

    logger.log(Logger::LogLevel::DEBUG, "Created arena of capaticy %lu, at %p",
               capacity, &this->buffers_.back().data);
  }

  ~Arena() {
    for (auto &buffer : this->buffers_) {
      ::operator delete(buffer.data);
      logger.log(Logger::LogLevel::DEBUG, "Deleted %p", &buffer.data);
    }
    logger.log(Logger::LogLevel::DEBUG, "Destroyed Arena !");
  }

  void *allocate(std::size_t size, std::size_t alignment) {
    // Get current buffer infos
    Buffer &current_buffer = *std::next(
        this->buffers_.begin(),
        this->bufferIdx_); // Is usually buffer.back(), unless we did a reset
                           // and are left with a bunch of free buffers
    char *curr_ptr = current_buffer.data + this->offset_;
    void *aligned_ptr = curr_ptr;
    std::size_t space_left = current_buffer.capacity - this->offset_;

    // If we fail to fit our bytes, we either allocate a new buffer or move on
    // to the next one
    if (std::align(alignment, size, aligned_ptr, space_left) == nullptr) {
      try {
        // If there isn't any buffer available, we create a new one
        if (std::next(this->buffers_.begin(), this->bufferIdx_ + 1) ==
            this->buffers_.end()) {

          // We want to heap allocate as little as possible, so we double the
          // capacity of each new buffer
          std::size_t new_capacity = current_buffer.capacity * 2;
          this->buffers_.push_back(Buffer{
              static_cast<char *>(::operator new(new_capacity)), new_capacity});
          logger.log(Logger::LogLevel::DEBUG,
                     "Created new buffer of capacity %lu at %p", new_capacity,
                     &this->buffers_.back().data);
        }

        // If next buffer is availble, we just increment the buffer_idx to use
        // this one for our allocations
        this->offset_ = 0; // We'll work in a new buffer so we reset the offset;
        this->bufferIdx_++;
        return this->allocate(size,
                              alignment); // Retry allocation in new buffer

      } catch (const std::exception &) {
        logger.log(Logger::LogLevel::ERROR,
                   "Arena.allocate(): Failed to align, memory is likely full");
        throw std::bad_alloc();
      }
    }

    this->offset_ = (char *)aligned_ptr - current_buffer.data + size;
    return aligned_ptr;
  }

  void reset() {
    this->offset_ = 0;
    this->bufferIdx_ = 0;
    logger.log(Logger::LogLevel::DEBUG, "Arena was reset");
  }

  template <typename T, typename... Args> T *make(Args &&...args) {
    static_assert(std::is_trivially_destructible_v<T>,
                  "Arena<T>::make() requires trivially destructible types");
    void *ptr = this->allocate(sizeof(T), alignof(T));
    logger.log(Logger::LogLevel::DEBUG,
               "Allocated new object of size %lu and align %lu", sizeof(T),
               alignof(T));
    return new (ptr) T(std::forward<Args>(args)...);
  }
};

// https://en.cppreference.com/w/cpp/named_req/Allocator.html
template <typename T> class ArenaAllocator {
public:
  typedef T value_type;

  explicit ArenaAllocator(Arena *arena) noexcept : arena_(arena) {}

  template <class U>
  constexpr ArenaAllocator(const ArenaAllocator<U> &other) noexcept
      : arena_(other.arena_) {}

  template <typename U> friend class ArenaAllocator;

  [[nodiscard]] T *allocate(std::size_t n) {
    return static_cast<T *>(arena_->allocate(n * sizeof(T), alignof(T)));
  }

  void deallocate(T * /*p*/, std::size_t /*n*/) noexcept {
    // Do nothing, as it's managed by the Arena
  }

  bool operator==(const ArenaAllocator &other) const noexcept {
    return arena_ == other.arena_;
  }

  bool operator!=(const ArenaAllocator &other) const noexcept {
    return !(*this == other);
  }

private:
  Arena *arena_;
};
