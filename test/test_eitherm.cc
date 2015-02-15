#include <gtest/gtest.h>

#include "eitherm.h"

using namespace hf;

TEST(eitherm,ctor_explicit_field) {
    either<int,const char *> e1(in_place_field_t<0>{},7);
    either<int,const char *> e2(in_place_field_t<1>{},"hello");

    ASSERT_EQ(7,e1.unsafe_get<0>());
    ASSERT_STREQ("hello",e2.unsafe_get<1>());
}

TEST(eitherm,ctor_implicit_field) {
    either<int,const char *> e1(in_place,7);
    either<int,const char *> e2(in_place,"hello");

    ASSERT_EQ(7,e1.unsafe_get<0>());
    ASSERT_STREQ("hello",e2.unsafe_get<1>());
}

TEST(eitherm,ctor_implicit_copy) {
    int seven=7;
    const char *hello="hello";
    either<int,const char *> e1(seven);
    either<int,const char *> e2(hello);

    ASSERT_EQ(7,e1.unsafe_get<0>());
    ASSERT_STREQ("hello",e2.unsafe_get<1>());
}

struct move_copy_counts {
    int n_move_ctor=0;
    int n_copy_ctor=0;
    int n_move_assign=0;
    int n_copy_assign=0;

    void reset() {
        n_move_ctor=0;
        n_copy_ctor=0;
        n_move_assign=0;
        n_copy_assign=0;
    }
};

template <typename X>
struct nocopy {
    move_copy_counts *k;
    X x;

    nocopy(move_copy_counts &counts,const X &x_): k(&counts),x(x_) {}

    nocopy(const nocopy &n) = delete;
    nocopy(nocopy &&n): k(n.k),x(std::move(n.x)) { ++k->n_move_ctor; }

    nocopy &operator=(const nocopy &n) = delete;
    nocopy &operator=(nocopy &&n) { k=n.k; ++k->n_move_assign; x=std::move(n.x); return *this; }
};


TEST(eitherm,ctor_implicit_move) {
    move_copy_counts K;

    nocopy<int> n_7(K,7);
    nocopy<std::string> n_hello(K,"hello");

    typedef either<nocopy<int>,nocopy<std::string>> e_int_string;
    e_int_string e1(std::move(n_7));
    e_int_string e2(std::move(n_hello));

    ASSERT_EQ(7,e1.unsafe_get<0>().x);
    ASSERT_STREQ("hello",e2.unsafe_get<1>().x.c_str());

    ASSERT_EQ(2,K.n_move_ctor);
}

TEST(eitherm,ctor_ref_explicit_field) {
    typedef either<int &,double>  e_intref_double;

    int a=10;
    double b=20.0;

    e_intref_double e1(in_place_field_t<0>{},a);
    e_intref_double e2(in_place_field_t<1>{},b);

    ASSERT_EQ(10,e1.unsafe_get<0>());
    ASSERT_EQ(20.0,e2.unsafe_get<1>());

    e1.unsafe_get<0>()=11;
    ASSERT_EQ(11,a);

    int &ir=e1.unsafe_get<0>();
    ir=12;
    ASSERT_EQ(12,a);

    double &dr=e2.unsafe_get<1>();
    dr=21.0;
    ASSERT_EQ(20.0,b);
}
    
TEST(eitherm,ptr) {
    struct A { int foo() const { return 3; } };
    struct B { int foo() const { return 4; } };

    either<A,B> e1(B{}),e2{A{}};
    ASSERT_EQ(nullptr,e1.ptr<0>());
    ASSERT_EQ(nullptr,e2.ptr<1>());
}
