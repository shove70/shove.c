#pragma once

#include <queue>
#include <mutex>

using namespace std;

namespace shove
{
namespace container
{

template <typename T>
class Queue
{
private:

    std::queue<T> _queue;
    mutable mutex _mutex;

public:

    size_t size() const noexcept
    {
        size_t size;

        lock_guard<mutex> lock(_mutex);
        size = _queue.size();
        return size;
    }

    void push(const T& t) noexcept
    {
        lock_guard<mutex> lock(_mutex);
        _queue.push(t);
    }

    void push(const vector<T>& t) noexcept
    {
        lock_guard<mutex> lock(_mutex);

        for (auto& e: t)
        {
            _queue.push(e);
        }
    }

    bool empty() noexcept
    {
        return (size() == 0);
    }

    T& front() noexcept
    {
        lock_guard<mutex> lock(_mutex);
        T& value = _queue.front();
        return value;
    }

    void pop() noexcept
    {
        lock_guard<mutex> lock(_mutex);
        if (_queue.size() == 0)
        {
            return;
        }
        _queue.pop();
    }

    void clear() noexcept
    {
        lock_guard<mutex> lock(_mutex);
        while (_queue.size() > 0)
        {
            _queue.pop();
        }
    }
};

}
}
