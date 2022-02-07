#ifndef _FSET_H_
#define _FSET_H_

#include <string>
#include <functional>
#include "skiplist.h"

namespace filebased {

template <typename _Key, typename _Cmp = std::less<_Key>>
class fset: public skiplist<_Key, _Cmp> {
public:
    fset(const std::string &_name): skiplist<_Key, _Cmp>(_name) {}
    fset(const fset &o) = delete;

    std::pair<
        typename skiplist<_Key, _Cmp>::iterator, 
        typename skiplist<_Key, _Cmp>::iterator
    > equal_range(const _Key &nkey) {
        return make_pair(lower_bound(nkey), upper_bound(nkey));
    }
    siz_t count(const _Key &nkey) {
        return skiplist<_Key, _Cmp>::find(nkey) 
            != skiplist<_Key, _Cmp>::end()? 1: 0;
    }

    ~fset() = default;
};

}

#endif // _FSET_H_