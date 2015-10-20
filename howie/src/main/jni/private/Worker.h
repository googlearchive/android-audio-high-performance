//
// Created by ilewis on 10/20/15.
//

#ifndef HPA_WORKQUEUE_H
#define HPA_WORKQUEUE_H


#include <functional>
#include <thread>
#include "Sempahore.h"
#include "Ringbuffer.h"

class Worker {
public:
  Worker(int queueLength);
  ~Worker();

  typedef std::function<void ()> work_item_t;

  bool push_work(const work_item_t& item);

private:
  Ringbuffer<work_item_t> queue_;
  Sempahore sem_;
  std::unique_ptr<std::thread> thread_;
  std::atomic<bool> cancelled_ {false};

  bool pop_work(work_item_t* out_item);
  void threadFn();
};


#endif //HPA_WORKQUEUE_H
