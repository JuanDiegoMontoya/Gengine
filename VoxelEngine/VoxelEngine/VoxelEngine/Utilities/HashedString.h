#pragma once
#include <cstddef>
#include <cstdint>
#include <cctype>

// slim version of entt's hashed string

namespace detail
{
  template<typename>
  struct fnv1a_traits;


  template<>
  struct fnv1a_traits<std::uint32_t>
  {
    using type = std::uint32_t;
    static constexpr std::uint32_t offset = 2166136261;
    static constexpr std::uint32_t prime = 16777619;
  };


  template<>
  struct fnv1a_traits<std::uint64_t>
  {
    using type = std::uint64_t;
    static constexpr std::uint64_t offset = 14695981039346656037ull;
    static constexpr std::uint64_t prime = 1099511628211ull;
  };
}


/**
* @brief Zero overhead unique identifier.
*
* A hashed string is a compile-time tool that allows users to use
* human-readable identifers in the codebase while using their numeric
* counterparts at runtime.<br/>
* Because of that, a hashed string can also be used in constant expressions if
* required.
*
* @tparam Char Character type.
*/
template<typename Char>
class basic_hashed_string
{
  using traits_type = detail::fnv1a_traits<uint32_t>;

  struct const_wrapper
  {
    // non-explicit constructor on purpose
    constexpr const_wrapper(const Char* curr) noexcept : str{ curr } {}
    const Char* str;
  };

  // Fowler–Noll–Vo hash function v. 1a - the good
  [[nodiscard]] static constexpr uint32_t helper(const Char* curr) noexcept
  {
    auto value = traits_type::offset;

    while (*curr != 0)
    {
      value = (value ^ static_cast<traits_type::type>(std::tolower(*(curr++)))) * traits_type::prime;
    }

    return value;
  }

public:
  /*! @brief Character type. */
  using value_type = Char;
  /*! @brief Unsigned integer type. */
  using hash_type = uint32_t;

  /**
    * @brief Returns directly the numeric representation of a string.
    *
    * Forcing template resolution avoids implicit conversions. An
    * human-readable identifier can be anything but a plain, old bunch of
    * characters.<br/>
    * Example of use:
    * @code{.cpp}
    * const auto value = basic_hashed_string<char>::to_value("my.png");
    * @endcode
    *
    * @tparam N Number of characters of the identifier.
    * @param str Human-readable identifer.
    * @return The numeric representation of the string.
    */
  template<std::size_t N>
  [[nodiscard]] static constexpr hash_type value(const value_type(&str)[N]) noexcept
  {
    return helper(str);
  }

  /**
    * @brief Returns directly the numeric representation of a string.
    * @param wrapper Helps achieving the purpose by relying on overloading.
    * @return The numeric representation of the string.
    */
  [[nodiscard]] static hash_type value(const_wrapper wrapper) noexcept
  {
    return helper(wrapper.str);
  }

  /**
    * @brief Returns directly the numeric representation of a string view.
    * @param str Human-readable identifer.
    * @param size Length of the string to hash.
    * @return The numeric representation of the string.
    */
  [[nodiscard]] static hash_type value(const value_type* str, std::size_t size) noexcept
  {
    uint32_t partial{ traits_type::offset };
    while (size--) { partial = (partial ^ (str++)[0]) * traits_type::prime; }
    return partial;
  }

  /*! @brief Constructs an empty hashed string. */
  constexpr basic_hashed_string() noexcept
    : str{ nullptr }, hash{}
  {}

  /**
    * @brief Constructs a hashed string from an array of const characters.
    *
    * Forcing template resolution avoids implicit conversions. An
    * human-readable identifier can be anything but a plain, old bunch of
    * characters.<br/>
    * Example of use:
    * @code{.cpp}
    * basic_hashed_string<char> hs{"my.png"};
    * @endcode
    *
    * @tparam N Number of characters of the identifier.
    * @param curr Human-readable identifer.
    */
  template<std::size_t N>
  constexpr basic_hashed_string(const value_type(&curr)[N]) noexcept
    : str{ curr }, hash{ helper(curr) }
  {}

  /**
    * @brief Explicit constructor on purpose to avoid constructing a hashed
    * string directly from a `const value_type *`.
    * @param wrapper Helps achieving the purpose by relying on overloading.
    */
  explicit constexpr basic_hashed_string(const_wrapper wrapper) noexcept
    : str{ wrapper.str }, hash{ helper(wrapper.str) }
  {}

  /**
    * @brief Returns the human-readable representation of a hashed string.
    * @return The string used to initialize the instance.
    */
  [[nodiscard]] constexpr const value_type* data() const noexcept
  {
    return str;
  }

  /**
    * @brief Returns the numeric representation of a hashed string.
    * @return The numeric representation of the instance.
    */
  [[nodiscard]] constexpr hash_type value() const noexcept
  {
    return hash;
  }

  /*! @copydoc data */
  [[nodiscard]] constexpr operator const value_type* () const noexcept { return data(); }

  /**
    * @brief Returns the numeric representation of a hashed string.
    * @return The numeric representation of the instance.
    */
  [[nodiscard]] constexpr operator hash_type() const noexcept { return value(); }

  /**
    * @brief Compares two hashed strings.
    * @param other Hashed string with which to compare.
    * @return True if the two hashed strings are identical, false otherwise.
    */
  [[nodiscard]] constexpr bool operator==(const basic_hashed_string& other) const noexcept
  {
    return hash == other.hash;
  }

private:
  const value_type* str;
  hash_type hash;
};


/**
  * @brief Deduction guide.
  *
  * It allows to deduce the character type of the hashed string directly from a
  * human-readable identifer provided to the constructor.
  *
  * @tparam Char Character type.
  * @tparam N Number of characters of the identifier.
  * @param str Human-readable identifer.
  */
template<typename Char, std::size_t N>
basic_hashed_string(const Char(&str)[N]) noexcept
->basic_hashed_string<Char>;


/**
  * @brief Compares two hashed strings.
  * @tparam Char Character type.
  * @param lhs A valid hashed string.
  * @param rhs A valid hashed string.
  * @return True if the two hashed strings are identical, false otherwise.
  */
template<typename Char>
[[nodiscard]] constexpr bool operator!=(const basic_hashed_string<Char>& lhs, const basic_hashed_string<Char>& rhs) noexcept
{
  return !(lhs == rhs);
}


using hashed_string = basic_hashed_string<char>;
using hashed_wstring = basic_hashed_string<wchar_t>;




///**
// * @brief User defined literal for hashed strings.
// * @param str The literal without its suffix.
// * @return A properly initialized hashed string.
// */
//[[nodiscard]] constexpr hashed_string operator"" hs(const char* str, std::size_t) noexcept
//{
//  return hashed_string{ str };
//}
//
//
///**
// * @brief User defined literal for hashed wstrings.
// * @param str The literal without its suffix.
// * @return A properly initialized hashed wstring.
// */
//[[nodiscard]] constexpr hashed_wstring operator"" hws(const wchar_t* str, std::size_t) noexcept
//{
//  return hashed_wstring{ str };
//}