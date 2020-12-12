#pragma once
#include <cinttypes>

namespace Utils
{

  template <typename enumtype, typename storagetype = uint32_t>
  class Flags
  {
  public:
    typedef storagetype InternalType;

    inline Flags();
    inline Flags(enumtype e);
    inline Flags(const Flags<enumtype, storagetype>& f);
    inline explicit Flags(storagetype b);

    inline bool isSet(enumtype e) const;
    inline Flags<enumtype, storagetype>& set(enumtype e);
    inline bool operator==(enumtype e) const;
    inline bool operator==(const Flags<enumtype, storagetype>& f) const;
    inline bool operator==(bool b) const;
    inline bool operator!=(enumtype e) const;
    inline bool operator!=(const Flags<enumtype, storagetype>& f) const;

    inline Flags<enumtype, storagetype>& operator=(const Flags<enumtype, storagetype>& f);
    inline Flags<enumtype, storagetype>& operator=(enumtype e);

    inline Flags<enumtype, storagetype>& operator|=(enumtype e);
    inline Flags<enumtype, storagetype>& operator|=(const Flags<enumtype, storagetype>& f);
    inline Flags<enumtype, storagetype> operator|(enumtype e) const;
    inline Flags<enumtype, storagetype> operator|(const Flags<enumtype, storagetype>& f) const;

    inline Flags<enumtype, storagetype>& operator&=(enumtype e);
    inline Flags<enumtype, storagetype>& operator&=(const Flags<enumtype, storagetype>& f);
    inline Flags<enumtype, storagetype> operator&(enumtype e) const;
    inline Flags<enumtype, storagetype> operator&(const Flags<enumtype, storagetype>& f) const;

    inline Flags<enumtype, storagetype>& operator^=(enumtype e);
    inline Flags<enumtype, storagetype>& operator^=(const Flags<enumtype, storagetype>& f);
    inline Flags<enumtype, storagetype> operator^(enumtype e) const;
    inline Flags<enumtype, storagetype> operator^(const Flags<enumtype, storagetype>& f) const;

    inline Flags<enumtype, storagetype> operator~(void) const;

    inline operator bool(void) const;
    inline operator uint8_t(void) const;
    inline operator uint16_t(void) const;
    inline operator uint32_t(void) const;

    inline void clear(enumtype e);

  public:
    friend inline Flags<enumtype, storagetype> operator&(enumtype a, Flags<enumtype, storagetype>& b)
    {
      Flags<enumtype, storagetype> out;
      out.mBits = a & b.mBits;
      return out;
    }

  private:
    storagetype mBits;
  };

#define _FLAGS_OPERATORS(enumtype, storagetype)                          \
  inline Flags<enumtype, storagetype> operator|(enumtype a, enumtype b)  \
  {                                                                        \
    Flags<enumtype, storagetype> r(a);                                   \
    r |= b;                                                                \
    return r;                                                              \
  }                                                                        \
  inline Flags<enumtype, storagetype> operator&(enumtype a, enumtype b)  \
  {                                                                        \
    Flags<enumtype, storagetype> r(a);                                   \
    r &= b;                                                                \
    return r;                                                              \
  }                                                                        \
  inline Flags<enumtype, storagetype> operator~(enumtype a)              \
  {                                                                        \
    return ~Flags<enumtype, storagetype>(a);                             \
  }

#define _FLAGS_TYPEDEF(x, y)       \
  typedef Flags<x::Enum, y> x##s;  \
  _FLAGS_OPERATORS(x::Enum, y)

  template <typename enumtype, typename storagetype>
  inline Flags<enumtype, storagetype>::Flags(void)
  {
    mBits = 0;
  }

  template <typename enumtype, typename storagetype>
  inline Flags<enumtype, storagetype>::Flags(enumtype e)
  {
    mBits = static_cast<storagetype>(e);
  }

  template <typename enumtype, typename storagetype>
  inline Flags<enumtype, storagetype>::Flags(const Flags<enumtype, storagetype>& f)
  {
    mBits = f.mBits;
  }

  template <typename enumtype, typename storagetype>
  inline Flags<enumtype, storagetype>::Flags(storagetype b)
  {
    mBits = b;
  }

  template <typename enumtype, typename storagetype>
  inline bool Flags<enumtype, storagetype>::isSet(enumtype e) const
  {
    return (mBits & static_cast<storagetype>(e)) == static_cast<storagetype>(e);
  }

  template <typename enumtype, typename storagetype>
  inline Flags<enumtype, storagetype>& Flags<enumtype, storagetype>::set(enumtype e)
  {
    mBits = static_cast<storagetype>(e);
    return *this;
  }

  template <typename enumtype, typename storagetype>
  inline bool Flags<enumtype, storagetype>::operator==(enumtype e) const
  {
    return mBits == static_cast<storagetype>(e);
  }

  template <typename enumtype, typename storagetype>
  inline bool Flags<enumtype, storagetype>::operator==(const Flags<enumtype, storagetype>& f) const
  {
    return mBits == f.mBits;
  }

  template <typename enumtype, typename storagetype>
  inline bool Flags<enumtype, storagetype>::operator==(bool b) const
  {
    return bool(*this) == b;
  }

