#include <gtest/gtest.h>

#include "uninitialized.h"

using namespace hf;

struct nocopy {
    nocopy() {}
    nocopy(const nocopy &n) = delete;
    nocopy(nocopy &&n) { ++move_ctor_count; }

    nocopy &operator=(const nocopy &n) = delete;
    nocopy &operator=(nocopy &&n) { ++move_assign_count; return *this; }

    static int move_ctor_count,move_assign_count;
    static void reset_counts() { move_ctor_count=move_assign_count=0; }
};

int nocopy::move_ctor_count=0;
int nocopy::move_assign_count=0;

TEST(uninitialized,ctor_nocopy) {
    nocopy::reset_counts();

    uninitialized<nocopy> ua;
    ua.construct(nocopy{});

    EXPECT_EQ(1,nocopy::move_ctor_count);
    EXPECT_EQ(0,nocopy::move_assign_count);

    ua.ref()=nocopy{};

    EXPECT_EQ(1,nocopy::move_ctor_count);
    EXPECT_EQ(1,nocopy::move_assign_count);
}

struct nomove {
    nomove() {}
    nomove(const nomove &n) { ++copy_ctor_count; }
    nomove(nomove &&n) = delete;

    nomove &operator=(const nomove &n) { ++copy_assign_count; return *this; }
    nomove &operator=(nomove &&n) = delete;

    static int copy_ctor_count,copy_assign_count;
    static void reset_counts() { copy_ctor_count=copy_assign_count=0; }
};

int nomove::copy_ctor_count=0;
int nomove::copy_assign_count=0;

TEST(uninitialized,ctor_nomove) {
    nomove::reset_counts();

    uninitialized<nomove> ua;
    ua.construct(nomove{});

    EXPECT_EQ(1,nomove::copy_ctor_count);
    EXPECT_EQ(0,nomove::copy_assign_count);

    nomove a;
    ua.ref()=a;

    EXPECT_EQ(1,nomove::copy_ctor_count);
    EXPECT_EQ(1,nomove::copy_assign_count);
}

