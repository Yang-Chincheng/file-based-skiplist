#ifndef _FMAP_H_
#define _FMAP_H_

#include <algorithm>
#include <string>
#include <functional>
#include "skiplist.h"

namespace filebased {

template <typename _K, typename _V> 
    using fmapele = std::pair<_K, _V>;

template <typename _Key, typename _Val, typename _Cmp = std::less<_Key> > 
class fmapCmp {
public:
    _Cmp comp;
    bool operator () (const fmapele<_Key, _Val> &lhs, const fmapele<_Key, _Val> &rhs) {
        return comp(lhs.first, rhs.first);
    }
};

template <typename _Key, typename _Val, typename _Cmp = std::less<_Key>>
class fmap: public skiplist<fmapele<_Key, _Val>, fmapCmp<_Key, _Val, _Cmp>> {
public:
    fmap(const std::string &_name): skiplist<fmapele<_Key, _Val>, fmapCmp<_Key, _Val, _Cmp>>(_name) {}
    fmap(const fmap &o) = delete;

    _Val at(const _Key &nkey) {
        auto it = skiplist<fmapele<_Key, _Val>, fmapCmp<_Key, _Val, _Cmp>>::find(make_pair(nkey, _Val()));
        return it == skiplist<fmapele<_Key, _Val>, fmapCmp<_Key, _Val, _Cmp>>::end()? _Val(): (*it).second;
    }

    _Val set(const _Key &nkey, const _Val &nval) {
        fmapele<_Key, _Val> ele = std::make_pair(nkey, nval);
        skiplist<fmapele<_Key, _Val>, fmapCmp<_Key, _Val, _Cmp>>::erase(ele);
        skiplist<fmapele<_Key, _Val>, fmapCmp<_Key, _Val, _Cmp>>::insert(ele);
        return nval;
    }

    ~fmap() = default;
};

}
#endif // _FMAP_H_