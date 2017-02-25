#include <gtest/gtest.h>

#include <optionalm/either.h>

#include "test_common.h"

using namespace hf;


TEST(either, ctor_in_place_index) {
    either<int, const char*> e1(in_place_index_t<0>{}, 7);
    either<int, const char*> e2(in_place_index_t<1>{}, "hello");

    ASSERT_EQ(0, e1.index());
    ASSERT_EQ(7, e1.unsafe_get<0>());

    ASSERT_EQ(1, e2.index());
    ASSERT_STREQ("hello", e2.unsafe_get<1>());
}

struct cat {
    std::string value;
    cat(const std::string& a, const std::string& b): value(a+b) {}
};

TEST(eitherm, ctor_in_place) {
    either<int, const char*> e1(in_place, 7);
    either<int, const char*> e2(in_place, "hello");
    either<int, cat> e3(in_place, "hello ", "there");

    ASSERT_EQ(0, e1.index());
    ASSERT_EQ(7, e1.unsafe_get<0>());

    ASSERT_EQ(1, e2.index());
    ASSERT_STREQ("hello", e2.unsafe_get<1>());
    ASSERT_STREQ("hello there", e3.unsafe_get<1>().value.c_str());
}

TEST(eitherm, ctor_implicit) {
    either<int, const char*> e1(7);
    either<int, const char*> e2("hello");

    ASSERT_EQ(0, e1.index());
    ASSERT_EQ(7, e1.unsafe_get<0>());

    ASSERT_EQ(1, e2.index());
    ASSERT_STREQ("hello", e2.unsafe_get<1>());
}

TEST(eitherm, ctor_implicit_copy) {
    using count_int=testing::ctor_count<int>;
    using count_charp=testing::ctor_count<const char*>;

    count_int::reset_counts();
    count_charp::reset_counts();

    count_int seven=7;
    count_charp hello="hello";

    either<count_int, count_charp> e1(seven);
    either<count_int, count_charp> e2(hello);

    ASSERT_EQ(7, e1.unsafe_get<0>().value);
    EXPECT_EQ(1, count_int::copy_ctor_count);

    ASSERT_STREQ("hello", e2.unsafe_get<1>().value);
    EXPECT_EQ(1, count_charp::copy_ctor_count);
}

TEST(eitherm, ctor_implicit_move) {
    using count_int=testing::ctor_count<int>;
    using count_charp=testing::ctor_count<const char*>;

    count_int::reset_counts();
    count_charp::reset_counts();

    either<count_int, count_charp> e1(count_int{7});
    either<count_int, count_charp> e2(count_charp{"hello"});

    ASSERT_EQ(7, e1.unsafe_get<0>().value);
    EXPECT_EQ(1, count_int::move_ctor_count);

    ASSERT_STREQ("hello", e2.unsafe_get<1>().value);
    EXPECT_EQ(1, count_charp::move_ctor_count);
}

TEST(eitherm, get) {
    either<int, int> e1(in_place_index_t<0>{}, 3);
    either<int, int> e2(in_place_index_t<1>{}, 5);

    EXPECT_EQ(e1.get<0>(), e1.unsafe_get<0>());
    EXPECT_EQ(e2.get<1>(), e2.unsafe_get<1>());

    ASSERT_THROW(e1.get<1>(), bad_either_access);
    ASSERT_THROW(e2.get<0>(), bad_either_access);
}

TEST(eitherm, ref) {
    double x=3.0;
    either<int&, double&> e1(in_place, x);
    either<double&, int&> e2(in_place_index_t<0>{}, x);
    either<int&, double&> e3(x);

    e1.get<1>()+=1.0;
    EXPECT_EQ(4.0, x);

    e2.get<0>()+=2.0;
    EXPECT_EQ(6.0, x);

    e3.get<1>()+=3.0;
    EXPECT_EQ(9.0, x);
}

TEST(eitherm, assign) {
    either<int, const char*> e1("abc");
    either<int, const char*> e2("def");
    either<int, const char*> e3(123);
    either<int, const char*> e4(456);

    // same field
    e2=e1;
    ASSERT_EQ(1, e2.index());
    EXPECT_STREQ(e1.get<1>(), e2.get<1>());

    e4=e3;
    ASSERT_EQ(0, e4.index());
    EXPECT_EQ(e3.get<0>(), e4.get<0>());

    // different field
    e4=e1;
    ASSERT_EQ(1, e4.index());
    EXPECT_STREQ(e1.get<1>(), e4.get<1>());

    e2=e3;
    ASSERT_EQ(0, e2.index());
    EXPECT_EQ(e3.get<0>(), e2.get<0>());
}

