#ifndef TERMITE_EXPECTED_H
#define TERMITE_EXPECTED_H

#include <functional>
#include <initializer_list>
#include <memory>
#include <type_traits>
#include <utility>

#ifndef TERMITE_ASSERT
#ifdef __GNUC__
#define TERMITE_UNREACHABLE (__builtin_unreachable())
#elif defined(_MSC_VER)
#define TERMITE_UNREACHABLE (__assume(false))
#else
#define TERMITE_UNREACHABLE ((void)0)
#endif

#define TERMITE_ASSERT(cond)                                        \
  do {                                                              \
    if (std::is_constant_evaluated() && !static_cast<bool>(cond)) { \
      TERMITE_UNREACHABLE;                                          \
    }                                                               \
  } while (false)
#endif

namespace termite {
template <typename T, typename E>
class Expected;

template <typename E>
class Unexpected;

struct InPlaceTag {
  explicit InPlaceTag() = default;
};

inline constexpr InPlaceTag kInPlace{};

struct UnexpectTag {
  explicit UnexpectTag() = default;
};

inline constexpr UnexpectTag kUnexpect{};

namespace detail {

template <typename T>
struct IsUnexpectedImpl : std::false_type {};

template <typename E>
struct IsUnexpectedImpl<Unexpected<E>> : std::true_type {};

template <typename T>
concept IsUnexpected = IsUnexpectedImpl<T>::value;

template <typename E>
concept CanBeUnexpected = std::is_object_v<E> && (!std::is_array_v<E>) &&
                          (!detail::IsUnexpected<E>) && (!std::is_const_v<E>) &&
                          (!std::is_volatile_v<E>);
}  // namespace detail

template <typename E>
class Unexpected {
  static_assert(detail::CanBeUnexpected<E>);

 public:
  constexpr Unexpected() = delete;

  constexpr Unexpected(const Unexpected&) = default;
  constexpr Unexpected(Unexpected&&) = default;

  constexpr Unexpected& operator=(const Unexpected&) = default;
  constexpr Unexpected& operator=(Unexpected&&) = default;

  constexpr ~Unexpected() = default;

  template <typename Err = E>
    requires(!std::is_same_v<std::remove_cvref_t<Err>, Unexpected>) &&
            (!std::is_same_v<std::remove_cvref_t<Err>, InPlaceTag>) &&
            std::is_constructible_v<E, Err>
  constexpr explicit Unexpected(Err&& e) : unex_(std::forward<Err>(e)) {}

  template <typename... Args>
    requires std::is_constructible_v<E, Args...>
  constexpr explicit Unexpected(InPlaceTag, Args&&... args)
      : unex_(std::forward<Args>(args)...) {}

  template <typename U, typename... Args>
    requires std::is_constructible_v<E, std::initializer_list<U>&, Args...>
  constexpr explicit Unexpected(InPlaceTag, std::initializer_list<U> il,
                                Args&&... args)
      : unex_(il, std::forward<Args>(args)...) {}

  constexpr const E& Error() const& noexcept { return unex_; }

  constexpr E& Error() & noexcept { return unex_; }

  constexpr const E&& Error() const&& noexcept { return std::move(unex_); }

  constexpr E&& Error() && noexcept { return std::move(unex_); }

  constexpr void swap(Unexpected& other) noexcept(
      std::is_nothrow_swappable_v<E>)
    requires std::is_swappable_v<E>
  {
    using std::swap;
    swap(unex_, other.unex_);
  }

  template <typename G>
  friend constexpr bool operator==(const Unexpected& x,
                                   const Unexpected<G>& y) noexcept {
    return x.Error() == y.Error();
  }

  friend constexpr void swap(Unexpected& x,
                             Unexpected& y) noexcept(noexcept(x.swap(y)))
    requires std::is_swappable_v<E>
  {
    x.swap(y);
  }

 private:
  E unex_;
};

template <typename E>
Unexpected(E) -> Unexpected<E>;

namespace detail {
template <typename T>
struct IsExpectedImpl : std::false_type {};

template <typename T, typename E>
struct IsExpectedImpl<Expected<T, E>> : std::true_type {};

template <typename T>
concept IsExpected = IsExpectedImpl<T>::value;

template <typename T>
concept CanBeExpected =
    std::is_void_v<T> ||
    (std::is_object_v<T> && (!std::is_array_v<T>) &&
     (!std::is_same_v<T, InPlaceTag>) && (!std::is_same_v<T, UnexpectTag>) &&
     (!detail::IsUnexpected<T>));

template <typename T>
class Guard {
 public:
  static_assert(std::is_nothrow_move_constructible_v<T>);

  constexpr Guard() = delete;

  constexpr Guard(const Guard&) = delete;

  constexpr Guard(Guard&&) = default;

  constexpr explicit Guard(T* x) : guarded_(x), tmp_(std::move(*x)) {
    std::destroy_at(guarded_);
  }

  constexpr ~Guard() {
    if (guarded_) [[unlikely]] {
      std::construct_at(guarded_, std::move(tmp_));
    }
  }

  constexpr Guard& operator=(const Guard&) = delete;

  constexpr Guard& operator=(Guard&&) = default;

  constexpr T&& Release() noexcept {
    guarded_ = nullptr;
    return std::move(tmp_);
  }

 private:
  T* guarded_;
  T tmp_;
};

template <typename T, typename U, typename... Args>
constexpr void ReinitExpected(T* new_val, U* old_val, Args&&... args) {
  if constexpr (std::is_nothrow_constructible_v<T, Args...>) {
    std::destroy_at(old_val);
    std::construct_at(new_val, std::forward<Args>(args)...);
  } else if constexpr (std::is_nothrow_move_constructible_v<T>) {
    T tmp(std::forward<Args>(args)...);
    std::destroy_at(old_val);
    std::construct_at(new_val, std::move(tmp));
  } else {
    Guard<U> guard(old_val);
    std::construct_at(new_val, std::forward<Args>(args)...);
    guard.Release();
  }
}

}  // namespace detail

template <typename T, typename E>
class Expected {
  static_assert(!std::is_void_v<T>);
  static_assert(detail::CanBeExpected<std::remove_cv_t<T>>);
  static_assert(detail::CanBeUnexpected<E>);
  static_assert(std::is_destructible_v<T> && std::is_destructible_v<E>);

