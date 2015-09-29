//
// Created by ilewis on 9/29/15.
//

#ifndef HIGH_PERFORMANCE_STATEPIPE_H
#define HIGH_PERFORMANCE_STATEPIPE_H


#include <memory>
#include <atomic>
#include "unique_buffer.h"

class ParameterPipe {
private:
  enum class State : unsigned int {
    Free,
    BusyWriting,
    BusyReading,
  };

public:
  ParameterPipe(size_t size)
      : transfer_(size),
        cache_(size),
        currentState_(State::Free) {
    transfer_.clear();
    cache_.clear();
  }

  bool push(const void *src, size_t size);
  bool pop();
  unsigned char *get() const { return cache_.get(); }
  size_t size() const { return cache_.size(); }

private:
  static State freeState_;
  unique_buffer transfer_;
  unique_buffer cache_;
  std::atomic<State> currentState_;
};


#endif //HIGH_PERFORMANCE_STATEPIPE_H