TEST(eitherm, move_assign) {
    using nc_string=testing::no_copy<std::string>;
    using nc_int=testing::no_copy<int>;

    either<nc_int, nc_string> e1(nc_string("abc"));
    either<nc_int, nc_string> e2(nc_string("def"));
    either<nc_int, nc_string> e3(nc_int(123));
    either<nc_int, nc_string> e4(nc_int(456));

    nc_string::reset_counts();
    nc_int::reset_counts();

    // same field (uses move assignment)
    e2=std::move(e1);
    ASSERT_EQ(1, e2.index());
    EXPECT_EQ("abc", e2.get<1>().value);

    EXPECT_EQ(1u, nc_string::move_assign_count);
    EXPECT_EQ(0u, nc_string::move_ctor_count);
    EXPECT_EQ(0u, nc_int::move_assign_count);
    EXPECT_EQ(0u, nc_int::move_ctor_count);

    e4=std::move(e3);
    ASSERT_EQ(0, e4.index());
    EXPECT_EQ(123, e4.get<0>().value);

    EXPECT_EQ(1u, nc_string::move_assign_count);
    EXPECT_EQ(0u, nc_string::move_ctor_count);
    EXPECT_EQ(1u, nc_int::move_assign_count);
    EXPECT_EQ(0u, nc_int::move_ctor_count);

    // different field (uses move construction)
    either<nc_int, nc_string> e1bis(nc_string("efg"));
    either<nc_int, nc_string> e3bis(nc_int(789));

    nc_string::reset_counts();
    nc_int::reset_counts();

    e4=std::move(e1bis);
    ASSERT_EQ(1, e4.index());
    EXPECT_EQ("efg", e4.get<1>().value);

    EXPECT_EQ(0u, nc_string::move_assign_count);
    EXPECT_EQ(1u, nc_string::move_ctor_count);
    EXPECT_EQ(0u, nc_int::move_assign_count);
    EXPECT_EQ(0u, nc_int::move_ctor_count);

    e2=std::move(e3bis);
    ASSERT_EQ(0, e2.index());
    EXPECT_EQ(789, e2.get<0>().value);

    EXPECT_EQ(0u, nc_string::move_assign_count);
    EXPECT_EQ(1u, nc_string::move_ctor_count);
    EXPECT_EQ(0u, nc_int::move_assign_count);
    EXPECT_EQ(1u, nc_int::move_ctor_count);
}

TEST(eitherm, ref_assign_0) {
    // assignments from ref in second field
    double x=3.0;
    double y=5.0;
    either<double&, void*> e1(x);
    either<double&, void*> e2(y);

    e1=e2;
    ASSERT_EQ(0u, e1.index());
    EXPECT_EQ(5.0, e1.get<0>());
    EXPECT_EQ(&y, &e1.get<0>());
    EXPECT_EQ(3.0, x);

    either<double&, void*> e3(nullptr);
    e3=e2;
    ASSERT_EQ(0u, e3.index());
    EXPECT_EQ(&y, &e3.get<0>());
}

TEST(eitherm, ref_assign_1) {
    // assignments from ref in second field
    double x=3.0;
    double y=5.0;
    either<void*, double&> e1(x);
    either<void*, double&> e2(y);

    e1=e2;
    ASSERT_EQ(1u, e1.index());
    EXPECT_EQ(5.0, e1.get<1>());
    EXPECT_EQ(&y, &e1.get<1>());
    EXPECT_EQ(3.0, x);

    either<void*, double&> e3(nullptr);
    e3=e2;
    ASSERT_EQ(1u, e3.index());
    EXPECT_EQ(&y, &e3.get<1>());
}

TEST(eitherm, ref_move_assign) {
    int a=1;
    double b=2;

    either<int&, double&> e1=a;
    either<int&, double&> e2=b;

    e2=std::move(e1);
    ASSERT_EQ(0, e2.index());
    EXPECT_EQ(&a, &e2.unsafe_get<0>());

    either<int&, double&> e3=a;
    either<int&, double&> e4=b;

    e3=std::move(e4);
    ASSERT_EQ(1, e3.index());
    EXPECT_EQ(&b, &e3.unsafe_get<1>());
}

TEST(eitherm, throw_in_assign) {
    struct throws_on_move {
        int value;
        explicit throws_on_move(int v): value(v) {}

        throws_on_move(const throws_on_move& x): value(x.value) {}
        throws_on_move& operator=(const throws_on_move& x) {
            value=x.value;
            return *this;
        }

        throws_on_move(throws_on_move&& x): value(x.value) {
            if (value==-1) throw -1;
        }
        throws_on_move& operator=(throws_on_move&& x) {
            value=x.value;
            if (value==-1) throw -1;
            return *this;
        }
    };

    either<int, throws_on_move> e1(10);
    either<int, throws_on_move> e2(throws_on_move(2));

    EXPECT_NO_THROW(e1=std::move(e2));
    EXPECT_EQ(1u, e1.index());
    EXPECT_EQ(2, e1.get<1>().value);

    either<int, throws_on_move> e3(10);
    either<int, throws_on_move> e4(throws_on_move(2));
    e4.get<1>().value = -1;
    try {
        e3=std::move(e4);
        ASSERT_TRUE(false);
    }
    catch (int) {}

    constexpr signed char npos = either<int, int>::either_npos;
    EXPECT_EQ(npos, e3.index());
    EXPECT_TRUE(e3.valueless_by_exception());
    EXPECT_NE(0u, e3.index());
    EXPECT_NE(1u, e3.index());

    e3=10;
    EXPECT_FALSE(e3.valueless_by_exception());

    either<int, throws_on_move> e5(throws_on_move(2));
    e5.get<1>().value = -1;
    try {
        e3=e5;
        ASSERT_TRUE(false);
    }
    catch (int) {}
    EXPECT_EQ(npos, e3.index());
    EXPECT_TRUE(e3.valueless_by_exception());
    EXPECT_NE(0u, e3.index());
    EXPECT_NE(1u, e3.index());
}
