#pragma once
#include <cstddef>
#include <utility>
#include <algorithm>
#include <array>

template<typename T>
class SharedPtr {
private:
    struct ControlBlockBase {
        size_t shared_count;
        size_t weak_count;

        ControlBlockBase() : shared_count(0), weak_count(0) {}
        virtual ~ControlBlockBase() = default;
        virtual T* GetObject() const = 0;
        virtual void DestroyObject() noexcept = 0;
    };

    class ControlBlockPtr : public ControlBlockBase {
    private:
        T* ptr_;

    public:
        explicit ControlBlockPtr(T* ptr) : ptr_(ptr) {}
        T* GetObject() const override {
             return ptr_;
    }
        void DestroyObject() noexcept override{
            delete ptr_;
        }
    };

    class ControlBlockMakeShared : public ControlBlockBase {
    private:
        mutable std::array<char, sizeof(T)> t_;
    public:
        template<typename... Args>
        explicit ControlBlockMakeShared(Args&&... args){
            new (reinterpret_cast<T*>(t_.data())) T(std::forward<Args>(args)...);
        }
        T* GetObject() const override {
            return reinterpret_cast<T*>(t_.data());
        }
        void DestroyObject() noexcept override{
            GetObject()->~T();
        }
    };

private:
    ControlBlockBase* block_ptr_;

    template<typename... Args>
    SharedPtr(Args&&... args)
        : block_ptr_(new ControlBlockMakeShared(std::forward<Args>(args)...)) {
        block_ptr_->shared_count = 1;
    }

    explicit SharedPtr(ControlBlockBase* block) : block_ptr_(block) {}

    void UnconnectSharedPtr() {
        if(!block_ptr_){
            return;
        }
        --block_ptr_->shared_count;
        if(block_ptr_->shared_count > 0){
            return;
        }
        block_ptr_->DestroyObject();
        if(block_ptr_->weak_count == 0){
            delete block_ptr_;
        }
    }

public:
    SharedPtr() : block_ptr_(nullptr) {}

    explicit SharedPtr(T* ptr) : block_ptr_(new ControlBlockPtr(ptr)) {
        if (block_ptr_) {
            block_ptr_->shared_count = 1;
        }
    }

    SharedPtr(const SharedPtr& other) : block_ptr_(other.block_ptr_) {
        if (block_ptr_) {
            ++block_ptr_->shared_count;
        }
    }

    SharedPtr(SharedPtr&& other) : block_ptr_(other.block_ptr_) {
        other.block_ptr_ = nullptr;
    }

    ~SharedPtr() { UnconnectSharedPtr(); }

    SharedPtr& operator=(const SharedPtr& other) {
        if (this != &other) {
            SharedPtr buffer(other);
            Swap(buffer);
        }
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) {
        if (this != &other) {
            Swap(other);
        }
        return *this;
    }

    void Swap(SharedPtr& other) {
        std::swap(block_ptr_, other.block_ptr_);
    }

    T* Get() const {
        return block_ptr_ ? block_ptr_->GetObject() : nullptr;
    }

    T* operator->() const { return Get(); }

    T& operator*() const { return *(Get()); }

    explicit operator bool() const { return Get() != nullptr; }

    size_t UseCount() const { return block_ptr_ ? block_ptr_->shared_count : 0; }

    void Reset() noexcept { SharedPtr{}.Swap(*this); }

    void Reset(T* p) {
        SharedPtr tmp(p);
        Swap(tmp);
    }

    template <class... Args>
    static SharedPtr MakeShared(Args&&... args) {
        return SharedPtr(std::forward<Args>(args)...);
    }
};

template<typename T>
class WeakPtr {
    template<typename>
    friend class SharedPtr;
    private:
        typename SharedPtr<T>::ControlBlockBase* block_ptr_;
    private:
        void UnconnectWeakPtr() {
            if (!block_ptr_) {
                return;
            }
            --block_ptr_->weak_count;
            if (block_ptr_->weak_count > 0) {
                return;
            }
            if (block_ptr_->weak_count == 0 && block_ptr_->shared_count == 0) {
                delete block_ptr_;
            }
        }
    public:
        WeakPtr() : block_ptr_(nullptr) {}

        WeakPtr(const WeakPtr& other) : block_ptr_(other.block_ptr_) {
            if (block_ptr_) {
                ++block_ptr_->weak_count;
            }
        }
        WeakPtr(WeakPtr&& other) : block_ptr_(other.block_ptr_) {
            other.block_ptr_ = nullptr;
        }

        WeakPtr(const SharedPtr<T>& shptr) : block_ptr_(shptr.block_ptr_) {
            if (block_ptr_) {
                ++block_ptr_->weak_count;
            }
        }

        ~WeakPtr() { UnconnectWeakPtr(); }

        WeakPtr& operator=(const WeakPtr& other) {
            if (this != &other) {
                WeakPtr buffer(other);
                Swap(buffer);
            }
            return *this;
        }

        WeakPtr& operator=(WeakPtr&& other) {
            if (this != &other) {
                Swap(other);
            }
            return *this;
        }

        void Swap(WeakPtr& other) { std::swap(block_ptr_, other.block_ptr_); }

        void Reset() { UnconnectWeakPtr(); }

        size_t UseCount() const {
            if (block_ptr_ == nullptr) {
                return 0;
            }
            return block_ptr_->shared_count;
        }

        bool Expired() const { return UseCount() == 0; }

        SharedPtr<T> Lock() const {
            if (block_ptr_ != nullptr && block_ptr_->shared_count > 0) {
                SharedPtr<T> result;
                result.block_ptr_ = block_ptr_;
                ++block_ptr_->shared_count;
                return result;
            }
            return SharedPtr<T>();
        }
    };

template <typename T>
class UniquePtr {
private:
    T* ptr_;
public:
    UniquePtr() noexcept : ptr_(nullptr) {}
    explicit UniquePtr(T* ptr) noexcept : ptr_(ptr) {}

    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    UniquePtr(UniquePtr&& other) noexcept : ptr_(other.ptr_) {
        other.ptr_ = nullptr;
    }

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        Reset(other.Release());
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) noexcept {
        Reset();
        return *this;
    }

    ~UniquePtr() {
        delete ptr_;
    }

    T* Release() noexcept {
        T* temp = ptr_;
        ptr_ = nullptr;
        return temp;
    }

    void Reset(T* new_ptr = nullptr) noexcept {
        T* old_ptr = ptr_;
        ptr_ = new_ptr;
        if (old_ptr) {
            delete old_ptr;
        }
    }

    void Swap(UniquePtr& other) noexcept {
        std::swap(ptr_, other.ptr_);
    }

    T* Get() const noexcept { return ptr_; }
    T& operator*() const noexcept { return *ptr_; }
    T* operator->() const noexcept { return ptr_; }
    explicit operator bool() const noexcept { return ptr_ != nullptr; }

    template<typename ...Args>
    static UniquePtr MakeUnique(Args&&... args) {
        return UniquePtr(new T(std::forward<Args>(args)...));
    }
};

