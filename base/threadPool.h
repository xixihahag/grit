#pragma once
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>

using namespace std;

namespace grit {

class ThreadPool
{
  public:
    ThreadPool(size_t threads)
        : stop(false)
    {
        for (size_t i = 0; i < threads; ++i)
            workers.emplace_back([this] {
                while (!this->stop) {
                    function<void()> task;
                    {
                        unique_lock<mutex> lock(this->queue_mutex);
                        this->condition.wait(lock, [this] {
                            return this->stop || !this->tasks.empty();
                        });
                        if (this->stop && this->tasks.empty()) return;
                        task = move(this->tasks.front());
                        this->tasks.pop();
                    }
                    // FIXME: 考虑怎么传一个参数进去
                    task();
                }
            });
    }

    // add new work item to the pool
    void enqueue(function<void()> task)
    {
        {
            unique_lock<mutex> lock(queue_mutex);

            // don't allow enqueueing after stopping the pool
            if (stop) throw runtime_error("enqueue on stopped ThreadPool");

            tasks.emplace(task);
        }
        condition.notify_one();
    }

    ~ThreadPool()
    {
        {
            unique_lock<mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (thread &worker : workers)
            worker.join();
    }

  private:
    vector<thread> workers;
    // the task queue
    queue<function<void()> > tasks;

    // synchronization
    mutex queue_mutex;
    condition_variable condition;
    atomic<bool> stop;
};

} // namespace grit