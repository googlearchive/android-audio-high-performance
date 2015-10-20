//
// Created by ilewis on 10/20/15.
//

#include "Sempahore.h"
#include "howie-private.h"

void Sempahore::wait() {
  // use acquire semantics here because we assume that the order
  // of operations will be:
  // 1. wait
  // 2. do something
  int available = count_.fetch_sub(1, std::memory_order_acquire);
  // if the previous value of the available count was zero or less, that
  // means it's at most -1 now. We'll need to wait.
  if (available <= 0) {
    std::unique_lock<std::mutex> lock(mu_);
    cond_.wait(lock);
  }
}

void Sempahore::signal() {
  // Release semantics used here because we assume the order of
  // operations is
  // 1. do something
  // 2. signal
  int available = count_.fetch_add(1, std::memory_order_release);

  // If the available count was less than zero, there's somebody waiting.
  if (available < 0) {
    cond_.notify_one();
  }
}
