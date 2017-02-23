#ifndef HF_EITHER_H_
#define HF_EITHER_H_

#include <type_traits>
#include <string>
#include <stdexcept>

#include <optionalm/uninitialized.h>

namespace hf {
namespace optionalm {

struct bad_either_access: public std::runtime_error {
    explicit bad_either_access(const std::string &what_str): std::runtime_error(what_str) {}
    bad_either_access(): bad_either_access("get on unset either field") {}
};

namespace detail {
    template <typename A,typename B>
    struct either_data {
        union {
            uninitialized<A> ua;
            uninitialized<B> ub;
        };

        either_data()=default;

        either_data(const either_data&)=delete;
        either_data(either_data&&)=delete;
        either_data& operator=(const either_data&)=delete;
        either_data& operator=(either_data&&)=delete;
    };

    template <std::size_t, typename A, typename B>
    struct either_select;

    template <typename A, typename B>
    struct either_select<0, A, B> {
        typedef uninitialized<A> type;
        static type& field(either_data<A, B>& u) { return u.ua; }
        static const type& field(const either_data<A, B>& u) { return u.ua; }
    };

    template <typename A,typename B>
    struct either_select<1, A, B> {
        typedef uninitialized<B> type;
        static type& field(either_data<A, B> &u) { return u.ub; }
        static const type& field(const either_data<A, B> &u) { return u.ub; }
    };

    template <std::size_t I, typename A, typename B>
    struct either_get: either_select<I, A, B> {
        using typename either_select<I, A, B>::type;
        using either_select<I, A, B>::field;

        static typename type::reference unsafe_get(either_data<A,B>& u) {
            return field(u).ref();
        }

        static typename type::const_reference unsafe_get(const either_data<A, B>& u) {
            return field(u).cref();
        }

        static typename type::reference get(char which, either_data<A, B>& u) {
            if (I!=which) throw bad_either_access();
            return field(u).ref();
        }

        static typename type::const_reference get(char which, const either_data<A, B>& u) {
            if (I!=which) throw bad_either_access();
            return field(u).cref();
        }

        static typename type::pointer ptr(char which, either_data<A, B>& u) {
            return I==which? field(u).ptr(): nullptr;
        }
        static typename type::const_pointer ptr(char which, const either_data<A, B>& u) {
            return I==which? field(u).cptr(): nullptr;
        }
    };

    struct ctor_tag {};
} // namespace detail

template <std::size_t I>
struct in_place_index_t: detail::ctor_tag {};

#if defined(__cpp_variable_templates)
template <std::size_t I> constexpr in_place_index_t<I> in_place_index{};
#endif

struct in_place_t: detail::ctor_tag {};
constexpr in_place_t in_place{};

template <typename A,typename B>
class either: public detail::either_data<A, B> {
    using base=detail::either_data<A, B>;
    using base::ua;
    using base::ub;

    template <std::size_t I>
    using getter=detail::either_get<I, A, B>;

    template <std::size_t I>
    typename getter<I>::type& field() { return getter<I>::field(*this); }

    template <std::size_t I>
    const typename getter<I>::type& field() const { return getter<I>::field(*this); }

    unsigned char which;

public:
    static constexpr char either_npos=-1;

    // Can default construct if A or B is; try A first.
    template <
        typename A_ = A,
        bool a_ok = std::is_default_constructible<A_>::value,
        bool b_ok = std::is_default_constructible<B>::value,
        typename = typename std::enable_if<a_ok || b_ok>::type,
        std::size_t w_ = a_ok? 0: 1
    >
    either()
        noexcept(std::is_nothrow_default_constructible<typename getter<w_>::type>::value):
        which(w_)
    {
        getter<w_>::field(*this).constuct();
    }

    // Explicitly construct field in-place given by `in_place_index`.
    template <std::size_t w_, typename... Args>
    either(in_place_index_t<w_>, Args&&... args)
        noexcept(std::is_nothrow_constructible<typename getter<w_>::type, Args...>::value):
        which(w_)
    {
        getter<w_>::field(*this).construct(std::forward<Args>(args)...);
    }


    // Construct first field in-place that is constructible from arguments.
    template <
        typename... Args,
        typename A_ = A,
        bool a_ok = std::is_constructible<A_, Args...>::value,
        bool b_ok = std::is_constructible<B, Args...>::value,
        typename = typename std::enable_if<a_ok || b_ok>::type,
        std::size_t w_ = a_ok? 0: 1
    >
    either(in_place_t, Args&&... args)
        noexcept(std::is_nothrow_constructible<typename getter<w_>::type, Args...>::value):
        which(w_)
    {
        field<w_>().construct(std::forward<Args>(args)...);
    }

    // Implicit conversion from argument.
    template <
        typename T,
        bool a_ok = std::is_lvalue_reference<A>::value? std::is_convertible<T&, A>::value: std::is_constructible<A, T>::value,
        bool b_ok = std::is_lvalue_reference<B>::value? std::is_convertible<T&, B>::value: std::is_constructible<B, T>::value,
        typename = typename std::enable_if<
            (a_ok || b_ok) &&
            !std::is_base_of<detail::ctor_tag, T>::value &&
            !std::is_same<typename std::decay<T>::type, either>::value>::type,
        std::size_t w_ = a_ok? 0: 1
    >
    either(T&& x)
        noexcept(std::is_nothrow_constructible<typename getter<w_>::type, T>::value):
        which(w_)
    {
        field<w_>().construct(std::forward<T>(x));
    }

    // Copy constructor.
    either(const either& x)
        noexcept(std::is_nothrow_copy_constructible<A>::value && std::is_nothrow_copy_constructible<B>::value):
        which(x.which)
    {
        switch (which) {
        case 0:
            field<0>().construct(x.unsafe_get<0>());
            break;
        case 1:
            field<1>().construct(x.unsafe_get<1>());
            break;
        }
    }

