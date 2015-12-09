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
#ifndef HIGH_PERFORMANCE_STATEPIPE_H
#define HIGH_PERFORMANCE_STATEPIPE_H


#include <atomic>
#include <memory>
#include <mutex>
#include <vector>

namespace howie {
  class ParameterPipe {
  public:
    ParameterPipe(size_t maxElement, size_t margin);

    // push returns the number of bytes written.
    size_t push(const void *src, size_t srcSize);

    // pop returns true on success. Failure implies that the
    // reader found itself in contention with the writer.
    bool pop();
    unsigned char * top() const;

    // top returns

    size_t maxElementSize() const { return elementSize_; }


  private:
    // Track the write head. No need to track the read head, as it's
    // always just behind the write head.
    std::atomic<size_t> writepos_{0};

    // Although this queue is lock-free to readers, writers can
    // block each other.
    std::vector<std::mutex> locks_;

    // Size of each data element
    size_t elementSize_;

    // How many elements are in the queue. This is a margin of safety--a
    // lower value makes it more likely that the reader will collide
    // with the writer, which would cause updates to be missed. (This is
    // expected behavior in collisions, as the alternative is to block
    // until the update is ready, and that is impermissible.)
    size_t margin_;

    // The actual data buffer. Its size is elementSize_ * margin_.
    std::unique_ptr<unsigned char> data_;

    // keep a cached element so that top() can always succeed.
    std::unique_ptr<unsigned char> cache_;
    std::unique_ptr<unsigned char> temp_;

    // pop returns the number of bytes read, which will be zero
    // if contention was detected during the read. In this case,
    // the operation may be retried.
    size_t pop (void * dest, size_t size);

    size_t logicalPosToByteOffset(size_t current) const;

  };
} // namespace howie

#endif //HIGH_PERFORMANCE_STATEPIPE_H
