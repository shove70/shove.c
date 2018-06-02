#pragma once

#include <vector>
#include <queue>
#include <atomic>
#include <future>
#include <stdexcept>

using namespace std;

namespace shove
{
namespace thread
{

#define THREADPOOL_MAX_NUM 16
//#define THREADPOOL_AUTO_GROW

class ThreadPool
{
    using Task = function<void()>;

    vector<thread>     _pool;
    queue<Task>        _tasks;
    mutex              _lock;
    condition_variable _task_cv;
    atomic<bool>       _run { true };
    atomic<int>        _idleTotal { 0 };

public:

    inline ThreadPool(unsigned short threadNum = 4)
    {
        createThread(threadNum);
    }

    inline ~ThreadPool()
    {
        _run = false;
        _task_cv.notify_all();

        for (std::thread& t: _pool)
        {
            //thread.detach();
            if (t.joinable())
                t.join();
        }
    }

    template<class F, class... Args>
    auto run(F&& f, Args&&... args) -> future<decltype(f(args...))>
    {
        if (!_run)
        {
            throw runtime_error("commit on ThreadPool is stopped.");
        }

        using RetType = decltype(f(args...));
        auto task = make_shared<packaged_task<RetType()>>(bind(forward<F>(f), forward<Args>(args)...));
        future<RetType> future = task->get_future();
        {
            lock_guard<mutex> lock { _lock };
            _tasks.emplace( [task] ()
            {
                (*task)();
            });
        }

#ifdef THREADPOOL_AUTO_GROW
        if (_idleTotal < 1 && _pool.size() < THREADPOOL_MAX_NUM)
        {
            createThread(1);
        }
#endif

        _task_cv.notify_one();

        return future;
    }

    int idlCount()
    {
        return _idleTotal;
    }

    int thrCount()
    {
        return _pool.size();
    }

#ifndef THREADPOOL_AUTO_GROW
private:
#endif
    void createThread(unsigned short threadNum)
    {
        for (; _pool.size() < THREADPOOL_MAX_NUM && threadNum > 0; --threadNum)
        {
            _pool.emplace_back( [this]
            {
                while (_run)
                {
                    Task task;
                    {
                        unique_lock<mutex> lock{ _lock };
                        _task_cv.wait(lock, [this]
                        {
                            return !_run || !_tasks.empty();
                        });

                        if (!_run && _tasks.empty())
                        {
                            return;
                        }

                        task = move(_tasks.front());
                        _tasks.pop();
                    }

                    _idleTotal--;
                    task();
                    _idleTotal++;
                }
            });

            _idleTotal++;
        }
    }
};

}
