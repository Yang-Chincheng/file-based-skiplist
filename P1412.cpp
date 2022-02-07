#include <bits/stdc++.h>
using namespace std;

#include "fset.h"
using namespace filebased;

struct Data {
    char str[65];
    int val;
    friend bool operator < (const Data &lhs, const Data &rhs) {
        auto tmp = strcmp(lhs.str, rhs.str);
        return tmp < 0 || (tmp == 0 && lhs.val < rhs.val);
    }
};

fset<Data> fs("P1412");

int main() {
    int n; cin >> n;
    while(n--) {
        Data res;
        char opt[10];
        cin >> opt >> res.str;
        if(strcmp(opt, "insert") == 0) {
            cin >> res.val;
            fs.insert(res);
        }
        if(strcmp(opt, "delete") == 0) {
            cin >> res.val;
            fs.erase(res);
        }
        if(strcmp(opt, "find") == 0) {
            res.val = INT32_MIN;
            auto st = fs.lower_bound(res);
            res.val = INT32_MAX;
            auto ed = fs.upper_bound(res);
            for(auto it = st; it != ed; ++it) {
                cout << (*it).val << " ";
            }
            if(st == ed) cout << "null";
            cout << "\n";
        }
    }
    return 0;
}
