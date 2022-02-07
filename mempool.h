#ifndef _FILE_MEMPOOL_H_
#define _FILE_MEMPOOL_H_ 1

#include <string>
#include <fstream>
#include <cassert>
#include <iostream>
#include <cstring>
#include <unordered_map>

namespace filebased {

typedef short fsiz;
typedef unsigned fptr;

typedef class FileMemoryPool {
    const static fptr RESERVE = 1u << 6;
    const static fsiz MAXHEAD = 1u << 7;
    const static fsiz UNITBIT = 2u;
    struct Header {
        fsiz _size;
        fptr _unused;
    } _header[MAXHEAD];
    
    struct Tag {
        fptr _next;
        fsiz _id;
    };

    std::fstream _file;
    std::string _file_name;
    bool _create;

private:
    inline fsiz _getid(fsiz _siz) {
        return (_siz - 1) >> UNITBIT;
    }
    inline fsiz _getsiz(fsiz _id) {
        return (_id + 1) << UNITBIT;
    }

public:
    FileMemoryPool(const std::string _name)
    : _file_name(_name), _file(_name) {
        if(!_file) {
            _create = 1;
            _file.close();
            _file.open(_name, std::ios::out);
            for(int i = 0; i < MAXHEAD; ++i) {
                _header[i]._unused = 0;
                _header[i]._size = _getsiz(i);
            }
            _file.seekp(RESERVE);
            _file.write(reinterpret_cast<const char *>(&_header), sizeof(_header));
        }
        else {
            _create = 0;
            _file.seekg(RESERVE);
            _file.read(reinterpret_cast<char *>(&_header), sizeof(_header));
        }
        _file.close();
        _file.open(_name, std::ios::binary | std::ios::in | std::ios::out);
    }

    std::string filename() {return _file_name; }
    bool create() {return _create; }

    fptr malloc(fsiz _req_siz) {
        fsiz _id = _getid(_req_siz);
        Header &_hd = _header[_id];
        fptr _target = _hd._unused;
        if(_target) {
            Tag _tag;
            _file.seekg(_target);
            _file.read(reinterpret_cast<char *>(&_tag), sizeof(_tag));
            _hd._unused = _tag._next;
            return _target + sizeof(Tag);
        }
        else {
            Tag _tag = (Tag) {._next = 0, ._id = _id};
            _file.seekp(0L, std::ios::end);
            _file.write(reinterpret_cast<const char *>(&_tag), sizeof(_tag));
            fptr _ret = _file.tellp();
            _file.seekp(_hd._size - 1, std::ios::cur);
            char _placeholder;
            _file.write(reinterpret_cast<const char *>(&_placeholder), 1);
            return _ret;
        }
    }

    void free(fptr _ptr) {
        Tag _tag;
        fptr _tagptr = _ptr - sizeof(Tag);
        _file.seekg(_tagptr);
        _file.read(reinterpret_cast<char *>(&_tag), sizeof(_tag));
        Header &_hd = _header[_tag._id];
        _tag._next = _hd._unused;
        _hd._unused = _tagptr;
        _file.seekp(_tagptr);
        _file.write(reinterpret_cast<const char *>(&_tag), sizeof(_tag));
    }

    template <typename T>
    void get(T &_obj, fptr _ptr) {
        _file.seekg(_ptr);
        _file.read(reinterpret_cast<char *>(&_obj), sizeof(_obj));
    }

    template <typename T>
    void put(T &_obj, fptr _ptr) {
        _file.seekp(_ptr);
        _file.write(reinterpret_cast<char *>(&_obj), sizeof(_obj));
    }

    void clear() {
        _file.close();
        _file.open(_file_name, std::ios::trunc | std::ios::out);
        for(int i = 0; i < MAXHEAD; ++i) {
            _header[i]._unused = 0;
            _header[i]._size = _getsiz(i);
        }
        _file.seekp(RESERVE);
        _file.write(reinterpret_cast<const char *>(&_header), sizeof(_header));
        _file.close();
        _file.open(_file_name, std::ios::binary | std::ios::in | std::ios::out);
    }
    
    ~FileMemoryPool() {
        _file.seekp(RESERVE);
        _file.write(reinterpret_cast<const char *>(&_header), sizeof(_header));
        _file.close();
    }
} Mpl;

}

#endif // !_FILE_MEMPOOL_H_