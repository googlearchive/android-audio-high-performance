//
// Created by ilewis on 10/20/15.
//

#include "Worker.h"

Worker::Worker(int queueLength) : queue_(queueLength){
  thread_.reset(new std::thread([this]{threadFn();}));
}

Worker::~Worker() {
  push_work([this]{cancelled_ = true;});
}

bool Worker::push_work(const Worker::work_item_t &item) {
  bool result = queue_.push(item);
  if (result) {
    sem_.signal();
  }
  return result;
}

bool Worker::pop_work(Worker::work_item_t *out_item) {
  bool result = false;
  sem_.wait();
  result = queue_.pop(out_item);
  return result;
}

void Worker::threadFn() {
  while (!cancelled_) {
    work_item_t item;
    if (pop_work(&item)) {
      item();
    }
  }
}
