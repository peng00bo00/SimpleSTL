#pragma once

#include <cstdio>
#include <utility>
#include <concepts>

namespace SimpleSTL
{

// DefaultDeleter
template <typename T>
struct DefaultDeleter {
    void operator() (T *p) const {
        delete p;
    }
};

template <typename T>
struct DefaultDeleter<T[]> {
    void operator() (T *p) const {
        delete[] p;
    }
};

template <>
struct DefaultDeleter<FILE> {
    void operator()(FILE *p) const {
        fclose(p);
    }
};

// exchange
template <typename T, typename U>
T exchange(T &dst, U &&val) {
    T tmp = std::move(dst);
    dst = std::forward<U>(val);

    return tmp;
}

// UniquePtr
template <typename T, typename Deleter = DefaultDeleter<T>>
struct UniquePtr {
private:
    T *m_p;

    template <typename U, typename UDeleter>
    friend struct UniquePtr;

public:
    UniquePtr(std::nullptr_t dummy = nullptr) {
        m_p = nullptr;
    }

    explicit UniquePtr(T *p) {
        m_p = p;
    }

    // template <class U, class UDeleter, class = std::enable_if_t<std::is_convertible_v<U *, T *>>> // 没有 C++20 的写法
    template <class U, class UDeleter> requires (std::convertible_to<U *, T *>) // 有 C++20 的写法
    UniquePtr(UniquePtr<U, UDeleter> &&that) {  // 从子类型U的智能指针转换到T类型的智能指针
        m_p = exchange(that.m_p, nullptr);
    }

    ~UniquePtr() {
        if (m_p) Deleter{}(m_p);
    }

    // copy constructor
    UniquePtr(UniquePtr const & that) = delete;

    // copy assignment
    UniquePtr &operator= (UniquePtr const &that) = delete;

    // move constructor
    UniquePtr(UniquePtr &&that) {
        m_p = exchange(that.m_p, nullptr);
    }

    // move assignment
    UniquePtr &operator=(UniquePtr &&that) {
        if (this != &that) [[likely]] {
            if (m_p)
                Deleter{}(m_p);
            m_p = exchange(that.m_p, nullptr);
        }
        return *this;
    }

    // get()
    T *get() const {
        return m_p;
    }

    // release()
    T *release() const {
        return exchange(m_p, nullptr);
    }

    // reset()
    void reset(T *p = nullptr) {
        if (m_p) Deleter{}(m_p);

        m_p = p;
    }

    // * operator shortcut
    T &operator*() const {
        return *m_p;
    }

    // -> operator shortcut
    T *operator->() const {
        return m_p;
    }
};

template <typename T, typename Deleter>
struct UniquePtr<T[], Deleter> : UniquePtr<T, Deleter> {};

// makeUnique()
template <typename T, typename ...Args>
UniquePtr<T> makeUnique(Args &&...args) {
    return UniquePtr<T>(new T(std::forward<Args>(args)...));
}

// makeUniqueForOverwrite()
template <typename T>
UniquePtr<T> makeUniqueForOverwrite() {
    return UniquePtr<T>(new T);
}

}