#include <typeinfo>
#include <array>
#include <algorithm>
#include <gtest/gtest.h>

#include <optionalm/optional.h>

#include "test_common.h"

using namespace hf;

TEST(optional, ctors) {
    optional<int> a, b(3), c=b, d=4;

    ASSERT_FALSE((bool)a);
    ASSERT_TRUE((bool)b);
    ASSERT_TRUE((bool)c);
    ASSERT_TRUE((bool)d);

    EXPECT_EQ(3, b.get());
    EXPECT_EQ(3, c.get());
    EXPECT_EQ(4, d.get());
}

TEST(optional, unset_throw) {
    optional<int> a;
    int check=10;

    try { a.get(); }
    catch(optional_unset_error &e) {
        ++check;
    }
    EXPECT_EQ(11, check);

    check=20;
    a=2;
    try { a.get(); }
    catch(optional_unset_error &e) {
        ++check;
    }
    EXPECT_EQ(20, check);

    check=30;
    a.reset();
    try { a.get(); }
    catch(optional_unset_error &e) {
        ++check;
    }
    EXPECT_EQ(31, check);
}

TEST(optional, deref) {
    struct foo {
        int a;
        explicit foo(int a_): a(a_) {}
        double value() { return 3.0*a; }
    };

    optional<foo> f=foo(2);
    EXPECT_EQ(6.0, f->value());
    EXPECT_EQ(2, (*f).a);
}

TEST(optional, ctor_conv) {
    optional<std::array<int, 3>> x{{1, 2,  3}};
    EXPECT_EQ(3, x->size());
}

TEST(optional, ctor_ref) {
    int v=10;
    optional<int &> a(v);

    EXPECT_EQ(10, a.get());
    v=20;
    EXPECT_EQ(20, a.get());

    optional<int &> b(a), c=b, d=v;
    EXPECT_EQ(&(a.get()), &(b.get()));
    EXPECT_EQ(&(a.get()), &(c.get()));
    EXPECT_EQ(&(a.get()), &(d.get()));
}

TEST(optional, assign_returns) {
    optional<int> a=3;

    auto b=(a=4);
    EXPECT_EQ(typeid(optional<int>), typeid(b));

    auto bp=&(a=4);
    EXPECT_EQ(&a, bp);

    auto b2=(a=optional<int>(10));
    EXPECT_EQ(typeid(optional<int>), typeid(b2));

    auto bp2=&(a=4);
    EXPECT_EQ(&a, bp2);

    auto b3=(a=nothing);
    EXPECT_EQ(typeid(optional<int>), typeid(b3));

    auto bp3=&(a=4);
    EXPECT_EQ(&a, bp3);
}

TEST(optional, ctor_nomove) {
    using no_move=testing::no_move<int>;
    no_move::reset_counts();

    optional<no_move> a(no_move(3));
    EXPECT_EQ(3, a.get().value);
    EXPECT_EQ(1, no_move::copy_ctor_count);

    optional<no_move> b;
    b=a; // uses no_move copy ctor, as b is empty.
    EXPECT_EQ(3, b.get().value);
    EXPECT_EQ(2, no_move::copy_ctor_count);

    b=optional<no_move>(no_move(4));
    EXPECT_EQ(4, b.get().value);
    EXPECT_EQ(3, no_move::copy_ctor_count);
    EXPECT_EQ(1, no_move::copy_assign_count);
}

TEST(optional, ctor_nocopy) {
    using no_copy=testing::no_copy<int>;
    no_copy::reset_counts();

    optional<no_copy> a(no_copy(5));
    EXPECT_EQ(5, a.get().value);
    EXPECT_EQ(1, no_copy::move_ctor_count);

    optional<no_copy> b(std::move(a));
    EXPECT_EQ(5, b.get().value);
    EXPECT_EQ(2, no_copy::move_ctor_count);

    b=optional<no_copy>(no_copy(6));
    EXPECT_EQ(6, b.get().value);
    EXPECT_EQ(1, no_copy::move_assign_count);
}

optional<double> odd_half(int n) {
    optional<double> h;
    if (n%2==1) h=n/2.0;
    return h;
}

TEST(optional, bind) {
    optional<int> a;
    auto b=a.bind(odd_half);

    EXPECT_EQ(typeid(optional<double>), typeid(b));

    a=10;
    b=a.bind(odd_half);
    EXPECT_FALSE((bool)b);

    a=11;
    b=a.bind(odd_half);
    EXPECT_TRUE((bool)b);
    EXPECT_EQ(5.5, b.get());

    b=a >> odd_half >> [](double x) { return (int)x; } >> odd_half;
    EXPECT_TRUE((bool)b);
    EXPECT_EQ(2.5, b.get());
}

