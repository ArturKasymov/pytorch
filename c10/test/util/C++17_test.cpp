#include <c10/util/C++17.h>
#include <gtest/gtest.h>

namespace {

namespace test_if_constexpr {

using c10::guts::if_constexpr;

TEST(if_constexpr, whenIsTrue_thenReturnsTrueCase) {
  EXPECT_EQ(
      4, if_constexpr<true>([](auto) { return 4; }, [](auto) { return 5; }));
}

TEST(if_constexpr, whenIsFalse_thenReturnsFalseCase) {
  EXPECT_EQ(
      5, if_constexpr<false>([](auto) { return 4; }, [](auto) { return 5; }));
}

struct MovableOnly final {
  int value;

  MovableOnly(int v) : value(v) {}
  MovableOnly(MovableOnly&&) = default;
  MovableOnly(const MovableOnly&) = delete;
  MovableOnly& operator=(MovableOnly&&) = default;
  MovableOnly& operator=(const MovableOnly&) = delete;
};

TEST(if_constexpr, worksWithMovableOnlyTypes_withIdentityArg) {
  EXPECT_EQ(
      4,
      if_constexpr<true>(
          [](auto) { return MovableOnly(4); },
          [](auto) { return MovableOnly(5); })
          .value);
  EXPECT_EQ(
      5,
      if_constexpr<false>(
          [](auto) { return MovableOnly(4); },
          [](auto) { return MovableOnly(5); })
          .value);
}

TEST(if_constexpr, worksWithMovableOnlyTypes_withoutIdentityArg) {
  EXPECT_EQ(
      4,
      if_constexpr<true>(
          [] { return MovableOnly(4); }, [] { return MovableOnly(5); })
          .value);
  EXPECT_EQ(
      5,
      if_constexpr<false>(
          [] { return MovableOnly(4); }, [] { return MovableOnly(5); })
          .value);
}

struct MyClass1 {
  int value;
};

struct MyClass2 {
  int val;
};

template <class T>
int func(T t) {
  return if_constexpr<std::is_same<T, MyClass1>::value>(
      [&](auto _) {
        return _(t).value;
      }, // this code is invalid for T == MyClass2
      [&](auto _) { return _(t).val; } // this code is invalid for T == MyClass1
  );
}

TEST(if_constexpr, otherCaseCanHaveInvalidCode) {
  EXPECT_EQ(8, func(MyClass1{/* .value =  */ 8}));
  EXPECT_EQ(4, func(MyClass2{/* .val =  */ 4}));
}

TEST(if_constexpr, worksWithoutElseCase_withIdentityArg) {
  int var = 5;
  if_constexpr<false>([&](auto) { var = 3; });
  EXPECT_EQ(5, var);
  if_constexpr<true>([&](auto) { var = 3; });
  EXPECT_EQ(3, var);
}

TEST(if_constexpr, worksWithoutElseCase_withoutIdentityArg) {
  int var = 5;
  if_constexpr<false>([&] { var = 3; });
  EXPECT_EQ(5, var);
  if_constexpr<true>([&] { var = 3; });
  EXPECT_EQ(3, var);
}

TEST(if_constexpr, returnTypeCanDiffer_withIdentityArg) {
  auto a_string = if_constexpr<false>(
      [&](auto) -> int64_t { return 3; },
      [&](auto) -> std::string { return "3"; });
  static_assert(std::is_same<std::string, decltype(a_string)>::value, "");

  // NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores)
  auto an_int = if_constexpr<true>(
      [&](auto) -> int64_t { return 3; },
      [&](auto) -> std::string { return "3"; });
  static_assert(std::is_same<int64_t, decltype(an_int)>::value, "");
}

TEST(if_constexpr, returnTypeCanDiffer_withoutIdentityArg) {
  auto a_string = if_constexpr<false>(
      [&]() -> int64_t { return 3; }, [&]() -> std::string { return "3"; });
  static_assert(std::is_same<std::string, decltype(a_string)>::value, "");

  // NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores)
  auto an_int = if_constexpr<true>(
      [&]() -> int64_t { return 3; }, [&]() -> std::string { return "3"; });
  static_assert(std::is_same<int64_t, decltype(an_int)>::value, "");
}

} // namespace test_if_constexpr
} // namespace
