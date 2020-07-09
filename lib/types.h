#ifndef TYPES_H
#define TYPES_H

#include <vector>

namespace onion
{
template <class T> using array_of = std::vector<T>;
template <class T> class matrix_of :
        public std::vector<array_of<T>>
{
public:
    matrix_of(size_t i, size_t j):std::vector<array_of<T>>(i,array_of<T>(j)),
        _rows(i),_cols(j){}
    void clear(){
        for(unsigned i = 0; i < _rows; ++i)
            for(unsigned j = 0; j < _cols; ++j)
                (*this)[i][j] = T();
    }
private:
    size_t _rows;
    size_t _cols;
};
}

#endif // TYPES_H
