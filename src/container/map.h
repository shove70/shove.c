#pragma once

#include <map>
#include <mutex>
#include <vector>

using namespace std;

namespace shove
{
namespace container
{

template <typename _Key, typename _Tp>
class Map
{
private:

    std::map<_Key, _Tp> _map;
    mutable mutex       _mutex;

public:

    using iterator    = typename std::map<_Key, _Tp>::iterator;
    using mapped_type = typename std::map<_Key, _Tp>::mapped_type;

    size_t size() const noexcept
    {
        size_t size;

        lock_guard<mutex> lock(_mutex);
        size = _map.size();
        return size;
    }

    iterator begin() noexcept
    {
        lock_guard<mutex> lock(_mutex);

        return _map.begin();
    }

    iterator end() noexcept
    {
        lock_guard<mutex> lock(_mutex);

        return _map.end();
    }

    bool exists(const _Key& __x) noexcept
    {
        lock_guard<mutex> lock(_mutex);

        return (_map.find(__x) != _map.end());
    }

    iterator find(const _Key& __x) noexcept
    {
        lock_guard<mutex> lock(_mutex);

        return _map.find(__x);
    }

    iterator insert(iterator __hint, const _Tp& __x) noexcept
    {
        lock_guard<mutex> lock(_mutex);

        return _map.insert(__hint, __x);
    }

    iterator erase(iterator __position) noexcept
    {
        lock_guard<mutex> lock(_mutex);

        return _map.erase(__position);
    }

    size_t erase(const _Key& __x) noexcept
    {
        lock_guard<mutex> lock(_mutex);

        return _map.erase(__x);
    }
    
    iterator erase(iterator __first, iterator __last) noexcept
    {
        lock_guard<mutex> lock(_mutex);

        return _map.erase(__first, __last);
    }

    void clear() noexcept
    {
        lock_guard<mutex> lock(_mutex);

        _map.clear();
    }

    mapped_type& operator[](const _Key& __k) noexcept
    {
        lock_guard<mutex> lock(_mutex);

        return _map[__k];
    }

    mapped_type& operator[](_Key&& __k) noexcept
    {
        lock_guard<mutex> lock(_mutex);

        return _map[std::move(__k)];
    }

    mapped_type& at(const _Key& __k) noexcept
    {
        lock_guard<mutex> lock(_mutex);

        return _map.at(__k);
    }

    const mapped_type& at(const _Key& __k) const noexcept
    {
        lock_guard<mutex> lock(_mutex);

        return _map.at(__k);
    }

    void reserve(size_t __n) noexcept
    {
        lock_guard<mutex> lock(_mutex);

        _map.reserve(__n);
    }

    bool empty() noexcept
    {
        return (size() == 0);
    }

    void getKeys(vector<_Key>& keys) noexcept
    {
        lock_guard<mutex> lock(_mutex);

        keys.clear();
        iterator it = _map.begin();

        while (it != _map.end())
        {
            keys.push_back(it->first);
            it++;
        }
    }
};

}
}
