#ifndef HF_OPTIONALM_TEST_COMMON_H
#define HF_OPTIONALM_TEST_COMMON_H

#include <type_traits>
#include <utility>

namespace testing {

template <typename V>
struct ctor_count {
    V value;

    template <typename... Args, typename = typename std::enable_if<std::is_constructible<V,Args...>::value>::type>
    ctor_count(Args&&... args): value(std::forward<Args>(args)...) {}

    ctor_count(const ctor_count& n): value(n.value) {
        ++copy_ctor_count;
    }

    ctor_count(ctor_count&& n): value(std::move(n.value)) {
        ++move_ctor_count;
    }

    ctor_count &operator=(const ctor_count& n) {
        value=n.value;
        ++copy_assign_count;
        return *this;
    }

    ctor_count &operator=(ctor_count&& n) {
        value=std::move(n.value);
        ++move_assign_count;
        return *this;
    }

    static int copy_ctor_count, copy_assign_count;
    static int move_ctor_count, move_assign_count;

    static void reset_counts() {
        copy_ctor_count=copy_assign_count=0;
        move_ctor_count=move_assign_count=0;
    }
};

template <typename V> int ctor_count<V>::copy_ctor_count;
template <typename V> int ctor_count<V>::copy_assign_count;
template <typename V> int ctor_count<V>::move_ctor_count;
template <typename V> int ctor_count<V>::move_assign_count;

template <typename V>
struct no_copy {
    V value;

    template <typename... Args, typename = typename std::enable_if<std::is_constructible<V,Args...>::value>::type>
    no_copy(Args&&... args): value(std::forward<Args>(args)...) {}

    no_copy(const no_copy& n)=delete;
    no_copy &operator=(const no_copy& n)=delete;

    no_copy(no_copy&& n): value(std::move(n.value)) {
        ++move_ctor_count;
    }

    no_copy &operator=(no_copy&& n) {
        value=std::move(n.value);
        ++move_assign_count;
        return *this;
    }

    static int move_ctor_count, move_assign_count;

    static void reset_counts() {
        move_ctor_count=move_assign_count=0;
    }
};

template <typename V> int no_copy<V>::move_ctor_count;
template <typename V> int no_copy<V>::move_assign_count;

template <typename V>
struct no_move {
    V value;

    template <typename... Args, typename = typename std::enable_if<std::is_constructible<V,Args...>::value>::type>
    no_move(Args&&... args): value(std::forward<Args>(args)...) {}

    no_move(const no_move& n): value(n.value) {
        ++copy_ctor_count;
    }

    no_move &operator=(const no_move& n) {
        value=n.value;
        ++copy_assign_count;
        return *this;
    }

    static int copy_ctor_count, copy_assign_count;

    static void reset_counts() {
        copy_ctor_count=copy_assign_count=0;
    }
};

template <typename V> int no_move<V>::copy_ctor_count;
template <typename V> int no_move<V>::copy_assign_count;

}

#endif // HF_OPTIONALM_TEST_COMMON_H