  template <typename U, typename G>
  static constexpr bool kConsFromExpected = std::conjunction_v<
      std::disjunction<std::is_same<std::remove_cv_t<T>, bool>,
                       std::negation<std::disjunction<
                           std::is_constructible<T, Expected<U, G>&>,
                           std::is_constructible<T, Expected<U, G>>,
                           std::is_constructible<T, const Expected<U, G>&>,
                           std::is_constructible<T, const Expected<U, G>>,
                           std::is_convertible<Expected<U, G>&, T>,
                           std::is_convertible<Expected<U, G>, T>,
                           std::is_convertible<const Expected<U, G>&, T>,
                           std::is_convertible<const Expected<U, G>, T>>>>,
      std::negation<std::disjunction<
          std::is_constructible<Unexpected<E>, Expected<U, G>&>,
          std::is_constructible<Unexpected<E>, Expected<U, G>>,
          std::is_constructible<Unexpected<E>, const Expected<U, G>&>,
          std::is_constructible<Unexpected<E>, const Expected<U, G>>>>>;

  template <typename U, typename G>
  static constexpr bool kExplicitConv =
      std::disjunction_v<std::negation<std::is_convertible<U, T>>,
                         std::negation<std::is_convertible<G, E>>>;

 public:
  using ValueType = T;
  using ErrorType = E;

  constexpr Expected()
    requires std::is_default_constructible_v<T>
      : val_(), has_val_(true) {}

  constexpr Expected(const Expected& rhs) = delete;

  constexpr Expected(const Expected& rhs)
    requires std::is_copy_constructible_v<T> &&
             std::is_copy_constructible_v<E> &&
             ((!std::is_trivially_copy_constructible_v<T>) ||
              (!std::is_trivially_copy_constructible_v<E>))
      : has_val_(rhs.HasValue()) {
    if (rhs.HasValue()) {
      std::construct_at(std::addressof(val_), *rhs);
    } else {
      std::construct_at(std::addressof(unex_), rhs.Error());
    }
  }

  constexpr Expected(const Expected& rhs)
    requires std::is_copy_constructible_v<T> &&
                 std::is_copy_constructible_v<E> &&
                 std::is_trivially_copy_constructible_v<T> &&
                 std::is_trivially_copy_constructible_v<E>
  = default;

  constexpr Expected(Expected&& rhs) noexcept(
      std::conjunction_v<std::is_nothrow_move_constructible<T>,
                         std::is_nothrow_move_constructible<E>>)
    requires std::is_move_constructible_v<T> &&
             std::is_move_constructible_v<E> &&
             ((!std::is_trivially_move_constructible_v<T>) ||
              (!std::is_trivially_move_constructible_v<E>))
      : has_val_(rhs.HasValue()) {
    if (rhs.HasValue()) {
      std::construct_at(std::addressof(val_), std::move(*rhs));
    } else {
      std::construct_at(std::addressof(unex_), std::move(rhs.Error()));
    }
  }

  constexpr Expected(Expected&& rhs) noexcept(
      std::conjunction_v<std::is_nothrow_move_constructible<T>,
                         std::is_nothrow_move_constructible<E>>)
    requires std::is_move_constructible_v<T> &&
                 std::is_move_constructible_v<E> &&
                 std::is_trivially_move_constructible_v<T> &&
                 std::is_trivially_move_constructible_v<E>
  = default;

  template <typename U, typename G>
    requires std::is_constructible_v<T, const U&> &&
             std::is_constructible_v<E, const G&> && kConsFromExpected<U, G>
  constexpr explicit(kExplicitConv<const U&, const G&>)
      Expected(const Expected<U, G>& rhs)
      : has_val_(rhs.HasValue()) {
    if (rhs.HasValue()) {
      std::construct_at(std::addressof(val_), std::forward<const U&>(*rhs));
    } else {
      std::construct_at(std::addressof(unex_),
                        std::forward<const G&>(rhs.Error()));
    }
  }

  template <typename U, typename G>
    requires std::is_constructible_v<T, U> && std::is_constructible_v<E, G> &&
             kConsFromExpected<U, G>
  constexpr explicit(kExplicitConv<U, G>) Expected(Expected<U, G>&& rhs)
      : has_val_(rhs.HasValue()) {
    if (rhs.HasValue()) {
      std::construct_at(std::addressof(val_), std::forward<U>(*rhs));
    } else {
      std::construct_at(std::addressof(unex_), std::forward<G>(rhs.Error()));
    }
  }

  template <typename U = std::remove_cv_t<T>>
    requires(!std::is_same_v<std::remove_cvref_t<U>, InPlaceTag>) &&
                (!std::is_same_v<std::remove_cvref_t<U>, Expected>) &&
                (!std::is_same_v<std::remove_cvref_t<U>, UnexpectTag>) &&
                (!detail::IsUnexpected<std::remove_cvref_t<U>>) &&
                std::is_constructible_v<T, U> &&
                ((!std::is_same_v<std::remove_cv_t<T>, bool>) ||
                 (!detail::IsExpected<std::remove_cvref_t<U>>))
  constexpr explicit(!std::is_convertible_v<U, T>) Expected(U&& v)
      : val_(std::forward<U>(v)), has_val_(true) {}

  template <typename G>
    requires std::is_constructible_v<E, const G&>
  constexpr explicit(!std::is_convertible_v<const G&, E>)
      Expected(const Unexpected<G>& e)
      : unex_(std::forward<const G&>(e.Error())), has_val_(false) {}

  template <typename G>
    requires std::is_constructible_v<E, G>
  constexpr explicit(!std::is_convertible_v<G, E>) Expected(Unexpected<G>&& e)
      : unex_(std::forward<G>(e.Error())), has_val_(false) {}

