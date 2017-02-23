#include <gtest/gtest.h>

#include <optionalm/either.h>

#include "test_common.h"

using namespace hf::optionalm;


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
    either<int, const char*> e2(123);
    either<int, const char*> e3("def");

    e2=e1;
    EXPECT_EQ(1, e2.index());
    EXPECT_STREQ(e1.get<1>(), e2.get<1>());

    e3=std::move(e1);
    EXPECT_EQ(1, e3.index());
    EXPECT_STREQ("abc", e3.get<1>());
}

#if 0
TEST(eitherm, ref_assign) {
    double x=3.0;
    double y=5.0;

    either<int, double&> e1(x);
    either<int, double&> e2(y);

.* WIP .... */

    e1=e2;
    EXPECT_EQ(5.0, e1.get<1>());

    e1.get<1>()+=7.;
    EXPECT_EQ(12.0, e1.get<1>());
}
#endif

