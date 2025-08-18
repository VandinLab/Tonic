#ifndef FIXEDSIZEPQ_H
#define FIXEDSIZEPQ_H

#include <iostream>
#include <vector>
#include <algorithm>

template<typename T, typename Compare = std::less<T>>
class FixedSizePQ {

public:

    FixedSizePQ() : max_size(0) {}
    FixedSizePQ(size_t max_size) : max_size(max_size) {
        heap_.reserve(max_size);
    }

    typedef typename std::vector<T>::iterator iterator;
    iterator begin() { return heap_.begin(); }
    iterator end() { return heap_.end(); }

    inline bool empty() const { return heap_.empty(); }

    inline const size_t size() const { return heap_.size(); }

    inline const T& top() const { return heap_.front(); }

    inline void enlarge_max_size(size_t new_max_size) { max_size = new_max_size; }

    void push(const T& elem) {
        heap_.push_back(elem);
        std::push_heap(heap_.begin(), heap_.end(), comp);
    }

    void pop() {
        std::pop_heap(heap_.begin(), heap_.end(), comp);
        heap_.pop_back();
    }

protected:
    std::vector<T> heap_;
    size_t max_size;
    Compare comp;

private:
    void* operator new (size_t);
    void* operator new[] (size_t);
    void operator delete (void *);
    void operator delete[] (void *);

};

#endif //FIXEDSIZEPQ_H