  template <typename... Args>
    requires std::is_constructible_v<T, Args...>
  constexpr explicit Expected(InPlaceTag, Args&&... args)
      : val_(std::forward<Args>(args)...), has_val_(true) {}

  template <typename U, typename... Args>
    requires std::is_constructible_v<T, std::initializer_list<U>&, Args...>
  constexpr explicit Expected(InPlaceTag, std::initializer_list<U> il,
                              Args&&... args)
      : val_(il, std::forward<Args>(args)...), has_val_(true) {}

  template <typename... Args>
    requires std::is_constructible_v<E, Args...>
  constexpr explicit Expected(UnexpectTag, Args&&... args)
      : unex_(std::forward<Args>(args)...), has_val_(false) {}

  template <typename U, typename... Args>
    requires std::is_constructible_v<E, std::initializer_list<U>&, Args...>
  constexpr explicit Expected(UnexpectTag, std::initializer_list<U> il,
                              Args&&... args)
      : unex_(il, std::forward<Args>(args)...), has_val_(false) {}

  constexpr ~Expected() {
    if (HasValue()) {
      std::destroy_at(std::addressof(val_));
    } else {
      std::destroy_at(std::addressof(unex_));
    }
  }

  constexpr ~Expected()
    requires std::is_trivially_destructible_v<T> &&
                 std::is_trivially_destructible_v<E>
  = default;

  constexpr Expected& operator=(const Expected& rhs) = delete;

  constexpr Expected& operator=(const Expected& rhs)
    requires std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T> &&
             std::is_copy_constructible_v<E> && std::is_copy_assignable_v<E> &&
             (std::is_nothrow_move_constructible_v<T> ||
              std::is_nothrow_move_constructible_v<E>)
  {
    if (this == &rhs) {
      return *this;
    }

    if (HasValue() && rhs.HasValue()) {
      val_ = *rhs;
    } else if (HasValue()) {
      detail::ReinitExpected(std::addressof(unex_), std::addressof(val_),
                             rhs.Error());
    } else if (rhs.HasValue()) {
      detail::ReinitExpected(std::addressof(val_), std::addressof(unex_), *rhs);
    } else {
      unex_ = rhs.Error();
    }

    has_val_ = rhs.HasValue();
    return *this;
  }

  constexpr Expected& operator=(Expected&& rhs) noexcept(
      std::conjunction_v<std::is_nothrow_move_constructible<T>,
                         std::is_nothrow_move_assignable<T>,
                         std::is_nothrow_move_constructible<E>,
                         std::is_nothrow_move_assignable<E>>)
    requires std::is_move_constructible_v<T> && std::is_move_assignable_v<T> &&
             std::is_move_constructible_v<E> && std::is_move_assignable_v<E> &&
             (std::is_nothrow_move_constructible_v<T> ||
              std::is_nothrow_move_constructible_v<E>)
  {
    if (this == &rhs) {
      return *this;
    }

    if (HasValue() && rhs.HasValue()) {
      val_ = std::move(*rhs);
    } else if (HasValue()) {
      detail::ReinitExpected(std::addressof(unex_), std::addressof(val_),
                             std::move(rhs.Error()));
    } else if (rhs.HasValue()) {
      detail::ReinitExpected(std::addressof(val_), std::addressof(unex_),
                             std::move(*rhs));
    } else {
      unex_ = std::move(rhs.Error());
    }
    has_val_ = rhs.HasValue();
    return *this;
  }

  template <typename U = std::remove_cv_t<T>>
    requires(!std::is_same_v<Expected, std::remove_cvref_t<U>>) &&
            (!detail::IsUnexpected<std::remove_cvref_t<U>>) &&
            std::is_constructible_v<T, U> && std::is_assignable_v<T&, U> &&
            (std::is_nothrow_constructible_v<T, U> ||
             std::is_nothrow_move_constructible_v<T> ||
             std::is_nothrow_move_constructible_v<E>)
  constexpr Expected& operator=(U&& v) {
    if (HasValue()) {
      val_ = std::forward<U>(v);
    } else {
      detail::ReinitExpected(std::addressof(val_), std::addressof(unex_),
                             std::forward<U>(v));
      has_val_ = true;
    }
    return *this;
  }

  template <typename G>
    requires std::is_constructible_v<E, const G&> &&
             std::is_assignable_v<E&, const G&> &&
             (std::is_nothrow_constructible_v<E, const G&> ||
              std::is_nothrow_move_constructible_v<T> ||
              std::is_nothrow_move_constructible_v<E>)
  constexpr Expected& operator=(const Unexpected<G>& e) {
    if (HasValue()) {
      detail::ReinitExpected(std::addressof(unex_), std::addressof(val_),
                             std::forward<const G&>(e.Error()));
      has_val_ = false;
    } else {
      unex_ = std::forward<const G&>(e.Error());
    }
    return *this;
  }

  template <typename G>
    requires std::is_constructible_v<E, G> && std::is_assignable_v<E&, G> &&
             (std::is_nothrow_constructible_v<E, G> ||
              std::is_nothrow_move_constructible_v<T> ||
              std::is_nothrow_move_constructible_v<E>)
  constexpr Expected& operator=(Unexpected<G>&& e) {
    if (HasValue()) {
      detail::ReinitExpected(std::addressof(unex_), std::addressof(val_),
                             std::forward<G>(e.Error()));
      has_val_ = false;
    } else {
      unex_ = std::forward<G>(e.Error());
    }
    return *this;
  }

  template <typename... Args>
    requires std::is_nothrow_constructible_v<T, Args...>
  constexpr T& Emplace(Args&&... args) noexcept {
    if (HasValue()) {
      std::destroy_at(std::addressof(val_));
    } else {
      std::destroy_at(std::addressof(unex_));
      has_val_ = true;
    }
    return *std::construct_at(std::addressof(val_),
                              std::forward<Args>(args)...);
  }

  template <typename U, typename... Args>
    requires std::is_nothrow_constructible_v<T, std::initializer_list<U>&,
                                             Args...>
  constexpr T& Emplace(std::initializer_list<U> il, Args&&... args) noexcept {
    if (HasValue()) {
      std::destroy_at(std::addressof(val_));
    } else {
      std::destroy_at(std::addressof(unex_));
      has_val_ = true;
    }
    return *std::construct_at(std::addressof(val_), il,
                              std::forward<Args>(args)...);
  }