TEST(optional, void) {
    optional<void> a, b(true), c(a), d=b, e(false);

    EXPECT_FALSE((bool)a);
    EXPECT_TRUE((bool)b);
    EXPECT_FALSE((bool)c);
    EXPECT_TRUE((bool)d);
    EXPECT_TRUE((bool)e);

    auto x=a >> []() { return 1; };
    EXPECT_FALSE((bool)x);

    x=b >> []() { return 1; };
    EXPECT_TRUE((bool)x);
    EXPECT_EQ(1, x.get());
}

TEST(optional, bind_to_void) {
    optional<int> a, b(3);

    int call_count=0;
    auto vf=[&call_count](int i) -> void { ++call_count; };

    auto x=a >> vf;
    EXPECT_EQ(typeid(optional<void>), typeid(x));
    EXPECT_FALSE((bool)x);
    EXPECT_EQ(0, call_count);

    call_count=0;
    x=b >> vf;
    EXPECT_TRUE((bool)x);
    EXPECT_EQ(1, call_count);
}

TEST(optional, bind_to_optional_void) {
    optional<int> a, b(3), c(4);

    int count=0;
    auto count_if_odd=[&count](int i) { return i%2?(++count, optional<void>(true)):optional<void>(); };

    auto x=a >> count_if_odd;
    EXPECT_EQ(typeid(optional<void>), typeid(x));
    EXPECT_FALSE((bool)x);
    EXPECT_EQ(0, count);

    count=0;
    x=b >> count_if_odd;
    EXPECT_TRUE((bool)x);
    EXPECT_EQ(1, count);

    count=0;
    x=c >> count_if_odd;
    EXPECT_FALSE((bool)x);
    EXPECT_EQ(0, count);
}

TEST(optional, bind_with_ref) {
    optional<int> a=10;
    a >> [](int &v) {++v; };
    EXPECT_EQ(11, *a);
}

struct check_cref {
    int operator()(const int &) { return 10; }
    int operator()(int &) { return 11; }
};

TEST(optional, bind_constness) {
    check_cref checker;
    optional<int> a=1;
    int v=*(a >> checker);
    EXPECT_EQ(11, v);

    const optional<int> b=1;
    v=*(b >> checker);
    EXPECT_EQ(10, v);
}


TEST(optional, conversion) {
    optional<double> a(3), b=5;
    EXPECT_TRUE((bool)a);
    EXPECT_TRUE((bool)b);
    EXPECT_EQ(3.0, a.get());
    EXPECT_EQ(5.0, b.get());

    optional<int> x;
    optional<double> c(x);
    optional<double> d=optional<int>();
    EXPECT_FALSE((bool)c);
    EXPECT_FALSE((bool)d);

    auto doubler=[](double x) { return x*2; };
    auto y=optional<int>(3) >> doubler;
    EXPECT_TRUE((bool)y);
    EXPECT_EQ(6.0, y.get());
}

TEST(optional, or_operator) {
    optional<const char *> default_msg="default";
    auto x=nullptr | default_msg;
    EXPECT_TRUE((bool)x);
    EXPECT_STREQ("default", x.get());

    auto y="something" | default_msg;
    EXPECT_TRUE((bool)y);
    EXPECT_STREQ("something", y.get());

    optional<int> a(1), b, c(3);
    EXPECT_EQ(1, *(a|b|c));
    EXPECT_EQ(1, *(a|c|b));
    EXPECT_EQ(1, *(b|a|c));
    EXPECT_EQ(3, *(b|c|a));
    EXPECT_EQ(3, *(c|a|b));
    EXPECT_EQ(3, *(c|b|a));
}

TEST(optional, and_operator) {
    optional<int> a(1);
    optional<double> b(2.0);

    auto ab=a&b;
    auto ba=b&a;

    EXPECT_EQ(typeid(ab), typeid(b));
    EXPECT_EQ(typeid(ba), typeid(a));
    EXPECT_EQ(2.0, *ab);
    EXPECT_EQ(1, *ba);

    auto zb=false & b;
    EXPECT_EQ(typeid(zb), typeid(b));
    EXPECT_FALSE((bool)zb);

    auto b3=b & 3;
    EXPECT_EQ(typeid(b3), typeid(optional<int>));
    EXPECT_TRUE((bool)b3);
    EXPECT_EQ(3, *b3);
}

TEST(optional, provided) {
    std::array<int, 3> qs={1, 0, 3};
    std::array<int, 3> ps={14, 14, 14};
    std::array<int, 3> rs;

    std::transform(ps.begin(), ps.end(), qs.begin(), rs.begin(),
        [](int p, int q) { return *( provided(q!=0) >> [=]() { return p/q; } | -1 ); });

    EXPECT_EQ(14, rs[0]);
    EXPECT_EQ(-1, rs[1]);
    EXPECT_EQ(4, rs[2]);
}
