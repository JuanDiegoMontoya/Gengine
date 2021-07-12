#pragma once
#include <utility/BitArray.h>
#include <vector>
#include <cereal/types/vector.hpp>
#include <shared_mutex>

namespace Voxels
{
  template<typename T>
  struct CompressedMaterialInfo;
}

// fixed-size array optimized for space
template<typename T, size_t Size>
class Palette
{
public:
  Palette();
  ~Palette();
  Palette(const Palette&);
  Palette& operator=(const Palette&);

  void SetVal(size_t index, T);
  T GetVal(size_t index) const;

  const BitArray& GetData() const { return data_; }

private:
  friend class cereal::access;
  friend struct Voxels::CompressedMaterialInfo<T>;

  struct PaletteEntry
  {
    T type{};
    int refcount = 0;

    PaletteEntry() = default;
    PaletteEntry(const T& ty) : type(ty) {}
    PaletteEntry(const T& ty, int refc) : type(ty), refcount(refc) {}
    bool operator==(const PaletteEntry& other) { return type == other.type; }

    template <class Archive>
    void serialize(Archive& ar)
    {
      ar(type, refcount);
    }
  };

  unsigned newPaletteEntry();
  void growPalette();
  void fitPalette();

  BitArray data_;
  std::vector<PaletteEntry> palette_;
  size_t paletteEntryLength_ = 1;

  template <class Archive>
  void serialize(Archive& ar)
  {
    ar(data_, palette_, paletteEntryLength_);
  }
};


// thread-safe variation of the palette
template<typename T, size_t Size>
class ConcurrentPalette : public Palette<T, Size>
{
public:

  ConcurrentPalette() {};

  ConcurrentPalette(const ConcurrentPalette& other) { *this = other; }

  ~ConcurrentPalette() {}

  ConcurrentPalette<T, Size>& operator=(const ConcurrentPalette<T, Size>& other)
  {
    std::lock_guard w(mtx);
    Palette<T, Size>::operator=(other);
    return *this;
  }

  void SetVal(size_t index, T val)
  {
    std::lock_guard w(mtx);
    Palette<T, Size>::SetVal(index, val);
  }

  T GetVal(size_t index) const
  {
    std::shared_lock r(mtx);
    return Palette<T, Size>::GetVal(index);
  }

private:
  // writes are exclusive, reads are shared
  mutable std::shared_mutex mtx;
};

#include "Palette.inl"