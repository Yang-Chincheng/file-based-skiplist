#ifndef _SKIPLIST_H_
#define _SKIPLIST_H_

#include <functional>
#include <cstdlib>
#include <utility>
#include <cstring>
#include <ctime>
#include <algorithm>

#include "mempool.h"

namespace filebased {
    
typedef unsigned long lev_t, siz_t;

template <typename _Key, typename _Cmp = std::less<_Key>>
class skiplist {

protected:
    struct node {
        _Key key;
        lev_t node_lev;
        fptr backward;
        struct {
            fptr forward;
            siz_t span;
        } level[skiplist::MAX_LEV];
    };
    friend struct node;

    Mpl mpl;
    _Cmp comp;
    struct {
        fptr head;
        fptr tail;
        lev_t list_lev;
        siz_t len;
    } list;

    lev_t getRandLev();

public:
    const static float RAND_PROB;
    const static lev_t MAX_LEV;
    class iterator {
        friend class skiplist;
        Mpl *mpl;
        fptr ptr;
    public:
        iterator() = default;
        iterator(Mpl *_mpl, fptr _ptr = 0L): mpl(_mpl), ptr(_ptr) {}
        operator bool() {return bool(ptr) && bool(mpl); }
        _Key operator * () {node tmp; mpl->get(tmp, ptr); return tmp.key; }
        bool operator == (const iterator &o) {return ptr == o.ptr && mpl == o.mpl; }
        bool operator != (const iterator &o) {return ptr != o.ptr || mpl != o.mpl; }
        iterator& operator ++ () {
            node tmp; mpl->get(tmp, ptr); 
            ptr = tmp.level[0].forward; 
            return *this;
        }
        iterator& operator -- () {
            node tmp; mpl->get(tmp, ptr); 
            ptr = tmp.backward; 
            return *this; 
        }
        iterator operator ++ (int) {
            iterator ret(*this); 
            node tmp; mpl->get(tmp, ptr); 
            ptr = tmp.level[0].forward; 
            return ret; 
        }
        iterator operator -- (int) {
            iterator ret(*this); 
            node tmp; mpl->get(tmp, ptr); 
            ptr = tmp.backward; 
            return ret; 
        }
        ~iterator() = default;
    };
    friend class iterator;
    
    skiplist(const std::string &_name);
    skiplist(const skiplist &o) = delete;
    /* TO-DO:
     * template <typename Iter>
     * skiplist(const std::string &_name, const Iter _beg, const Iter _end);
     */

    iterator begin() {
        node tmp; mpl.get(tmp, list.head); 
        return iterator(&mpl, tmp.level[0].forward); 
    }
    iterator end() {return iterator(&mpl); }
    iterator rbegin() {return iterator(&mpl, list.tail); }
    iterator rend() {return iterator(&mpl, list.head); }
    
    siz_t size() {return list.len; }
    bool empty() {return list.len == 0; }
    void clear();

    iterator lower_bound(const _Key &nkey);
    iterator upper_bound(const _Key &nkey);
    iterator find(const _Key &nkey);
    std::pair<iterator, bool> insert(const _Key &nkey);
    iterator erase(const _Key &nkey);
    // TO-DO: iterator erase(const iterator it);
    // TO-DO: iterator erase(const iterator beg, const iterator end);
    siz_t order_of_key(const _Key &nkey);
    iterator find_by_order(siz_t rank);
    
