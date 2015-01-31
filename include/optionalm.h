/**
 * \file optionalm.h
 * \brief An option class with a monadic interface.
 *
 * The std::option<T> class was proposed for inclusion into C++14, but was
 * ultimately rejected. (See N3672 proposal for details.) This class offers
 * similar functionality, namely a class that can represent a value (or
 * reference), or nothing at all.
 *
 * In addition, this class offers monadic and monoidal bindings, allowing
 * the chaining of operations any one of which might represent failure with
 * an unset optional value.
 *
 * One point of difference between the proposal N3672 and this implementation
 * is the lack of constexpr versions of the methods and constructors.
 */

#ifndef HF_OPTIONALM_H_
#define HF_OPTIONALM_H_

#include <type_traits>
#include <stdexcept>
#include <utility>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlogical-op-parentheses"

namespace hf {

template <typename X> struct optional;

struct optional_unset_error: std::runtime_error {
    explicit optional_unset_error(const std::string &what_str): std::runtime_error(what_str) {}
    optional_unset_error(): std::runtime_error("optional value unset") {}
};

struct optional_invalid_dereference: std::runtime_error {
    explicit optional_invalid_dereference(const std::string &what_str): std::runtime_error(what_str) {}
    optional_invalid_dereference(): std::runtime_error("derefernce of optional<void> value") {}
};

namespace optional_detail {
    template <typename Y> struct lift_type { typedef optional<Y> type; };
    template <typename Y> struct lift_type<optional<Y>> { typedef optional<Y> type; };

    struct optional_tag {};

    template <typename X> struct is_optional {
        enum {value=std::is_base_of<optional_tag,typename std::decay<X>::type>::value };
    };

    template <typename D,typename X> struct wrapped_type_ { typedef X type; };
    template <typename D,typename X> struct wrapped_type_<optional<D>,X> { typedef D type; };

    template <typename X> struct wrapped_type { typedef typename wrapped_type_<typename std::decay<X>::type,X>::type type; };

    template <typename X>
    struct optional_value_data {
        typename std::aligned_storage<sizeof(X),alignof(X)>::type data;

        typedef X *pointer_type;
        typedef const X *const_pointer_type;
        typedef X &reference_type;
        typedef const X &const_reference_type;

        optional_value_data() {}
        
        template <typename T,typename std::enable_if<
            std::is_same<T,X>::value &&
            std::is_copy_constructible<T>::value>::type *_=nullptr>
        optional_value_data(bool set,const T &x) { if (set) construct(x); }

        template <typename T,typename std::enable_if<
            std::is_same<T,X>::value &&
            std::is_move_constructible<T>::value>::type *_=nullptr>
        optional_value_data(bool set,T &&x) { if (set) construct(std::move(x)); }

        X *ptr() { return reinterpret_cast<X *>(&data); }
        const X *cptr() const { return reinterpret_cast<const X *>(&data); }

        X &ref() { return *reinterpret_cast<X *>(&data); }
        const X &cref() const { return *reinterpret_cast<const X *>(&data); }

        template <typename T,typename std::enable_if<
            std::is_same<T,X>::value &&
            std::is_copy_constructible<T>::value>::type *_=nullptr>
        void construct(const T &x) { new(&data) X(x); }

        template <typename T,typename std::enable_if<
            std::is_same<T,X>::value &&
            std::is_move_constructible<T>::value>::type *_=nullptr>
        void construct(T &&x) { new(&data) X(std::move(x)); }

        void destruct() { ptr()->~X(); }
    };

    template <typename X>
    struct optional_ref_data {
        X *data;

        typedef X *pointer_type;
        typedef const X *const_pointer_type;
        typedef X &reference_type;
        typedef const X &const_reference_type;

        optional_ref_data() {}

        optional_ref_data(bool set,X &x): data(&x) {}

        X *ptr() { return data; }
        const X *cptr() const { return data; }

        X &ref() { return *data; }
        const X &cref() const { return *data; }

        void destruct() {}
    };
}

template <typename X>
struct optional: optional_detail::optional_tag {
    typedef X enclosed_type;

private:
    bool set;
    typedef typename std::conditional<std::is_lvalue_reference<X>::value,
        optional_detail::optional_ref_data<typename std::remove_reference<X>::type>,
        optional_detail::optional_value_data<X>>::type data_type;
    data_type data;

public:
    typedef typename data_type::pointer_type pointer_type;
    typedef typename data_type::const_pointer_type const_pointer_type;
    typedef typename data_type::reference_type reference_type;
    typedef typename data_type::const_reference_type const_reference_type;

    optional(): set(false) {}

    // non-reference constructors
 