  constexpr void swap(Expected& rhs) noexcept(
      std::conjunction_v<
          std::is_nothrow_move_constructible<T>, std::is_nothrow_swappable<T>,
          std::is_nothrow_move_constructible<E>, std::is_nothrow_swappable<E>>)
    requires std::is_swappable_v<T> && std::is_swappable_v<E> &&
             std::is_move_constructible_v<T> &&
             std::is_move_constructible_v<E> &&
             (std::is_nothrow_move_constructible_v<T> ||
              std::is_nothrow_move_constructible_v<E>)
  {
    if (HasValue() && rhs.HasValue()) {
      using std::swap;
      swap(val_, rhs.val_);
    } else if (HasValue()) {
      if constexpr (std::is_nothrow_move_constructible_v<E>) {
        detail::Guard guard(std::addressof(rhs.unex_));
        std::construct_at(std::addressof(rhs.val_), std::move(val_));
        std::destroy_at(std::addressof(val_));
        std::construct_at(std::addressof(unex_), guard.Release());
      } else {
        detail::Guard guard(std::addressof(val_));
        std::construct_at(std::addressof(unex_), rhs.unex_);
        std::destroy_at(std::addressof(rhs.unex_));
        std::construct_at(std::addressof(rhs.val_), guard.Release());
      }
      has_val_ = false;
      rhs.has_val_ = true;
    } else if (rhs.HasValue()) {
      rhs.swap(*this);
    } else {
      using std::swap;
      swap(unex_, rhs.unex_);
    }
  }

  friend constexpr void swap(Expected& x,
                             Expected& y) noexcept(noexcept(x.swap(y)))
    requires std::is_swappable_v<T> && std::is_swappable_v<E> &&
             std::is_move_constructible_v<T> &&
             std::is_move_constructible_v<E> &&
             (std::is_nothrow_move_constructible_v<T> ||
              std::is_nothrow_move_constructible_v<E>)
  {
    x.swap(y);
  }

  constexpr const T* operator->() const noexcept {
    TERMITE_ASSERT(has_val_);
    return std::addressof(val_);
  }

  constexpr T* operator->() noexcept {
    TERMITE_ASSERT(has_val_);
    return std::addressof(val_);
  }

  constexpr const T& operator*() const& noexcept {
    TERMITE_ASSERT(has_val_);
    return val_;
  }

  constexpr T& operator*() & noexcept {
    TERMITE_ASSERT(has_val_);
    return val_;
  }

  constexpr const T&& operator*() const&& noexcept {
    TERMITE_ASSERT(has_val_);
    return std::move(val_);
  }

  constexpr T&& operator*() && noexcept {
    TERMITE_ASSERT(has_val_);
    return std::move(val_);
  }

  constexpr explicit operator bool() const noexcept { return has_val_; }

  constexpr bool HasValue() const noexcept { return has_val_; }

  constexpr const E& Error() const& noexcept {
    TERMITE_ASSERT(!has_val_);
    return unex_;
  }

  constexpr E& Error() & noexcept {
    TERMITE_ASSERT(!has_val_);
    return unex_;
  }

  constexpr const E&& Error() const&& noexcept {
    TERMITE_ASSERT(!has_val_);
    return std::move(unex_);
  }

  constexpr E&& Error() && noexcept {
    TERMITE_ASSERT(!has_val_);
    return std::move(unex_);
  }

  template <typename U = std::remove_cv_t<T>>
  constexpr T ValueOr(U&& v) const& {
    static_assert(std::is_copy_constructible_v<T> &&
                  std::is_convertible_v<U, T>);

    if (HasValue()) {
      return **this;
    } else {
      return static_cast<T>(std::forward<U>(v));
    }
  }

  template <typename U = std::remove_cv_t<T>>
  constexpr T ValueOr(U&& v) && {
    static_assert(std::is_move_constructible_v<T> &&
                  std::is_convertible_v<U, T>);

    if (HasValue()) {
      return std::move(**this);
    } else {
      return static_cast<T>(std::forward<U>(v));
    }
  }

  template <typename G = E>
  constexpr E ErrorOr(G&& e) const& {
    static_assert(std::is_copy_constructible_v<E> &&
                  std::is_convertible_v<G, E>);

    if (HasValue()) {
      return std::forward<G>(e);
    } else {
      return Error();
    }
  }

  template <typename G = E>
  constexpr E ErrorOr(G&& e) && {
    static_assert(std::is_move_constructible_v<E> &&
                  std::is_convertible_v<G, E>);

    if (HasValue()) {
      return std::forward<G>(e);
    } else {
      return std::move(Error());
    }
  }

  template <typename F>
    requires std::is_constructible_v<E, E&>
  constexpr auto AndThen(F&& f) & {
    using U = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
    static_assert(detail::IsExpected<U>);
    static_assert(std::is_same_v<typename U::ErrorType, E>);

    if (HasValue()) {
      return std::invoke(std::forward<F>(f), val_);
    } else {
      return U(kUnexpect, Error());
    }
  }

  template <typename F>
    requires std::is_constructible_v<E, const E&>
  constexpr auto AndThen(F&& f) const& {
    using U = std::remove_cvref_t<std::invoke_result_t<F, const T&>>;
    static_assert(detail::IsExpected<U>);
    static_assert(std::is_same_v<typename U::ErrorType, E>);

    if (HasValue()) {
      return std::invoke(std::forward<F>(f), val_);
    } else {
      return U(kUnexpect, Error());
    }
  }

  template <typename F>
    requires std::is_constructible_v<E, E>
  constexpr auto AndThen(F&& f) && {
    using U = std::remove_cvref_t<std::invoke_result_t<F, T&&>>;
    static_assert(detail::IsExpected<U>);
    static_assert(std::is_same_v<typename U::ErrorType, E>);

    if (HasValue()) {
      return std::invoke(std::forward<F>(f), std::move(val_));
    } else {
      return U(kUnexpect, std::move(Error()));
    }
  }

