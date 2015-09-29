//
// Created by ilewis on 9/29/15.
//

#include "ParameterPipe.h"

ParameterPipe::State ParameterPipe::freeState_ = ParameterPipe::State::Free;

bool ParameterPipe::push(const void *src, size_t size) {
  bool result = false;

  result = currentState_.compare_exchange_strong(
      freeState_,
      State::BusyWriting,
      std::memory_order_acquire,
      std::memory_order_relaxed);

  if (result) {
    transfer_.copy_from(src, size);
    currentState_.store(State::Free, std::memory_order_release);
  }
  return result;
}

bool ParameterPipe::pop() {
  bool result = false;
  result = currentState_.compare_exchange_strong(
      freeState_,
      State::BusyReading,
      std::memory_order_acquire,
      std::memory_order_relaxed);

  if (result) {
    cache_.copy_from(transfer_);
    currentState_.store(State::Free, std::memory_order_release);
  }

  return result;
}

