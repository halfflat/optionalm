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

namespace detail {
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

        X *ptr() { return reinterpret_cast<X *>(&data); }
        const X *cptr() const { return reinterpret_cast<const X *>(&data); }

        X &ref() { return *reinterpret_cast<X *>(&data); }
        const X &cref() const { return *reinterpret_cast<const X *>(&data); }

        void construct(const X &x) { new(&data) X(x); }
        void construct(X &&x) { new(&data) X(std::move(x)); }
        void destruct() { ptr()->~X(); }

        template <typename F>
        typename std::result_of<F(reference_type)>::type apply(F &&f) { return f(ref()); }
        template <typename F>
        typename std::result_of<F(const_reference_type)>::type apply(F &&f) const { return f(cref()); }
    };

    template <typename X>
    struct optional_ref_data {
        X *data;

        typedef X *pointer_type;
        typedef const X *const_pointer_type;
        typedef X &reference_type;
        typedef const X &const_reference_type;

        X *ptr() { return data; }
        const X *cptr() const { return data; }

        X &ref() { return *data; }
        const X &cref() const { return *data; }

        void construct(X &x) { data=&x; }
        void destruct() {}

        template <typename F>
        typename std::result_of<F(reference_type)>::type apply(F &&f) { return f(ref()); }
        template <typename F>
        typename std::result_of<F(const_reference_type)>::type apply(F &&f) const { return f(cref()); }
    };

    struct optional_void_data {
        typedef void *pointer_type;
        typedef const void *const_pointer_type;
        typedef void reference_type;
        typedef void const_reference_type;

        void *ptr() { return nullptr; }
        const void *cptr() const { return nullptr; }

        void ref() { throw optional_invalid_dereference(); }
        void cref() const { throw optional_invalid_dereference(); }

        template <typename X>
        void construct(X &&) {}
        void destruct() {}

        template <typename F>
        typename std::result_of<F()>::type apply(F &&f) const { return f(); }
    };

    template <typename D>
    struct optional_base: detail::optional_tag {
        template <typename X> friend struct optional;

        typedef typename D::reference_type reference_type;
        typedef typename D::const_reference_type const_reference_type;
        typedef typename D::pointer_type pointer_type;
        typedef typename D::const_pointer_type const_pointer_type;

    protected:
        bool set;
        D data;

        optional_base(): set(false) {}

        template <typename T>
        optional_base(bool set_,T&& init): set(set_) { if (set) data.construct(std::forward<T>(init)); }

        reference_type ref() { return data.ref(); }
        const_reference_type ref() const { return data.cref(); }

    public:
        ~optional_base() { if (set) data.destruct(); }

        const_pointer_type operator->() const { return data.ptr(); }
        pointer_type operator->() { return data.ptr(); }
            
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

        void reset() {
            if (set) data.destruct();
            set=false;
        }

        template <typename F>
        auto bind(F &&f) -> typename lift_type<decltype(data.apply(std::forward<F>(f)))>::type {
            typedef decltype(data.apply(std::forward<F>(f))) F_result_type;
            typedef typename lift_type<F_result_type>::type result_type;

            if (!set) return result_type();
            else return bind_impl<result_type,std::is_same<F_result_type,void>::value>::bind(data,std::forward<F>(f));
        }

        template <typename F>
        auto bind(F &&f) const -> typename lift_type<decltype(data.apply(std::forward<F>(f)))>::type {
            typedef decltype(data.apply(std::forward<F>(f))) F_result_type;
            typedef typename lift_type<F_result_type>::type result_type;

            if (!set) return result_type();
            else return bind_impl<result_type,std::is_same<F_result_type,void>::value>::bind(data,std::forward<F>(f));
        }

        template <typename F>
        auto operator>>(F &&f) -> decltype(this->bind(std::forward<F>(f))) { return bind(std::forward<F>(f)); }

        template <typename F>
        auto operator>>(F &&f) const -> decltype(this->bind(std::forward<F>(f))) { return bind(std::forward<F>(f)); }

    private:
        template <typename R,bool F_void_return>
        struct bind_impl {
            template <typename DT,typename F>
            static R bind(DT &d,F &&f) { return R(d.apply(std::forward<F>(f))); }
        };

        template <typename R>
        struct bind_impl<R,true> {
            template <typename DT,typename F>
            static R bind(DT &d,F &&f) { d.apply(std::forward<F>(f)); return R(true); }
        };
        
    };
}

