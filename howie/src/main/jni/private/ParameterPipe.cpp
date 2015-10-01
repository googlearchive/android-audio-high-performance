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