    // Move constructor.
    either(either&& x)
        noexcept(std::is_nothrow_move_constructible<A>::value && std::is_nothrow_move_constructible<B>::value):
        which(x.which)
    {
        switch (which) {
        case 0:
            getter<0>::field(*this).construct(std::move(x.unsafe_get<0>()));
            break;
        case 1:
            getter<1>::field(*this).construct(std::move(x.unsafe_get<1>()));
            break;
        }
    }

    // Copy assignment.
    either& operator=(const either& x) {
        if (which==0) {
            if (x.which==0) {
                unsafe_get<0>()=x.unsafe_get<0>();
            }
            else if (x.which==1) {
                B b_tmp(x.unsafe_get<1>());
                field<0>().destruct();
                which=(char)either_npos;
                field<1>().construct(std::move(b_tmp));
                which=1;
            }
            else {
                field<0>().destruct();
                which=(char)either_npos;
            }
        }
        else if (which==1) {
            if (x.which==0) {
                A a_tmp(x.unsafe_get<0>());
                field<1>().destruct();
                which=(char)either_npos;
                field<0>().construct(std::move(a_tmp));
                which=0;
            }
            else if (which==1) {
                unsafe_get<1>()=x.unsafe_get<1>();
            }
            else {
                field<1>().destruct();
                which=(char)either_npos;
            }
        }
        else {
            if (x.which==0) {
                unsafe_get<0>()=x.unsafe_get<0>();
            }
            else {
                unsafe_get<1>()=x.unsafe_get<1>();
            }
        }
        return *this;
    }

    // Move assignment.
    either& operator=(either&& x) {
        if (which==0) {
            if (x.which==0) {
                unsafe_get<0>()=std::move(x.unsafe_get<0>());
            }
            else if (x.which==1) {
                which=(char)either_npos;
                field<0>().destruct();
                field<1>().construct(std::move(x.unsafe_get<1>()));
                which=1;
            }
            else {
                which=(char)either_npos;
                field<0>().destruct();
            }
        }
        else if (which==1) {
            if (x.which==0) {
                which=(char)either_npos;
                field<1>().destruct();
                field<0>().construct(std::move(x.unsafe_get<0>()));
                which=0;
            }
            else if (x.which==1) {
                unsafe_get<1>()=std::move(x.unsafe_get<1>());
            }
            else {
                which=(char)either_npos;
                field<1>().destruct();
            }
        }
        else {
            if (x.which==0) {
                field<0>().construct(std::move(x.unsafe_get<0>()));
            }
            else {
                field<1>().construct(std::move(x.unsafe_get<1>()));
            }
        }
        return *this;
    }

    // Element access.
    template <std::size_t I>
    typename getter<I>::type::reference unsafe_get() { return getter<I>::unsafe_get(*this); }

    template <std::size_t I>
    typename getter<I>::type::const_reference unsafe_get() const { return getter<I>::unsafe_get(*this); }

    template <std::size_t I>
    typename getter<I>::type::reference get() { return getter<I>::get(which, *this); }

    template <std::size_t I>
    typename getter<I>::type::const_reference get() const { return getter<I>::get(which, *this); }

    template <std::size_t I>
    typename getter<I>::type::pointer ptr() { return getter<I>::ptr(which, *this); }

    template <std::size_t I>
    typename getter<I>::type::reference ptr() const { return getter<I>::ptr(which, *this); }

    // True if first field is occupied.
    constexpr operator bool() const { return which==0; }

    // Index of defined field.
    constexpr std::size_t index() const noexcept { return which; }
    constexpr bool valueless_by_exception() const noexcept { return which==(char)either_npos; }

    // Comparison operations.
    bool operator==(const either& x) const {
        return index()==x.index() &&
           index()==0? unsafe_get<0>()==x.unsafe_get<0>():
           index()==1? unsafe_get<1>()==x.unsafe_get<1>():
           true;
    }

    bool operator!=(const either& x) const {
        return index()!=x.index() ||
           index()==0? unsafe_get<0>()!=x.unsafe_get<0>():
           index()==1? unsafe_get<1>()!=x.unsafe_get<1>():
           false;
    }

    bool operator<(const either& x) const {
        return !x.valueless_by_exception() &&
           index()==0? (x.index()==1 || unsafe_get<0>()<x.unsafe_get<0>()):
           index()==1? (x.index()!=0 && unsafe_get<1>()<x.unsafe_get<1>()):
           true;
    }

    bool operator>=(const either& x) const {
        return x.valueless_by_exception() ||
           index()==0? (x.index()!=1 && unsafe_get<0>()>=x.unsafe_get<0>()):
           index()==1? (x.index()==0 || unsafe_get<1>()>=x.unsafe_get<1>()):
           false;
    }

    bool operator<=(const either& x) const {
        return valueless_by_exception() ||
           x.index()==0? (index()!=1 && unsafe_get<0>()<=x.unsafe_get<0>()):
           x.index()==1? (index()==0 || unsafe_get<1>()<=x.unsafe_get<1>()):
           false;
    }

    bool operator>(const either& x) const {
        return !valueless_by_exception() &&
           x.index()==0? (index()==1 || unsafe_get<0>()>x.unsafe_get<0>()):
           x.index()==1? (index()!=0 && unsafe_get<1>()>x.unsafe_get<1>()):
           true;
    }

    // Destruction.
    ~either() {
        switch (which) {
        case 0:
            field<0>().destruct();
            break;
        case 1:
            field<1>().destruct();
            break;
        }
    }
};

}} // namespace hf::optionalm

#endif // ndef HF_EITHER_H_