  template <typename F>
    requires std::is_constructible_v<E, const E>
  constexpr auto AndThen(F&& f) const&& {
    using U = std::remove_cvref_t<std::invoke_result_t<F, const T&&>>;
    static_assert(detail::IsExpected<U>);
    static_assert(std::is_same_v<typename U::ErrorType, E>);

    if (HasValue()) {
      return std::invoke(std::forward<F>(f), std::move(val_));
    } else {
      return U(kUnexpect, std::move(Error()));
    }
  }

  template <typename F>
    requires std::is_constructible_v<T, T&>
  constexpr auto OrElse(F&& f) & {
    using G = std::remove_cvref_t<std::invoke_result_t<F, E&>>;
    static_assert(detail::IsExpected<G>);
    static_assert(std::is_same_v<typename G::ValueType, T>);

    if (HasValue()) {
      return G(kInPlace, val_);
    } else {
      return std::invoke(std::forward<F>(f), Error());
    }
  }

  template <typename F>
    requires std::is_constructible_v<T, const T&>
  constexpr auto OrElse(F&& f) const& {
    using G = std::remove_cvref_t<std::invoke_result_t<F, const E&>>;
    static_assert(detail::IsExpected<G>);
    static_assert(std::is_same_v<typename G::ValueType, T>);

    if (HasValue()) {
      return G(kInPlace, val_);
    } else {
      return std::invoke(std::forward<F>(f), Error());
    }
  }

  template <typename F>
    requires std::is_constructible_v<T, T>
  constexpr auto OrElse(F&& f) && {
    using G = std::remove_cvref_t<std::invoke_result_t<F, E&&>>;
    static_assert(detail::IsExpected<G>);
    static_assert(std::is_same_v<typename G::ValueType, T>);

    if (HasValue()) {
      return G(kInPlace, std::move(val_));
    } else {
      return std::invoke(std::forward<F>(f), std::move(Error()));
    }
  }

  template <typename F>
    requires std::is_constructible_v<T, const T>
  constexpr auto OrElse(F&& f) const&& {
    using G = std::remove_cvref_t<std::invoke_result_t<F, const E&&>>;
    static_assert(detail::IsExpected<G>);
    static_assert(std::is_same_v<typename G::ValueType, T>);

    if (HasValue()) {
      return G(kInPlace, std::move(val_));
    } else {
      return std::invoke(std::forward<F>(f), std::move(Error()));
    }
  }

  template <typename F>
    requires std::is_constructible_v<E, E&>
  constexpr auto Transform(F&& f) & {
    using U = std::remove_cv_t<std::invoke_result_t<F, T&>>;
    using Result = Expected<U, E>;

    if (HasValue()) {
      if constexpr (std::is_void_v<U>) {
        std::invoke(std::forward<F>(f), val_);
        return Result();
      } else {
        return Result(kInPlace, std::invoke(std::forward<F>(f), val_));
      }
    } else {
      return Result(kUnexpect, Error());
    }
  }

  template <typename F>
    requires std::is_constructible_v<E, const E&>
  constexpr auto Transform(F&& f) const& {
    using U = std::remove_cv_t<std::invoke_result_t<F, const T&>>;
    using Result = Expected<U, E>;

    if (HasValue()) {
      if constexpr (std::is_void_v<U>) {
        std::invoke(std::forward<F>(f), val_);
        return Result();
      } else {
        return Result(kInPlace, std::invoke(std::forward<F>(f), val_));
      }
    } else {
      return Result(kUnexpect, Error());
    }
  }

  template <typename F>
    requires std::is_constructible_v<E, E>
  constexpr auto Transform(F&& f) && {
    using U = std::remove_cv_t<std::invoke_result_t<F, T&&>>;
    using Result = Expected<U, E>;

    if (HasValue()) {
      if constexpr (std::is_void_v<U>) {
        std::invoke(std::forward<F>(f), std::move(val_));
        return Result();
      } else {
        return Result(kInPlace,
                      std::invoke(std::forward<F>(f), std::move(val_)));
      }
    } else {
      return Result(kUnexpect, std::move(Error()));
    }
  }

  template <typename F>
    requires std::is_constructible_v<E, const E>
  constexpr auto Transform(F&& f) const&& {
    using U = std::remove_cv_t<std::invoke_result_t<F, const T&&>>;
    using Result = Expected<U, E>;

    if (HasValue()) {
      if constexpr (std::is_void_v<U>) {
        std::invoke(std::forward<F>(f), std::move(val_));
        return Result();
      } else {
        return Result(kInPlace,
                      std::invoke(std::forward<F>(f), std::move(val_)));
      }
    } else {
      return Result(kUnexpect, std::move(Error()));
    }
  }

  template <typename F>
    requires std::is_constructible_v<T, T&>
  constexpr auto TransformError(F&& f) & {
    using G = std::remove_cv_t<std::invoke_result_t<F, E&>>;
    using Result = Expected<T, G>;

    if (HasValue()) {
      return Result(kInPlace, val_);
    } else {
      return Result(kUnexpect, std::invoke(std::forward<F>(f), Error()));
    }
  }

  template <typename F>
    requires std::is_constructible_v<T, const T&>
  constexpr auto TransformError(F&& f) const& {
    using G = std::remove_cv_t<std::invoke_result_t<F, const E&>>;
    using Result = Expected<T, G>;

    if (HasValue()) {
      return Result(kInPlace, val_);
    } else {
      return Result(kUnexpect, std::invoke(std::forward<F>(f), Error()));
    }
  }

  template <typename F>
    requires std::is_constructible_v<T, T>
  constexpr auto TransformError(F&& f) && {
    using G = std::remove_cv_t<std::invoke_result_t<F, E&&>>;
    using Result = Expected<T, G>;

    if (HasValue()) {
      return Result(kInPlace, std::move(val_));
    } else {
      return Result(kUnexpect,
                    std::invoke(std::forward<F>(f), std::move(Error())));
    }
  }

