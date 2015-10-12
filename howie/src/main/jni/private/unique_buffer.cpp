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
#include "unique_buffer.h"
#include <algorithm>

void unique_buffer::reset(size_t size) {
  size_ = size;
  if (size_ > 0) {
    buffer_.reset(new unsigned char[size_]);
  } else {
    buffer_.reset(nullptr);
  }
}

void unique_buffer::clear(unsigned char clear_value) {
  if (buffer_.get())
    memset(buffer_.get(), clear_value, size_);
}

size_t unique_buffer::copy_from(const void *src, size_t size) {
  if (!src) return 0;

  size_t actualSize = std::min(size, size_);
  memcpy(buffer_.get(), src, actualSize);
  return actualSize;
}

size_t unique_buffer::copy_to(void *dest, size_t size) {
  if (!dest) return 0;

  size_t actualSize = std::min(size, size_);
  memcpy(dest, buffer_.get(), actualSize);
  return actualSize;
}

size_t unique_buffer::copy_from(const unique_buffer &other) {
  return copy_from(other.get(), other.size());
}
