#include <stdio.h>
#include <unistd.h>

#include <map>
#include <string>
#include <vector>

#include "../BlockingQueue.h"
#include "../CountDownLatch.h"
#include "../CurrentThread.h"
#include "../Thread.h"
#include "../TimeStamp.h"

class Bench {
public:
  Bench(int numThreads) : startLatch_(numThreads), stopLatch_(-1) {
    queues_.reserve(numThreads);
    threads_.reserve(numThreads);

    for (int i = 0; i < numThreads; ++i) {
      queues_.emplace_back(new muduo::BlockingQueue<int>());
      char name[32];
      snprintf(name, sizeof name, "work thread %d", i);
      threads_.emplace_back(new muduo::Thread(
          [this, i] {
            threadFunc(i);
          },
          muduo::string(name)));
    }
  }

  void Start() {
    muduo::TimeStamp start = muduo::TimeStamp::now();

    for (auto& thr : threads_) {
      thr->start();
    }
    startLatch_.wait();
    muduo::TimeStamp end = muduo::TimeStamp::now();
    printf("all %zd threads started, %.3fms\n",
           threads_.size(),
           1e3 * muduo::timeDifference(end, start));
  }

  void Run() {
    muduo::TimeStamp start  = muduo::TimeStamp::now();
    const int        rounds = 100003;
    queues_[0]->put(rounds);

    auto   done    = done_.take();
    double elapsed = muduo::timeDifference(done.second, start);

    printf("thread id = %d done, total %.3fms , %.3fus / round \n",
           done.first,
           1e3 * elapsed,
           1e6 * elapsed / rounds);
  }

  void Stop() {
    muduo::TimeStamp stop = muduo::TimeStamp::now();

    for (const auto& queue : queues_) {
      queue->put(-1);
    }

    for (auto& thr : threads_) {
      thr->join();
    }

    muduo::TimeStamp end = muduo::TimeStamp::now();
    printf("all %zd threads joined, %.3fms\n",
           threads_.size(),
           1e3 * muduo::timeDifference(end, stop));
  }

private:
  using TimeStampQueue = muduo::BlockingQueue<std::pair<int, muduo::TimeStamp>>;
  TimeStampQueue        done_;
  muduo::CountDownLatch startLatch_, stopLatch_;
  std::vector<std::unique_ptr<muduo::BlockingQueue<int>>> queues_;
  std::vector<std::unique_ptr<muduo::Thread>>             threads_;

  const bool verbose_ = true;

  void threadFunc(int id) {
    startLatch_.countDown();

    muduo::BlockingQueue<int>* input = queues_[id].get();
    muduo::BlockingQueue<int>* output =
        queues_[(id + 1) % queues_.size()].get();

    while (true) {
      int value = input->take();

      if (0 < value) {
        output->put(value - 1);
        if (verbose_) {
          printf("thread %d, got %d\n", id, value);
        }
        continue;
      }

      if (0 == value) {
        done_.put(std::make_pair(id, muduo::TimeStamp::now()));
      }

      break;
    }
  }
};

int main(int argc, char** argv) {
  int threads = argc > 1 ? atoi(argv[1]) : 1;

  printf("sizeof BlockingQueue = %zd\n", sizeof(muduo::BlockingQueue<int>));
  printf("sizeof deque<int> = %zd\n", sizeof(std::deque<int>));
  Bench t(threads);
  t.Start();
  t.Run();
  t.Stop();

  return 0;
}