    template <typename T,typename std::enable_if<
        std::is_same<T,X>::value &&
        !std::is_reference<T>::value &&
        std::is_copy_constructible<T>::value>::type *_=nullptr>
    optional(const optional<T> &o): set(o.set),data(set,o.ref()) {}
        
    template <typename T,typename std::enable_if<
        std::is_same<T,X>::value &&
        !std::is_reference<T>::value &&
        std::is_copy_constructible<T>::value>::type *_=nullptr>
    optional(const T &x): set(true),data(set,x) {}

    template <typename T,typename std::enable_if<
        std::is_same<T,X>::value &&
        !std::is_reference<T>::value &&
        std::is_move_constructible<T>::value>::type *_=nullptr>
    optional(optional<T> &&o): set(o.set),data(set,std::move(o.ref())) {}

    template <typename T,typename std::enable_if<
        std::is_same<T,X>::value &&
        !std::is_reference<T>::value &&
        std::is_move_constructible<T>::value>::type *_=nullptr>
    optional(T &&x): set(true),data(set,std::move(x)) {}

    // value-converting constructors

    template <typename T,typename std::enable_if<
        std::is_convertible<T,X>::value &&
        !std::is_same<T,X>::value &&
        !std::is_reference<X>::value &&
        std::is_copy_constructible<X>::value>::type *_=nullptr>
    optional(const optional<T> &o): set(o) { if (set) data.construct(static_cast<X>(o.get())); }
        
    template <typename T,typename std::enable_if<
        std::is_convertible<T,X>::value &&
        !std::is_same<T,X>::value &&
        !std::is_reference<X>::value &&
        std::is_copy_constructible<X>::value>::type *_=nullptr>
    optional(const T &t): set(true),data(set,static_cast<X>(t)) {}

    template <typename T,typename std::enable_if<
        std::is_convertible<T,X>::value &&
        !std::is_same<T,X>::value &&
        !std::is_reference<X>::value &&
        std::is_move_constructible<X>::value>::type *_=nullptr>
    optional(optional<T> &&o): set(o) { if (set) data.construct(static_cast<X>(std::move(o.get()))); }

    template <typename T,typename std::enable_if<
        std::is_same<T,X>::value &&
        !std::is_same<T,X>::value &&
        !std::is_reference<T>::value &&
        std::is_move_constructible<T>::value>::type *_=nullptr>
    optional(T &&x): set(true),data(set,static_cast<X>(std::move(x))) {}

    // reference constructors

    template <typename T,typename std::enable_if<
        std::is_same<T,reference_type>::value &&
        std::is_reference<T>::value>::type *_=nullptr>
    optional(optional<T> &o): set(o.set),data(set,o.ref()) {}
        
    template <typename T,typename std::enable_if<
        std::is_same<T,typename std::remove_volatile<typename std::remove_reference<X>::type>::type>::value &&
        std::is_reference<X>::value>::type *_=nullptr>
    optional(T &x): set(true),data(set,x) {}

    ~optional() {
        if (set) destruct();
    }

    template <typename Y,typename std::enable_if<
        std::is_assignable<X,Y>::value &&
        std::is_constructible<X,Y>::value>::type *_=nullptr>
    optional &operator=(Y &&y) {
        if (set) ref()=std::forward(y);
        else {
            set=true;
            construct(std::forward(y));
        }
        return *this;
    }

    // assignment

    template <typename Y,typename std::enable_if<
        std::is_assignable<X,Y>::value &&
        std::is_constructible<X,Y>::value>::type *_=nullptr>
    optional &operator=(const optional<Y> &o) {
        if (set) {
            if (o.set) ref()=o.ref();
            else {
                destruct();
                set=false;
            }
        }
        else {
            set=o.set;
            if (set) construct(X(o.ref()));
        }
        return *this;
    }

    template <typename Y,typename std::enable_if<
        std::is_assignable<X,Y &&>::value &&
        std::is_constructible<X,Y &&>::value>::type *_=nullptr>
    optional &operator=(optional<Y> &&o) {
        if (set) {
            if (o.set) ref()=std::move(o.ref());
            else {
                destruct();
                set=false;
            }
        }
        else {
            set=o.set;
            if (set) construct(std::move(o.ref()));
        }
        return *this;
    }

    void reset() {
        if (set) destruct();
        set=false;
    }

    const_pointer_type operator->() const { return ptr(); }
    pointer_type operator->() { return ptr(); }
        
    const_reference_type operator*() const { return ref(); }
    reference_type operator*() { return ref(); }
        
    reference_type get() {
        if (set) return ref();
        else throw optional_unset_error();
    }

    const_reference_type get() const {
        if (set) return ref();
        else throw optional_unset_error();
    }

