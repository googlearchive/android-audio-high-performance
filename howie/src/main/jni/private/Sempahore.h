//
// Created by ilewis on 10/20/15.
//

#ifndef HPA_SEMPAHORE_H
#define HPA_SEMPAHORE_H


#include <atomic>
#include <mutex>

class Sempahore {
public:
  void wait();
  void signal();
private:
  std::atomic<int> count_ {0};
  std::mutex mu_;
  std::condition_variable cond_;
};


#endif //HPA_SEMPAHORE_H