    ~skiplist();
};

template <typename _Key, typename _Cmp>
const lev_t skiplist<_Key, _Cmp>::MAX_LEV = 16u;
template <typename _Key, typename _Cmp>
const float skiplist<_Key, _Cmp>::RAND_PROB = 0.25f;

template <typename _Key, typename _Cmp>
skiplist<_Key, _Cmp>::skiplist(const std::string &_name)
: mpl(_name + ".bin") {
    srand(time(0));
    if(mpl.create()) {
        list.len = 0;
        list.list_lev = 1;
        list.head = mpl.malloc(sizeof(node));
        list.tail = list.head;
        node head;
        head.backward = 0;
        for(int i = 0; i < MAX_LEV; ++i) {
            head.level[i].forward = 0;
            head.level[i].span = 0;
        }
        mpl.put(head, list.head);
    }
    else {
        mpl.get(list, 0);
    }
}

template <typename _Key, typename _Cmp>
lev_t skiplist<_Key, _Cmp>::getRandLev() {
    lev_t lev = 1;
    while(lev < MAX_LEV && (rand() & 0xffff) < (RAND_PROB * 0xffff)) lev++;
    return lev;
}

template <typename _Key, typename _Cmp>
void skiplist<_Key, _Cmp>::clear() {
    mpl.clear();
    list.len = 0;
    list.list_lev = 1;
    list.head = mpl.malloc(sizeof(node));
    list.tail = list.head;
    node head;
    head.backward = 0;
    for(int i = 0; i < MAX_LEV; ++i) {
        head.level[i].forward = 0;
        head.level[i].span = 0;
    }
    mpl.put(head, list.head);
}

template <typename _Key, typename _Cmp>
typename skiplist<_Key, _Cmp>::iterator
skiplist<_Key, _Cmp>::lower_bound(const _Key &nkey) {
    node nod1, *cur = &nod1; 
    node nod2, *nex = &nod2;
    mpl.get(*cur, list.head);
    for(int i = list.list_lev - 1; i >= 0; --i) {
        while(cur->level[i].forward) {
            mpl.get(*nex, cur->level[i].forward);
            if(!comp(nex->key, nkey)) break;
            std::swap(cur, nex);
        }
    }
    return iterator(&mpl, cur->level[0].forward);
} 

template <typename _Key, typename _Cmp>
typename skiplist<_Key, _Cmp>::iterator
skiplist<_Key, _Cmp>::upper_bound(const _Key &nkey) {
    node nod1, *cur = &nod1; 
    node nod2, *nex = &nod2;
    mpl.get(*cur, list.head);
    for(int i = list.list_lev - 1; i >= 0; --i) {
        while(cur->level[0].forward) {
            mpl.get(*nex, cur->level[0].forward);
            if(comp(nkey, nex->key)) break;
            std::swap(cur, nex);
        }
    }
    return iterator(&mpl, cur->level[0].forward);
} 

template <typename _Key, typename _Cmp>
typename skiplist<_Key, _Cmp>::iterator
skiplist<_Key, _Cmp>::find(const _Key &nkey) {
    node nod1, *cur = &nod1; 
    node nod2, *nex = &nod2;
    mpl.get(*cur, list.head);
    for(int i = list.list_lev - 1; i >= 0; --i) {
        while(cur->level[i].forward) {
            mpl.get(*nex, cur->level[i].forward);
            if(!comp(nex->key, nkey)) break;
            std::swap(cur, nex);
        }
    }
    if(!cur->level[0].forward) return end();
    mpl.get(*nex, cur->level[0].forward);
    if(comp(nkey, nex->key)) return end();
    return iterator(&mpl, cur->level[0].forward);
}

#include <iostream>
template <typename _Key, typename _Cmp>
std::pair<typename skiplist<_Key, _Cmp>::iterator, bool>
skiplist<_Key, _Cmp>::insert(const _Key &nkey) {
    node nod1, *cur = &nod1; 
    node nod2, *nex = &nod2;
    fptr upd[MAX_LEV], res;
    siz_t rank[MAX_LEV]; 
    memset(upd, 0, sizeof(upd));
    // locate the node with a greatest key less than $nkey
    mpl.get(*cur, list.head);
    res = list.head;
    for(int i = list.list_lev - 1; i >= 0; --i) {
        rank[i] = (i == list.list_lev - 1)? 0: rank[i + 1];
        while(cur->level[i].forward) {
            mpl.get(*nex, cur->level[i].forward);
            if(!comp(nex->key, nkey)) break;
            rank[i] += cur->level[i].span;
            res = cur->level[i].forward;
            std::swap(cur, nex);
        }
        upd[i] = res;
    }
    // check if there has exited a node with $nkey 
    if(cur->level[0].forward) {
        mpl.get(*nex, cur->level[0].forward);
        if(!comp(nkey, nex->key)) {
            return std::make_pair(iterator(&mpl, cur->level[0].forward), false);
        }
    }
    lev_t new_lev = getRandLev();
    // update @list_lev and initialize info for untouched levels
    if(new_lev > list.list_lev) {
        node header;
        mpl.get(header, list.head);
        for(int i = list.list_lev; i < new_lev; ++i) {
            rank[i] = 0;
            upd[i] = list.head;
            header.level[i].span = list.len;
        }
        mpl.put(header, list.head);
        list.list_lev = new_lev;
    }
    // create a new node
    fptr new_ptr = mpl.malloc(sizeof(node));
    node new_node;
    new_node.node_lev = new_lev;
    new_node.key = nkey;
    for(int i = 0; i < new_lev; ++i) {
        mpl.get(*cur, upd[i]);
        // update @forward for new_node and upd[i]
        new_node.level[i].forward = cur->level[i].forward;
        cur->level[i].forward = new_ptr;
        // update @span for new_node and upd[i]
        new_node.level[i].span = cur->level[i].span - (rank[0] - rank[i]);
        cur->level[i].span = (rank[0] - rank[i]) + 1;
        mpl.put(*cur, upd[i]);
    }
    // increase @span for untouched levels
    for(int i = new_lev; i < list.list_lev; ++i) {
        mpl.get(*cur, upd[i]);
        cur->level[i].span++;
        mpl.put(*cur, upd[i]);
    }
    // update @backward and @tail
    new_node.backward = upd[0];
    if(new_node.level[0].forward) {
        mpl.get(*nex, new_node.level[0].forward);
        nex->backward = new_ptr;
        mpl.put(*nex, new_node.level[0].forward);
    }
    else list.tail = new_ptr;
    mpl.put(new_node, new_ptr);
    list.len++;
    return std::make_pair(iterator(&mpl, new_ptr), true);
}

template <typename _Key, typename _Cmp>
typename skiplist<_Key, _Cmp>::iterator
skiplist<_Key, _Cmp>::erase(const _Key &nkey) {
    node nod1, *cur = &nod1; 
    node nod2, *nex = &nod2;
    fptr upd[MAX_LEV], res;
    memset(upd, 0, sizeof(upd));
    // locate the node with a greatest key less than $nkey
    mpl.get(*cur, list.head);
    res = list.head;
    for(int i = list.list_lev - 1; i >= 0; --i) {
        while(cur->level[i].forward) {
            mpl.get(*nex, cur->level[i].forward);
            if(!comp(nex->key, nkey)) break;
            res = cur->level[i].forward;
            std::swap(cur, nex);
        }
        upd[i] = res;
    }
    // chech if there exits a node with $nkey
    node del_node;
    fptr del_ptr = cur->level[0].forward;
    if(del_ptr) {
        mpl.get(del_node, del_ptr);
        if(comp(nkey, del_node.key)) return end();
    }
    else return end();
    // update @forward and @span for upd[i]
    for(int i = 0; i < list.list_lev; ++i) {
        mpl.get(*cur, upd[i]);
        if(cur->level[i].forward == del_ptr) {
            cur->level[i].span += del_node.level[i].span - 1;
            cur->level[i].forward = del_node.level[i].forward;
        }
        else {
            cur->level[i].span--;
        }
        mpl.put(*cur, upd[i]);
    }
    // update @backward and @tail
    fptr ret_ptr = del_node.level[0].forward;
    if(ret_ptr) {
        mpl.get(*nex, ret_ptr);
        nex->backward = del_node.backward;
        mpl.put(*nex, ret_ptr);
    }
    else {
        list.tail = del_node.backward;
    }
    // maintain @list.list_lev
    node header;
    mpl.get(header, list.head);
    while(list.list_lev > 1 && !header.level[list.list_lev - 1].forward) {
        list.list_lev--;
    }
    mpl.free(del_ptr);
    list.len--;
    return iterator(&mpl, ret_ptr);
}

template <typename _Key, typename _Cmp>
siz_t skiplist<_Key, _Cmp>::order_of_key(const _Key &nkey) {
    node nod1, *cur = &nod1;
    node nod2, *nex = &nod2;
    siz_t rank = 0;
    mpl.get(*cur, list.head);
    for(int i = list.list_lev - 1; i >= 0; --i) {
        while(cur->level[i].forward) {
            mpl.get(*nex, cur->level[i].forward);
            if(!comp(nex->key, nkey)) break;
            rank += cur->level[i].span;
            std::swap(cur, nex);
        }
    }
    return rank;
}

template <typename _Key, typename _Cmp>
typename skiplist<_Key, _Cmp>::iterator 
skiplist<_Key, _Cmp>::find_by_order(siz_t rank) {
    if(rank >= list.len) return end();
    node nod1, *cur = &nod1;
    node nod2, *nex = &nod2;
    mpl.get(*cur, list.head);
    for(int i = list.list_lev - 1; i >= 0; --i) {
        while(rank && cur->level[i].forward) {
            mpl.get(*nex, cur->level[i].forward);
            if(rank < cur->level[i].span) break; 
            rank -= cur->level[i].span;
            std::swap(cur, nex);
        }
    }
    return iterator(&mpl, cur->level[0].forward);
}

template <typename _Key, typename _Cmp>
skiplist<_Key, _Cmp>::~skiplist() {mpl.put(list, 0); }

}

#endif // _SKIPLIST_H_