#ifndef HF_UNINITIALIZED_H_
#define HF_UNINITIALIZED_H_

/* Represent a possibly-uninitialized value, reference or void.
 *
 * The `uninitialized<X>` structure holds space for an item of
 * type `X`, leaving its construction or destruction to the user.
 *
 * The `uninitialized<X&>` structure represents a possibly
 * uninitialized reference by wrapping a `X*`.
 *
 * The specialization `uninitialized<void>` is included to
 * ease generic code; destruction and construction are NOPs.
 */

namespace hf {
namespace optionalm {

template <typename X>
struct uninitialized {
private:
    typename std::aligned_storage<sizeof(X),alignof(X)>::type data;

public:
    typedef X *pointer;
    typedef const X *const_pointer;
    typedef X &reference;
    typedef const X &const_reference;

    // Return a pointer to the value.
    pointer ptr() { return reinterpret_cast<X *>(&data); }
    // Return a const pointer to the value.
    const_pointer cptr() const { return reinterpret_cast<const X *>(&data); }

    // Return a reference to the value.
    reference ref() { return *reinterpret_cast<X *>(&data); }
    // Return a const reference to the value.
    const_reference cref() const { return *reinterpret_cast<const X *>(&data); }

    // Copy construct the value.
    template <typename Y=X,typename =typename std::enable_if<std::is_copy_constructible<Y>::value>::type>
    void construct(const X &x) { new(&data) X(x); }

    // General constructor
    template <typename... Y,typename =typename std::enable_if<std::is_constructible<X,Y...>::value>::type>
    void construct(Y&& ...args) { new(&data) X(std::forward<Y>(args)...); }

    // Call the destructor of the value.
    void destruct() { ptr()->~X(); }

    // Apply the one-parameter functor F to the value by reference.
    template <typename F>
    typename std::result_of<F(reference)>::type apply(F &&f) { return f(ref()); }
    // Apply the one-parameter functor F to the value by const reference.
    template <typename F>
    typename std::result_of<F(const_reference)>::type apply(F &&f) const { return f(cref()); }
};

template <typename X>
struct uninitialized<X&> {
private:
    X *data;

public:
    typedef X *pointer;
    typedef const X *const_pointer;
    typedef X &reference;
    typedef const X &const_reference;

    // Return a pointer to the value.
    pointer ptr() { return data; }
    // Return a const pointer to the value.
    const_pointer cptr() const { return data; }

    // Return a reference to the value.
    reference ref() { return *data; }
    // Return a const reference to the value.
    const_reference cref() const { return *data; }

    // Set the reference data.
    void construct(X &x) { data=&x; }
    // Destruct is a NOP for reference data.
    void destruct() {}

    // Apply the one-parameter functor F to the value by reference.
    template <typename F>
    typename std::result_of<F(reference)>::type apply(F &&f) { return f(ref()); }
    // Apply the one-parameter functor F to the value by const reference.
    template <typename F>
    typename std::result_of<F(const_reference)>::type apply(F &&f) const { return f(cref()); }
};

template <>
struct uninitialized<void> {
    typedef void* pointer;
    typedef const void* const_pointer;
    typedef void reference;
    typedef void const_reference;

    pointer ptr() { return nullptr; }
    const_pointer cptr() const { return nullptr; }

    reference ref() {}
    const_reference cref() const {}

    void construct(...) {}
    void destruct() {}

    // Equivalent to `f()`
    template <typename F>
    typename std::result_of<F()>::type apply(F &&f) const { return f(); }
};

#if 0
template <typename...>
struct uninitialized_can_construct: std::false_type {};

template <typename X,typename... Y>
struct uninitialized_can_construct<X,Y...>: std::integral_constant<bool,std::is_constructible<X,Y...>::value> {};

template <typename X,typename Y>
struct uninitialized_can_construct<X &,Y>: std::integral_constant<bool,std::is_convertible<X &,Y>::value> {};

template <typename... Y>
struct uninitialized_can_construct<void,Y...>: std::true_type {};
#endif


}} // namespace hf::optionalm

#endif // ndef HF_UNINITIALIZED_H_

