//
// Created by ilewis on 9/29/15.
//

#ifndef HIGH_PERFORMANCE_UNIQUE_BUFFER_H
#define HIGH_PERFORMANCE_UNIQUE_BUFFER_H

#include <stddef.h>
#include <memory>

class unique_buffer {
public:
  unique_buffer() { reset(0); }
  unique_buffer(size_t size) { reset(size); }

  unsigned char *get() const { return buffer_.get(); }
  size_t size() const { return size_; }

  // deletes the previous buffer, if any, and allocates a new one of
  // the specified size.
  void reset(size_t size);

  // Sets every byte in the buffer to the specified value.
  void clear(unsigned char clear_value = 0);

  //
  // Copy functions: return the number of bytes actually copied,
  // which will be less than or equal to the specified size.
  //
  size_t copy_to(void *dest, size_t size);
  size_t copy_from(const void *src, size_t size);
  size_t copy_from(const unique_buffer& other);

private:
  std::unique_ptr<unsigned char> buffer_;
  size_t size_;
};


#endif //HIGH_PERFORMANCE_UNIQUE_BUFFER_H