template <typename X>
struct optional: detail::optional_base<detail::optional_value_data<X>> {
    typedef detail::optional_base<detail::optional_value_data<X>> base;
    using base::set;
    using base::ref;
    using base::reset;
    using base::data;

    optional(): base() {}

    template <typename Y=X,typename = typename std::enable_if<std::is_copy_constructible<Y>::value>::type>
    optional(const X &x): base(true,x) {}

    template <typename Y=X,typename = typename std::enable_if<std::is_move_constructible<Y>::value>::type>
    optional(X &&x): base(true,std::move(x)) {}

    optional(const optional &ot): base(ot.set,ot.ref()) {}

    template <typename T>
    optional(const optional<T> &ot): base(ot.set,ot.ref()) {}

    optional(optional &&ot): base(ot.set,std::move(ot.ref())) {}

    template <typename T>
    optional(optional<T> &&ot): base(ot.set,std::move(ot.ref())) {}

    template <typename Y,typename = typename std::enable_if<!detail::is_optional<Y>::value>::type>
    optional &operator=(Y &&y) {
        if (set) ref()=std::forward<Y>(y);
        else {
            set=true;
            data.construct(std::forward<Y>(y));
        }
        return *this;
    }

    optional &operator=(const optional &o) {
        if (set) {
            if (o.set) ref()=o.ref();
            else reset();
        }
        else {
            set=o.set;
            if (set) data.construct(o.ref());
        }
        return *this;
    }

    template <typename Y=X, typename =typename std::enable_if<
        std::is_move_assignable<Y>::value &&
        std::is_move_constructible<Y>::value
    >::type>
    optional &operator=(optional &&o) {
        if (set) {
            if (o.set) ref()=std::move(o.ref());
            else reset();
        }
        else {
            set=o.set;
            if (set) data.construct(std::move(o.ref()));
        }
        return *this;
    }
};

template <typename X>
struct optional<X &>: detail::optional_base<detail::optional_ref_data<X>> {
    typedef detail::optional_base<detail::optional_ref_data<X>> base;
    using base::set;
    using base::ref;
    using base::data;

    optional(): base() {}
    optional(X &x): base(true,x) {}

    template <typename T>
    optional(optional<T &> &ot): base(ot.set,ot.ref()) {}

    template <typename Y,typename = typename std::enable_if<!detail::is_optional<Y>::value>::type>
    optional &operator=(Y &y) {
        set=true;
        ref()=y;
        return *this;
    }

    template <typename Y>
    optional &operator=(optional<Y &> &o) {
        set=o.set;
        data.construct(o);
        return *this;
    }
};


/* special case for optional<void>, used as e.g. the result of
 * binding to a void function */

template <>
struct optional<void>: detail::optional_base<detail::optional_void_data> {
    typedef detail::optional_base<detail::optional_void_data> base;
    using base::set;

    optional(): base() {}

    template <typename T>
    optional(T): base(true,true) {}

    template <typename T>
    optional(const optional<T> &o): base(o.set,true) {}
    
    template <typename T>
    optional &operator=(T) { set=true; return *this; }

    template <typename T>
    optional &operator=(const optional<T> &o) { set=o.set; return *this; }

    // override equality operators
    template <typename Y>
    bool operator==(const Y &y) const { return false; }

    bool operator==(const optional<void> &o) const {
        return set && o.set || !set && !o.set;
    }
};


template <typename A,typename B>
typename std::enable_if<
    detail::is_optional<A>::value || detail::is_optional<B>::value,
    optional<typename std::common_type<typename detail::wrapped_type<A>::type,typename detail::wrapped_type<B>::type>::type>
>::type
operator|(A &&a,B &&b) {
    typedef typename std::common_type<typename detail::wrapped_type<A>::type,typename detail::wrapped_type<B>::type>::type common;
    return a?a:b;
}

template <typename A,typename B>
typename std::enable_if<
    detail::is_optional<A>::value || detail::is_optional<B>::value,
    optional<typename detail::wrapped_type<B>::type>
>::type
operator&(A &&a,B &&b) {
    typedef optional<typename detail::wrapped_type<B>::type> result_type;
    return a?b:result_type();
}

inline optional<void> provided(bool condition) { return condition?optional<void>(true):optional<void>(); }

} // namespace hf

#pragma clang diagnostic pop

#endif // ndef HF_OPTIONALM_H_
