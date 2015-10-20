#ifndef COMMANDQUEUE_H
#define COMMANDQUEUE_H

#include <atomic>
#include <cassert>
#include <memory>
#include <limits>

#ifndef CACHE_ALIGN
#define CACHE_ALIGN 64
#endif

template <typename T>
class Ringbuffer {
public:
  explicit Ringbuffer(int size)
      : Ringbuffer(size, new T[size]) {}


  explicit Ringbuffer(int size, T* buffer)
      : size_(size), buffer_(buffer) {

    // This is necessary because we depend on twos-complement wraparound
    // to take care of overflow conditions.
    assert(size < std::numeric_limits<int>::max());
  }

  bool push(const T& item) {
    return push([&](T* ptr) -> bool {*ptr = item; return true; });
  }

  // get() is idempotent between calls to commit().
  T*getWriteablePtr() {
    T* result = nullptr;


    bool check  __attribute__((unused));//= false;

    check = push([&](T* head)-> bool {
        result = head;
        return false; // don't increment
    });

    // if there's no space, result should not have been set, and vice versa
    assert(check == (result != nullptr));

    return result;
  }

  bool commitWriteablePtr(T *ptr) {
    bool result = push([&](T* head)-> bool {
      // this writer func does nothing, because we assume that the caller
      // has already written to *ptr after acquiring it from a call to get().
      // So just double-check that ptr is actually at the write head, and
      // return true to indicate that it's safe to advance.

      // if this isn't the same pointer we got from a call to get(), then
      // something has gone terribly wrong. Either there was an intervening
      // call to push() or commit(), or the pointer is spurious.
      assert(ptr == head);
      return true;
    });
    return result;
  }

  // writer() can return false, which indicates that the caller
  // of push() changed its mind while writing (e.g. ran out of bytes)
  template<typename F>
  bool push(const F& writer) {
    bool result = false;
    int readptr = read_.load(std::memory_order_acquire);
    int writeptr = write_.load(std::memory_order_relaxed);

    // note that while readptr and writeptr will eventually
    // wrap around, taking their difference is still valid as
    // long as size_ < MAXINT.
    int space = size_ - (int)(writeptr - readptr);
    if (space >= 1) {
      result = true;

      // writer
      if (writer(buffer_.get() + (writeptr % size_))) {
        ++writeptr;
        write_.store(writeptr, std::memory_order_release);
      }
    }
    return result;
  }

  bool pop(T* out_item) {
    return pop([&](T* ptr)-> bool {*out_item = *ptr; return true;});
  }

  template<typename F>
  bool pop(const F& reader) {
    bool result = false;

    int writeptr = write_.load(std::memory_order_acquire);
    int readptr = read_.load(std::memory_order_relaxed);

    // As above, wraparound is ok
    int available = (int)(writeptr - readptr);
    if (available >= 1) {
      result = true;
      if (reader(buffer_.get() + (readptr % size_))) {
        ++readptr;
        read_.store(readptr, std::memory_order_release);

      }
    }

    return result;
  }

  uint32_t size(void) {
    int writeptr = write_.load(std::memory_order_acquire);
    int readptr = read_.load(std::memory_order_relaxed);

#if 0
    // As above, wraparound is ok
    int32_t count = (writeptr - readptr);
    if( count < 0 ) {
        count = size_ + count;
    }
    return (uint32_t )count;
#else
    return (uint32_t)(writeptr - readptr);
#endif
  }

  bool empty(void) {
     return (size() == 0 );
  }
private:
  int size_;
  std::unique_ptr<T> buffer_;

  // forcing cache line alignment to eliminate false sharing of the
  // frequently-updated read and write pointers. The object is to never
  // let these get into the "shared" state where they'd cause a cache miss
  // for every write.
  alignas(CACHE_ALIGN) std::atomic<int> read_ { 0 };
  alignas(CACHE_ALIGN) std::atomic<int> write_ { 0 };
};

#endif // COMMANDQUEUE_H