  template <typename F>
    requires std::is_constructible_v<T, const T>
  constexpr auto TransformError(F&& f) const&& {
    using G = std::remove_cv_t<std::invoke_result_t<F, const E&&>>;
    using Result = Expected<T, G>;

    if (HasValue()) {
      return Result(kInPlace, std::move(val_));
    } else {
      return Result(kUnexpect,
                    std::invoke(std::forward<F>(f), std::move(Error())));
    }
  }

  template <typename U, typename G>
    requires(!std::is_void_v<U>)
  friend constexpr bool operator==(const Expected& x, const Expected<U, G>& y) {
    if (x.HasValue() != y.HasValue()) {
      return false;
    } else if (x.HasValue()) {
      return *x == *y;
    } else {
      return x.Error() == y.Error();
    }
  }

  template <typename U>
    requires(!detail::IsExpected<U>)
  friend constexpr bool operator==(const Expected& x, const U& v) {
    return x.HasValue() && static_cast<bool>(*x == v);
  }

  template <typename G>
  friend constexpr bool operator==(const Expected& x, const Unexpected<G>& e) {
    return (!x.HasValue()) && static_cast<bool>(x.Error() == e.Error());
  }

 private:
  template <typename, typename>
  friend class Expected;

  union {
    T val_;
    E unex_;
  };
  bool has_val_;
};

template <typename T, typename E>
  requires std::is_void_v<T>
class Expected<T, E> {
  static_assert(detail::CanBeUnexpected<E>);
  static_assert(std::is_destructible_v<E>);

  template <typename U, typename G>
  static constexpr bool ConsFromExpected = std::negation_v<std::disjunction<
      std::is_constructible<Unexpected<E>, Expected<U, G>&>,
      std::is_constructible<Unexpected<E>, Expected<U, G>>,
      std::is_constructible<Unexpected<E>, const Expected<U, G>&>,
      std::is_constructible<Unexpected<E>, const Expected<U, G>>>>;

 public:
  using ValueType = T;
  using ErrorType = E;

  constexpr Expected() noexcept : has_val_(true) {}

  constexpr Expected(const Expected& rhs) = delete;

  constexpr Expected(const Expected& rhs)
    requires std::is_copy_constructible_v<E> &&
             (!std::is_trivially_copy_constructible_v<E>)
      : has_val_(rhs.HasValue()) {
    if (!rhs.HasValue()) {
      std::construct_at(std::addressof(Unex()), rhs.Error());
    }
  }

  constexpr Expected(const Expected& rhs)
    requires std::is_copy_constructible_v<E> &&
                 std::is_trivially_copy_constructible_v<E>
  = default;

  constexpr Expected(Expected&& rhs) noexcept(
      std::is_nothrow_move_constructible_v<E>)
    requires std::is_move_constructible_v<E> &&
             (!std::is_trivially_move_constructible_v<E>)
      : has_val_(rhs.HasValue()) {
    if (!rhs.HasValue()) {
      std::construct_at(std::addressof(Unex()), std::move(rhs.Error()));
    }
  }

  constexpr Expected(Expected&& rhs) noexcept(
      std::is_nothrow_move_constructible_v<E>)
    requires std::is_move_constructible_v<E> &&
                 std::is_trivially_move_constructible_v<E>
  = default;

  template <typename U, typename G>
    requires std::is_void_v<U> && std::is_constructible_v<E, const G&> &&
             ConsFromExpected<U, G>
  constexpr explicit(!std::is_convertible_v<const G&, E>)
      Expected(const Expected<U, G>& rhs)
      : has_val_(rhs.HasValue()) {
    if (!rhs.HasValue()) {
      std::construct_at(std::addressof(Unex()),
                        std::forward<const G&>(rhs.Error()));
    }
  }

  template <typename U, typename G>
    requires std::is_void_v<U> && std::is_constructible_v<E, G> &&
             ConsFromExpected<U, G>
  constexpr explicit(!std::is_convertible_v<G, E>)
      Expected(Expected<U, G>&& rhs)
      : has_val_(rhs.HasValue()) {
    if (!rhs.HasValue()) {
      std::construct_at(std::addressof(Unex()), std::forward<G>(rhs.Error()));
    }
  }

  template <typename G>
    requires std::is_constructible_v<E, const G&>
  constexpr explicit(!std::is_convertible_v<const G&, E>)
      Expected(const Unexpected<G>& e)
      : has_val_(false) {
    if (!HasValue()) {
      std::construct_at(std::addressof(Unex()),
                        std::forward<const G&>(e.Error()));
    }
  }

  template <typename G>
    requires std::is_constructible_v<E, G>
  constexpr explicit(!std::is_convertible_v<G, E>) Expected(Unexpected<G>&& e)
      : has_val_(false) {
    if (!HasValue()) {
      std::construct_at(std::addressof(Unex()), std::forward<G>(e.Error()));
    }
  }

  constexpr explicit Expected(InPlaceTag) noexcept : has_val_(true) {}

  template <typename... Args>
    requires std::is_constructible_v<E, Args...>
  constexpr explicit Expected(UnexpectTag, Args&&... args) : has_val_(false) {
    if (!HasValue()) {
      std::construct_at(std::addressof(Unex()), std::forward<Args>(args)...);
    }
  }

  template <typename U, typename... Args>
    requires std::is_constructible_v<E, std::initializer_list<U>&, Args...>
  constexpr explicit Expected(UnexpectTag, std::initializer_list<U> il,
                              Args&&... args)
      : has_val_(false) {
    if (!HasValue()) {
      std::construct_at(std::addressof(Unex()), il,
                        std::forward<Args>(args)...);
    }
  }

  constexpr ~Expected() {
    if (!HasValue()) {
      std::destroy_at(std::addressof(Unex()));
    }
  }

  constexpr ~Expected()
    requires std::is_trivially_destructible_v<E>
  = default;

  constexpr Expected& operator=(const Expected& rhs) = delete;

