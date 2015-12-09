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
#include "ParameterPipe.h"
#include <algorithm>
#include <chrono>
#include <cstring>

namespace howie {

  ParameterPipe::ParameterPipe(size_t maxElement, size_t margin)
      : locks_(margin), elementSize_(maxElement), margin_(margin) {
    data_.reset(new unsigned char[maxElement * margin]);
    cache_.reset(new unsigned char[maxElement]);
    temp_.reset(new unsigned char[maxElement]);
  }


  /**
   * Compute the ringbuffer byte offset of the given logical position.
   *
   * The logical positions (read position, write position) are written as if
   * the buffer extends to infinity (or at least to MAX_UINT) in the forward
   * direction. This makes it easier to reason about their relative positions
   * and prevents ambiguity. We allow the positions to wrap around at MAX_UINT.
   * Due to the magic of 2's complement arithmetic, the difference between
   * the two positions is correct even if the write pointer wraps before the
   * read pointer. (Assuming that the difference never exceeds MAX_INT, which
   * it won't if we maintain the positions correctly.)
   */
  size_t ParameterPipe::logicalPosToByteOffset(size_t current) const {
    size_t offset = (current % margin_) * elementSize_;
    return offset;
  }



  size_t ParameterPipe::push(const void *src, size_t srcSize) {
    size_t result = 0;
    size_t size = std::min(srcSize, elementSize_);

    // Check the write pointer before acquiring a lock, so we know which lock
    // to acquire. That does imply that the write pointer must be atomic, since
    // it can be accessed by multiple threads outside of the lock.
    // this is a "consume" load because there are no implicit dependencies,
    // all we need to do is make sure that explicit dependencies aren't
    // reordered.
    size_t current = writepos_.load(std::memory_order_consume);

    std::mutex& currentLock = locks_[current % margin_];
    std::lock_guard<std::mutex> lock(currentLock);
    {
      size_t offset = logicalPosToByteOffset(current);
      unsigned char *dest = data_.get() + offset;
      memcpy(dest, src, size);
      result = size;

      ++current;

      writepos_.store(current, std::memory_order_release);
    }
    return result;
  }

  size_t ParameterPipe::pop(void *dest, size_t destSize) {
    size_t result = 0;
    size_t size = std::min(destSize, elementSize_);

    // this read-acquire matches the write-release in push() and ensures
    // that data is not loaded before the latest value of writepos_ is loaded.
    size_t read = writepos_.load(std::memory_order_acquire);

    // read position is write - 1; we want the update from just before the
    // current write pointer, which is very recent yet also very likely safe
    // to read.
    read -= 1;
    std::mutex& currentLock = locks_[read % margin_];
    bool locked = currentLock.try_lock();
    std::lock_guard<std::mutex>(currentLock, std::adopt_lock);
    if (locked) {
      size_t offset = logicalPosToByteOffset(read);
      unsigned char *src = data_.get() + offset;
      std::memcpy(dest, src, size);
      result = size;
    }

    return result;
  }

  bool ParameterPipe::pop() {
    bool result = false;
    size_t popResult = pop(temp_.get(), elementSize_);
    if (popResult > 0) {
      cache_.swap(temp_);
      result = true;
    }
    return result;
  }

  unsigned char *ParameterPipe::top() const {
    return cache_.get();
  }

} // namespace howie

