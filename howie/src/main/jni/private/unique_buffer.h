/*
 * Copyright 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
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
