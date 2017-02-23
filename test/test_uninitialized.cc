#include <gtest/gtest.h>

#include <optionalm/uninitialized.h>

#include "test_common.h"

using namespace hf::optionalm;

TEST(uninitialized, ctor) {
    using count=testing::ctor_count<char>;
    count::reset_counts();

    uninitialized<count> ua;
    ua.construct(count{});

    count b;
    ua.construct(b);

    EXPECT_EQ(1, count::copy_ctor_count);
    EXPECT_EQ(0, count::copy_assign_count);
    EXPECT_EQ(1, count::move_ctor_count);
    EXPECT_EQ(0, count::move_assign_count);

    ua.ref()=count{};
    ua.ref()=b;

    EXPECT_EQ(1, count::copy_ctor_count);
    EXPECT_EQ(1, count::copy_assign_count);
    EXPECT_EQ(1, count::move_ctor_count);
    EXPECT_EQ(1, count::move_assign_count);
}

TEST(uninitialized, ctor_nocopy) {
    using no_copy=testing::no_copy<int>;
    no_copy::reset_counts();

    uninitialized<no_copy> ua;
    ua.construct(no_copy{});

    EXPECT_EQ(1, no_copy::move_ctor_count);
    EXPECT_EQ(0, no_copy::move_assign_count);

    ua.ref()=no_copy{};

    EXPECT_EQ(1, no_copy::move_ctor_count);
    EXPECT_EQ(1, no_copy::move_assign_count);
}

TEST(uninitialized, ctor_nomove) {
    using no_move=testing::no_move<int>;
    no_move::reset_counts();

    uninitialized<no_move> ua;
    ua.construct(no_move{}); // check against rvalue

    no_move b;
    ua.construct(b); // check against non-const lvalue

    const no_move c;
    ua.construct(c); // check against const lvalue

    EXPECT_EQ(3, no_move::copy_ctor_count);
    EXPECT_EQ(0, no_move::copy_assign_count);

    no_move a;
    ua.ref()=a;

    EXPECT_EQ(3, no_move::copy_ctor_count);
    EXPECT_EQ(1, no_move::copy_assign_count);
}

TEST(uninitialized, void) {
    uninitialized<void> a, b;
    a=b;

    EXPECT_EQ(typeid(a.ref()), typeid(void));
}

TEST(uninitialized, ref) {
    uninitialized<int &> x, y;
    int a;

    x.construct(a);
    y=x;

    x.ref()=2;
    EXPECT_EQ(2, a);

    y.ref()=3;
    EXPECT_EQ(3, a);
    EXPECT_EQ(3, x.cref());

    EXPECT_EQ(&a, x.ptr());
    EXPECT_EQ((const int *)&a, x.cptr());
}

struct apply_tester {
    mutable int op_count=0;
    mutable int const_op_count=0;

    int operator()(const int &a) const { ++const_op_count; return a+1; }
    int operator()(int &a) const { ++op_count; return ++a; }
};

TEST(uninitialized, apply) {
    uninitialized<int> ua;
    ua.construct(10);

    apply_tester A;
    int r=ua.apply(A);
    EXPECT_EQ(11, ua.cref());
    EXPECT_EQ(11, r);

    uninitialized<int &> ub;
    ub.construct(ua.ref());

    r=ub.apply(A);
    EXPECT_EQ(12, ua.cref());
    EXPECT_EQ(12, r);

    uninitialized<const int &> uc;
    uc.construct(ua.ref());

    r=uc.apply(A);
    EXPECT_EQ(12, ua.cref());
    EXPECT_EQ(13, r);

    const uninitialized<int> ud(ua);

    r=ud.apply(A);
    EXPECT_EQ(12, ua.cref());
    EXPECT_EQ(12, ud.cref());
    EXPECT_EQ(13, r);

    EXPECT_EQ(2, A.op_count);
    EXPECT_EQ(2, A.const_op_count);
}

TEST(uninitialized, void_apply) {
    uninitialized<void> uv;

    auto f=[]() { return 11; };
    EXPECT_EQ(11, uv.apply(f));

    EXPECT_EQ(12.5, uv.apply([]() { return 12.5; }));
}
