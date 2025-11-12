#pragma once
#include "ArraySeq.h"
#include <algorithm>
#include <cstddef>
#include <functional>
#include <utility>

template <typename T, class Cmp = std::less<T>>
class BinaryHeap {
private:
    ArraySeq<T> data_;
    Cmp cmp_;
private:


    void SiftUp(size_t idx) {
        while (idx > 0) {
            size_t parent = (idx - 1) / 2;
            if (!cmp_(data_[idx], data_[parent]))
                break;
            std::swap(data_[idx], data_[parent]);
            idx = parent;
        }
    }

    void SiftDown(size_t idx) {
        size_t n = data_.size();
        while (true) {
            size_t left = 2 * idx + 1;
            size_t right = 2 * idx + 2;
            size_t smallest = idx;

            if (left < n && cmp_(data_[left], data_[smallest])) smallest = left;
            if (right < n && cmp_(data_[right], data_[smallest])) smallest = right;

            if (smallest == idx) break;
            std::swap(data_[idx], data_[smallest]);
            idx = smallest;
        }
    }

public:
    explicit BinaryHeap(Cmp cmp = Cmp()) : cmp_(std::move(cmp)) {}

    const T& Top() const { 
        if (data_.empty()) throw std::out_of_range("Heap is empty");
        return data_.front(); 
    }
    T& Top() { 
        if (data_.empty()) throw std::out_of_range("Heap is empty");
        return data_.front(); 
    }

    template<typename U>
    void Push(U&& t){
        data_.push_back(std::forward<U>(t));
        SiftUp(data_.size() - 1);
    }

    template<typename ... Args>
    void Emplace(Args&&... args){
        data_.emplace_back(std::forward<Args>(args)...);
        SiftUp(data_.size() - 1);
    }

    void Pop(){
        if(data_.empty()) throw std::out_of_range("Pop called on empty heap");
        if(data_.size() == 1){
            data_.pop_back();
            return;
        }
        std::swap(data_.front(), data_.back());
        data_.pop_back();
        SiftDown(0);
    }
    
    bool Empty() const {
        return data_.empty();
    }

    size_t Size() const {
        return data_.size();
    }

    template<typename Seq>
    void Heapify(Seq&& arrseq){
        data_ = std::forward<Seq>(arrseq);
        for(size_t i = data_.size()/2; i > 0; --i){
            SiftDown(i - 1);
        }
    }
};