  template <typename enumtype, typename storagetype>
  inline bool Flags<enumtype, storagetype>::operator!=(enumtype e) const
  {
    return mBits != static_cast<storagetype>(e);
  }

  template <typename enumtype, typename storagetype>
  inline bool Flags<enumtype, storagetype>::operator!=(const Flags<enumtype, storagetype>& f) const
  {
    return mBits != f.mBits;
  }

  template <typename enumtype, typename storagetype>
  inline Flags<enumtype, storagetype>& Flags<enumtype, storagetype>::operator=(enumtype e)
  {
    mBits = static_cast<storagetype>(e);
    return *this;
  }

  template <typename enumtype, typename storagetype>
  inline Flags<enumtype, storagetype>& Flags<enumtype, storagetype>::operator=(const Flags<enumtype, storagetype>& f)
  {
    mBits = f.mBits;
    return *this;
  }

  template <typename enumtype, typename storagetype>
  inline Flags<enumtype, storagetype>& Flags<enumtype, storagetype>::operator|=(enumtype e)
  {
    mBits |= static_cast<storagetype>(e);
    return *this;
  }

  template <typename enumtype, typename storagetype>
  inline Flags<enumtype, storagetype>& Flags<enumtype, storagetype>::
    operator|=(const Flags<enumtype, storagetype>& f)
  {
    mBits |= f.mBits;
    return *this;
  }

  template <typename enumtype, typename storagetype>
  inline Flags<enumtype, storagetype> Flags<enumtype, storagetype>::operator|(enumtype e) const
  {
    Flags<enumtype, storagetype> out(*this);
    out |= e;
    return out;
  }

  template <typename enumtype, typename storagetype>
  inline Flags<enumtype, storagetype> Flags<enumtype, storagetype>::
    operator|(const Flags<enumtype, storagetype>& f) const
  {
    Flags<enumtype, storagetype> out(*this);
    out |= f;
    return out;
  }

  template <typename enumtype, typename storagetype>
  inline Flags<enumtype, storagetype>& Flags<enumtype, storagetype>::operator&=(enumtype e)
  {
    mBits &= static_cast<storagetype>(e);
    return *this;
  }

  template <typename enumtype, typename storagetype>
  inline Flags<enumtype, storagetype>& Flags<enumtype, storagetype>::
    operator&=(const Flags<enumtype, storagetype>& f)
  {
    mBits &= f.mBits;
    return *this;
  }

  template <typename enumtype, typename storagetype>
  inline Flags<enumtype, storagetype> Flags<enumtype, storagetype>::operator&(enumtype e) const
  {
    Flags<enumtype, storagetype> out = *this;
    out.mBits &= static_cast<storagetype>(e);
    return out;
  }

  template <typename enumtype, typename storagetype>
  inline Flags<enumtype, storagetype> Flags<enumtype, storagetype>::
    operator&(const Flags<enumtype, storagetype>& f) const
  {
    Flags<enumtype, storagetype> out = *this;
    out.mBits &= f.mBits;
    return out;
  }

  template <typename enumtype, typename storagetype>
  inline Flags<enumtype, storagetype>& Flags<enumtype, storagetype>::operator^=(enumtype e)
  {
    mBits ^= static_cast<storagetype>(e);
    return *this;
  }

  template <typename enumtype, typename storagetype>
  inline Flags<enumtype, storagetype>& Flags<enumtype, storagetype>::
    operator^=(const Flags<enumtype, storagetype>& f)
  {
    mBits ^= f.mBits;
    return *this;
  }

  template <typename enumtype, typename storagetype>
  inline Flags<enumtype, storagetype> Flags<enumtype, storagetype>::operator^(enumtype e) const
  {
    Flags<enumtype, storagetype> out = *this;
    out.mBits ^= static_cast<storagetype>(e);
    return out;
  }

  template <typename enumtype, typename storagetype>
  inline Flags<enumtype, storagetype> Flags<enumtype, storagetype>::
    operator^(const Flags<enumtype, storagetype>& f) const
  {
    Flags<enumtype, storagetype> out = *this;
    out.mBits ^= f.mBits;
    return out;
  }

  template <typename enumtype, typename storagetype>
  inline Flags<enumtype, storagetype> Flags<enumtype, storagetype>::operator~(void) const
  {
    Flags<enumtype, storagetype> out;
    out.mBits = storagetype(~mBits);
    return out;
  }

  template <typename enumtype, typename storagetype>
  inline Flags<enumtype, storagetype>::operator bool(void) const
  {
    return mBits ? true : false;
  }

  template <typename enumtype, typename storagetype>
  inline Flags<enumtype, storagetype>::operator uint8_t(void) const
  {
    return static_cast<uint8_t>(mBits);
  }

  template <typename enumtype, typename storagetype>
  inline Flags<enumtype, storagetype>::operator uint16_t(void) const
  {
    return static_cast<uint16_t>(mBits);
  }

  template <typename enumtype, typename storagetype>
  inline Flags<enumtype, storagetype>::operator uint32_t(void) const
  {
    return static_cast<uint32_t>(mBits);
  }

  template <typename enumtype, typename storagetype>
  inline void Flags<enumtype, storagetype>::clear(enumtype e)
  {
    mBits &= ~static_cast<storagetype>(e);
  }
}