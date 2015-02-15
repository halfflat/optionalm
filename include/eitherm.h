/* \file eitherm.h
 * \brief Type-safe discriminated union of two types.
 */

#ifndef HF_EITHERM_H_
#define HF_EITHERM_H_

#include <type_traits>
#include <string>
#include <stdexcept>

#include "uninitialized.h"

namespace hf {

template <std::size_t I>
struct in_place_field_t {};

#if __cplusplus >= 201302L
template <std::size_t I> constexpr in_place_field_t<I> in_place_field{};
#endif

struct in_place_t {};
constexpr in_place_t in_place{};

struct either_invalid_get: public std::runtime_error {
    explicit either_invalid_get(const std::string &what_str): std::runtime_error(what_str) {}
    either_invalid_get(): either_invalid_get("get on unset either field") {}
};

template <typename A,typename B>
struct either_data {
    union {
        uninitialized<A> ua;
        uninitialized<B> ub;
    } data;
};

template <typename A,typename B,typename... X>
struct either_constructible {
private:
    enum { is_a_constructible=uninitialized_can_construct<A,X...>::value };
    enum { is_b_constructible=uninitialized_can_construct<B,X...>::value };
public:
    enum { value=(is_a_constructible^is_b_constructible) };
    enum { which=is_a_constructible?0:1 };
};

template <std::size_t,typename A,typename B>
struct either_select;

template <typename A,typename B>
struct either_select<0,A,B> {
    typedef uninitialized<A> type;
    static type &field(either_data<A,B> &u) { return u.data.ua; }
    static const type &field(const either_data<A,B> &u) { return u.data.ua; }
};

template <typename A,typename B>
struct either_select<1,A,B> {
    typedef uninitialized<B> type;
    static type &field(either_data<A,B> &u) { return u.data.ub; }
    static const type &field(const either_data<A,B> &u) { return u.data.ub; }
};

template <std::size_t I,typename A,typename B>
struct either_get: either_select<I,A,B> {
    using typename either_select<I,A,B>::type;
    using either_select<I,A,B>::field;

    static bool match(char which) { return I==which; }

    static typename type::reference_type unsafe_get(either_data<A,B> &u) {
        return field(u).ref();
    }
    static typename type::const_reference_type unsafe_get(const either_data<A,B> &u) {
        return field(u).cref();
    }

    static typename type::reference_type unsafe_get(char which,either_data<A,B> &u) {
        if (!match(which)) throw either_invalid_get();
        return field(u).ref();
    }
    static typename type::const_reference_type unsafe_get(char which,const either_data<A,B> &u) {
        if (!match(which)) throw either_invalid_get();
        return field(u).cref();
    }

    static typename type::pointer_type ptr(char which,either_data<A,B> &u) {
        return match(which)?field(u).ptr():nullptr;
    }
    static typename type::const_pointer_type tr(char which,const either_data<A,B> &u) {
        return match(which)?field(u).cptr():nullptr;
    }
};

template <typename A,typename B>
struct either: either_data<A,B> {
    using either_data<A,B>::data;
    char which;

    enum { ab_distinct=!std::is_same<typename std::decay<A>::type,typename std::decay<B>::type>::value };

    // implicit copy/move construction relies on A and B being distinct

    template <typename Y,typename P=either_constructible<A,B,Y>,
              typename =typename std::enable_if<P::value>::type>
    either(Y &&a): which(P::which) { either_select<P::which,A,B>::field(*this).construct(std::forward<Y>(a)); }

    template <typename... X,typename =typename std::enable_if<
        uninitialized_can_construct<A,X...>::value>::type>
    either(in_place_field_t<0>,X&&...x): which(0) { data.ua.construct(std::forward<X>(x)...); }

    template <typename... X,typename =typename std::enable_if<
        uninitialized_can_construct<B,X...>::value>::type>
    either(in_place_field_t<1>,X&&...x): which(1) { data.ub.construct(std::forward<X>(x)...); }

    template <typename... X,typename =typename std::enable_if<either_constructible<A,B,X...>::value>::type>
    either(in_place_t,X&&...x): either(in_place_field_t<either_constructible<A,B,X...>::which>{},std::forward<X>(x)...) {}

    template <std::size_t I>
    auto unsafe_get() -> decltype(either_get<I,A,B>::unsafe_get(*this)) { return either_get<I,A,B>::unsafe_get(*this); }
    template <std::size_t I>
    auto unsafe_get() const -> decltype(either_get<I,A,B>::unsafe_get(*this)) { return either_get<I,A,B>::unsafe_get(*this); }

    template <std::size_t I>
    auto ptr() -> decltype(either_get<I,A,B>::ptr(which,*this)) { return either_get<I,A,B>::ptr(which,*this); }
    template <std::size_t I>
    auto ptr() const -> decltype(either_get<I,A,B>::ptr(which,*this)) { return either_get<I,A,B>::ptr(which,*this); }
};

    

} // namespace hf
#endif // ndef HF_EITHERM_H_