    explicit operator bool() const { return set; }

    template <typename Y>
    bool operator==(const Y &y) const { return set && ref()==y; }

    template <typename Y>
    bool operator==(const optional<Y> &o) const {
        return set && o.set && ref()==o.ref() || !set && !o.set;
    }

    // monadic interface

private:
    template <typename F,typename Ref,typename R,bool F_void_return>
    struct bind_impl {
        static R bind(F &&f,Ref r) { return R(f(r)); }
    };

    template <typename F,typename Ref,typename R>
    struct bind_impl<F,Ref,R,true> {
        static R bind(F &&f,Ref r) { f(r); return R(true); }
    };

public:
    template <typename F>
    typename optional_detail::lift_type<typename std::result_of<F(typename std::decay<reference_type>::type)>::type>::type bind(F &&f) {
        typedef typename std::result_of<F(typename std::decay<reference_type>::type)>::type F_result_type;
        typedef typename optional_detail::lift_type<F_result_type>::type result_type;

        if (!set) return result_type();
        else return bind_impl<F,reference_type,result_type,std::is_same<F_result_type,void>::value>::bind(std::forward<F>(f),ref());
    }

    template <typename F>
    typename optional_detail::lift_type<typename std::result_of<F(typename std::decay<reference_type>::type)>::type>::type operator>>(F &&f) {
        return this->bind(std::forward<F>(f));
    }

private:
    pointer_type ptr() { return data.ptr(); }
    const pointer_type ptr() const { return data.cptr(); }

    reference_type ref() { return data.ref(); }
    const_reference_type ref() const { return data.cref(); }

    void destruct() { data.destruct(); }

    template <typename Y>
    void construct(Y &&y) { data.construct(std::forward<Y>(y)); }
};

/* special case for optional<void>, used as e.g. the result of
 * binding to a void function */

template <>
struct optional<void>: optional_detail::optional_tag {
    typedef void enclosed_type;

private:
    bool set;

public:
    typedef void *pointer_type;
    typedef void reference_type;

    optional(): set(false) {}
    optional(const optional<void> &o): set(o.set) {}

    template <typename T>
    explicit optional(T): set(true) {}

    template <typename T>
    optional &operator=(T) { set=true; }

    template <typename T>
    optional &operator=(const optional<T> &o) { set=o.set; return *this; }

    template <typename T>
    optional &operator=(optional<T> &&o) { set=o.set; return *this; }

    void reset() { set=false; }

    const pointer_type operator->() const { return nullptr; }
    pointer_type operator->() { return nullptr; }
        
    const reference_type operator*() const { throw optional_invalid_dereference(); }
    reference_type operator*() { throw optional_invalid_dereference(); }
        
    reference_type get() {
        if (*this) return **this;
        else throw optional_unset_error();
    }

    const reference_type get() const {
        if (*this) return **this;
        else throw optional_unset_error();
    }

    explicit operator bool() const { return set; }

    template <typename Y>
    bool operator==(const Y &y) const { return false; }

    bool operator==(const optional<void> &o) const {
        return set && o.set || !set && !o.set;
    }

    // monadic interface

    template <typename F>
    typename optional_detail::lift_type<typename std::result_of<F()>::type>::type bind(F &&f) {
        typedef typename optional_detail::lift_type<typename std::result_of<F()>::type>::type result_type;

        if (!set) return result_type();
        else return result_type(f());
    }

    template <typename F>
    typename optional_detail::lift_type<typename std::result_of<F()>::type>::type operator>>(F &&f) {
        return this->bind(f);
    }
};


template <typename A,typename B>
typename std::enable_if<
    optional_detail::is_optional<A>::value || optional_detail::is_optional<B>::value,
    optional<typename std::common_type<typename optional_detail::wrapped_type<A>::type,typename optional_detail::wrapped_type<B>::type>::type>
>::type
operator|(A &&a,B &&b) {
    typedef typename std::common_type<typename optional_detail::wrapped_type<A>::type,typename optional_detail::wrapped_type<B>::type>::type common;
    return a?a:b;
}

template <typename A,typename B>
typename std::enable_if<
    optional_detail::is_optional<A>::value || optional_detail::is_optional<B>::value,
    optional<typename optional_detail::wrapped_type<B>::type>
>::type
operator&(A &&a,B &&b) {
    typedef optional<typename optional_detail::wrapped_type<B>::type> result_type;
    return a?b:result_type();
}

inline optional<void> provided(bool condition) { return condition?optional<void>(true):optional<void>(); }

} // namespace hf

#pragma clang diagnostic pop

#endif // ndef HF_OPTIONALM_H_