  constexpr Expected& operator=(const Expected& rhs)
    requires std::is_copy_assignable_v<E> && std::is_copy_constructible_v<E>
  {
    if (this == &rhs) {
      return *this;
    }

    if (HasValue() && rhs.HasValue()) {
    } else if (HasValue()) {
      std::construct_at(std::addressof(Unex()), rhs.Error());
      has_val_ = false;
    } else if (rhs.HasValue()) {
      std::destroy_at(std::addressof(Unex()));
      has_val_ = true;
    } else {
      Unex() = rhs.Error();
    }
    return *this;
  }

  constexpr Expected& operator=(Expected&& rhs) noexcept(
      std::conjunction_v<std::is_nothrow_move_constructible<E>,
                         std::is_nothrow_move_assignable<E>>)
    requires std::is_move_assignable_v<E> && std::is_move_constructible_v<E>
  {
    if (this == &rhs) {
      return *this;
    }

    if (HasValue() && rhs.HasValue()) {
    } else if (HasValue()) {
      std::construct_at(std::addressof(Unex()), std::move(rhs.Error()));
      has_val_ = false;
    } else if (rhs.HasValue()) {
      std::destroy_at(std::addressof(Unex()));
      has_val_ = true;
    } else {
      Unex() = std::move(rhs.Error());
    }
    return *this;
  }

  template <typename G>
    requires std::is_constructible_v<E, const G&> &&
             std::is_assignable_v<E&, const G&>
  constexpr Expected& operator=(const Unexpected<G>& e) {
    if (HasValue()) {
      std::construct_at(std::addressof(Unex()),
                        std::forward<const G&>(e.Error()));
      has_val_ = false;
    } else {
      Unex() = std::forward<const G&>(e.Error());
    }
    return *this;
  }

  template <typename G>
    requires std::is_constructible_v<E, G> && std::is_assignable_v<E&, G>
  constexpr Expected& operator=(Unexpected<G>&& e) {
    if (HasValue()) {
      std::construct_at(std::addressof(Unex()), std::forward<G>(e.Error()));
      has_val_ = false;
    } else {
      Unex() = std::forward<G>(e.Error());
    }
    return *this;
  }

  constexpr void Emplace() noexcept {
    if (!HasValue()) {
      std::destroy_at(std::addressof(Unex()));
      has_val_ = true;
    }
  }

  constexpr void swap(Expected& rhs) noexcept(
      std::conjunction_v<std::is_nothrow_move_constructible<E>,
                         std::is_nothrow_swappable<E>>)
    requires std::is_swappable_v<E> && std::is_move_constructible_v<E>
  {
    if (HasValue() && rhs.HasValue()) {
    } else if (HasValue()) {
      std::construct_at(std::addressof(Unex()), std::move(rhs.Unex()));
      std::destroy_at(std::addressof(rhs.Unex()));
      has_val_ = false;
      rhs.has_val_ = true;
    } else if (rhs.HasValue()) {
      rhs.swap(*this);
    } else {
      using std::swap;
      swap(Unex(), rhs.Unex());
    }
  }

  friend constexpr void swap(Expected& x,
                             Expected& y) noexcept(noexcept(x.swap(y)))
    requires std::is_swappable_v<E> && std::is_move_constructible_v<E>
  {
    x.swap(y);
  }

  constexpr explicit operator bool() const noexcept { return has_val_; }

  constexpr bool HasValue() const noexcept { return has_val_; }

  constexpr void operator*() const noexcept { TERMITE_ASSERT(has_val_); }

  constexpr const E& Error() const& noexcept {
    TERMITE_ASSERT(!has_val_);
    return Unex();
  }

  constexpr E& Error() & noexcept {
    TERMITE_ASSERT(!has_val_);
    return Unex();
  }

  constexpr E&& Error() && noexcept {
    TERMITE_ASSERT(!has_val_);
    return std::move(Unex());
  }

  constexpr const E&& Error() const&& noexcept {
    TERMITE_ASSERT(!has_val_);
    return std::move(Unex());
  }

  template <typename G = E>
    requires std::is_copy_constructible_v<E> && std::is_convertible_v<G, E>
  constexpr E ErrorOr(G&& e) const& {
    if (HasValue()) {
      return std::forward<G>(e);
    } else {
      return Error();
    }
  }

  template <typename G = E>
    requires std::is_move_constructible_v<E> && std::is_convertible_v<G, E>
  constexpr E ErrorOr(G&& e) && {
    if (HasValue()) {
      return std::forward<G>(e);
    } else {
      return std::move(Error());
    }
  }

  template <typename F>
    requires std::is_constructible_v<E, E&>
  constexpr auto AndThen(F&& f) & {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(detail::IsExpected<U>);
    static_assert(std::is_same_v<typename U::ErrorType, E>);

    if (HasValue()) {
      return std::invoke(std::forward<F>(f));
    } else {
      return U(kUnexpect, Error());
    }
  }

  template <typename F>
    requires std::is_constructible_v<E, const E&>
  constexpr auto AndThen(F&& f) const& {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(detail::IsExpected<U>);
    static_assert(std::is_same_v<typename U::ErrorType, E>);

    if (HasValue()) {
      return std::invoke(std::forward<F>(f));
    } else {
      return U(kUnexpect, Error());
    }
  }

  template <typename F>
    requires std::is_constructible_v<E, E>
  constexpr auto AndThen(F&& f) && {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(detail::IsExpected<U>);
    static_assert(std::is_same_v<typename U::ErrorType, E>);

    if (HasValue()) {
      return std::invoke(std::forward<F>(f));
    } else {
      return U(kUnexpect, std::move(Error()));
    }
  }

  template <typename F>
    requires std::is_constructible_v<E, const E>
  constexpr auto AndThen(F&& f) const&& {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(detail::IsExpected<U>);
    static_assert(std::is_same_v<typename U::ErrorType, E>);

    if (HasValue()) {
      return std::invoke(std::forward<F>(f));
    } else {
      return U(kUnexpect, std::move(Error()));
    }
  }

  template <typename F>
  constexpr auto OrElse(F&& f) & {
    using G = std::remove_cvref_t<std::invoke_result_t<F, E&>>;
    static_assert(detail::IsExpected<G>);
    static_assert(std::is_same_v<typename G::ValueType, T>);

    if (HasValue()) {
      return G();
    } else {
      return std::invoke(std::forward<F>(f), Error());
    }
  }

  template <typename F>
  constexpr auto OrElse(F&& f) const& {
    using G = std::remove_cvref_t<std::invoke_result_t<F, const E&>>;
    static_assert(detail::IsExpected<G>);
    static_assert(std::is_same_v<typename G::ValueType, T>);

    if (HasValue()) {
      return G();
    } else {
      return std::invoke(std::forward<F>(f), Error());
    }
  }

  template <typename F>
  constexpr auto OrElse(F&& f) && {
    using G = std::remove_cvref_t<std::invoke_result_t<F, E&&>>;
    static_assert(detail::IsExpected<G>);
    static_assert(std::is_same_v<typename G::ValueType, T>);

    if (HasValue()) {
      return G();
    } else {
      return std::invoke(std::forward<F>(f), std::move(Error()));
    }
  }

  template <typename F>
  constexpr auto OrElse(F&& f) const&& {
    using G = std::remove_cvref_t<std::invoke_result_t<F, const E&&>>;
    static_assert(detail::IsExpected<G>);
    static_assert(std::is_same_v<typename G::ValueType, T>);

    if (HasValue()) {
      return G();
    } else {
      return std::invoke(std::forward<F>(f), std::move(Error()));
    }
  }

  template <typename F>
    requires std::is_constructible_v<E, E&>
  constexpr auto Transform(F&& f) & {
    using U = std::remove_cv_t<std::invoke_result_t<F>>;
    using Result = Expected<U, E>;

    if (HasValue()) {
      if constexpr (std::is_void_v<U>) {
        std::invoke(std::forward<F>(f));
        return Result();
      } else {
        return Result(kInPlace, std::invoke(std::forward<F>(f)));
      }
    } else {
      return Result(kUnexpect, Error());
    }
  }

  template <typename F>
    requires std::is_constructible_v<E, const E&>
  constexpr auto Transform(F&& f) const& {
    using U = std::remove_cv_t<std::invoke_result_t<F>>;
    using Result = Expected<U, E>;

    if (HasValue()) {
      if constexpr (std::is_void_v<U>) {
        std::invoke(std::forward<F>(f));
        return Result();
      } else {
        return Result(kInPlace, std::invoke(std::forward<F>(f)));
      }
    } else {
      return Result(kUnexpect, Error());
    }
  }

  template <typename F>
    requires std::is_constructible_v<E, E>
  constexpr auto Transform(F&& f) && {
    using U = std::remove_cv_t<std::invoke_result_t<F>>;
    using Result = Expected<U, E>;

    if (HasValue()) {
      if constexpr (std::is_void_v<U>) {
        std::invoke(std::forward<F>(f));
        return Result();
      } else {
        return Result(kInPlace, std::invoke(std::forward<F>(f)));
      }
    } else {
      return Result(kUnexpect, std::move(Error()));
    }
  }

  template <typename F>
    requires std::is_constructible_v<E, E&>
  constexpr auto Transform(F&& f) const&& {
    using U = std::remove_cv_t<std::invoke_result_t<F>>;
    using Result = Expected<U, E>;

    if (HasValue()) {
      if constexpr (std::is_void_v<U>) {
        std::invoke(std::forward<F>(f));
        return Result();
      } else {
        return Result(kInPlace, std::invoke(std::forward<F>(f)));
      }
    } else {
      return Result(kUnexpect, std::move(Error()));
    }
  }

  template <typename F>
  constexpr auto TransformError(F&& f) & {
    using G = std::remove_cv_t<std::invoke_result_t<F, E&>>;
    using Result = Expected<T, G>;

    if (HasValue()) {
      return Result();
    } else {
      return Result(kUnexpect, std::invoke(std::forward<F>(f), Error()));
    }
  }

  template <typename F>
  constexpr auto TransformError(F&& f) const& {
    using G = std::remove_cv_t<std::invoke_result_t<F, const E&>>;
    using Result = Expected<T, G>;

    if (HasValue()) {
      return Result();
    } else {
      return Result(kUnexpect, std::invoke(std::forward<F>(f), Error()));
    }
  }

  template <typename F>
  constexpr auto TransformError(F&& f) && {
    using G = std::remove_cv_t<std::invoke_result_t<F, E&&>>;
    using Result = Expected<T, G>;

    if (HasValue()) {
      return Result();
    } else {
      return Result(kUnexpect,
                    std::invoke(std::forward<F>(f), std::move(Error())));
    }
  }

  template <typename F>
  constexpr auto TransformError(F&& f) const&& {
    using G = std::remove_cv_t<std::invoke_result_t<F, const E&&>>;
    using Result = Expected<T, G>;

    if (HasValue()) {
      return Result();
    } else {
      return Result(kUnexpect,
                    std::invoke(std::forward<F>(f), std::move(Error())));
    }
  }

  template <typename U, typename G>
    requires std::is_void_v<G>
  friend constexpr bool operator==(const Expected& x, const Expected<U, G>& y) {
    if (x.HasValue() != y.HasValue()) {
      return false;
    } else {
      return x.HasValue() || static_cast<bool>(x.Error() == y.Error());
    }
  }

  template <typename G>
  friend constexpr bool operator==(const Expected& x, const Unexpected<G>& e) {
    return !x.HasValue() && static_cast<bool>(x.Error() == e.Error());
  }

 private:
  constexpr E& Unex() & noexcept { return *reinterpret_cast<E*>(unex_raw_); }

  constexpr const E& Unex() const& noexcept {
    return *reinterpret_cast<const E*>(unex_raw_);
  }

  constexpr E&& Unex() && noexcept { return *reinterpret_cast<E*>(unex_raw_); }

  constexpr const E&& Unex() const&& noexcept {
    return *reinterpret_cast<const E*>(unex_raw_);
  }

  union {
    alignas(alignof(E)) std::byte unex_raw_[sizeof(E)];
  };
  bool has_val_;
};

}  // namespace termite

#endif
