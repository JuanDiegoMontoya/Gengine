#pragma once
#include <Utilities/Palette.h>
#include <shared_mutex>

#pragma warning(push)
#pragma warning(disable : 4334 4267 26451) // 32-bit shift, 8->4 byte int conversion

template<typename T, size_t Size>
Palette<T, Size>::Palette()
{
  data_.Resize(Size * paletteEntryLength_);
  palette_.resize(1 << paletteEntryLength_);
  palette_[0].refcount = Size;
}

template<typename T, size_t Size>
Palette<T, Size>::~Palette()
{}

template<typename T, size_t Size>
Palette<T, Size>::Palette(const Palette& other)
{
  *this = other;
}

template<typename T, size_t Size>
Palette<T, Size>& Palette<T, Size>::operator=(const Palette& other)
{
  this->data_ = other.data_;
  this->palette_ = other.palette_;
  this->paletteEntryLength_ = other.paletteEntryLength_;
  return *this;
}

template<typename T, size_t Size>
void Palette<T, Size>::SetVal(size_t index, T type)
{
  //std::unique_lock w(mtx);

  unsigned paletteIndex = data_.GetSequence(index * paletteEntryLength_, paletteEntryLength_);
  auto& current = palette_[paletteIndex]; // compiler forces me to make this auto

  // remove reference to block that is already there
  current.refcount--;

  // check if block type is already in palette
  int replaceIndex = -1;
  for (size_t i = 0; i < palette_.size(); i++)
  {
    if (palette_[i].type == type)
    {
      replaceIndex = i;
      // use existing palette entry
      data_.SetSequence(index * paletteEntryLength_, paletteEntryLength_, unsigned(replaceIndex));
      palette_[replaceIndex].refcount++;
      return;
    }
  }

  // check if palette entry of block we just removed is empty
  if (current.refcount == 0)
  {
    current.type = type;
    current.refcount = 1;
    return;
  }

  // we need a new palette entry, dawg
  unsigned newEntry = newPaletteEntry();
  palette_[newEntry] = { type, 1 };
  data_.SetSequence(index * paletteEntryLength_, paletteEntryLength_, newEntry);
}

template<typename T, size_t Size>
T Palette<T, Size>::GetVal(size_t index) const
{
  //std::shared_lock r(mtx);
  unsigned paletteIndex = data_.GetSequence(index * paletteEntryLength_, paletteEntryLength_);
  T ret = palette_[paletteIndex].type;
  return ret;
}

template<typename T, size_t Size>
unsigned Palette<T, Size>::newPaletteEntry()
{
  while (1)
  {
    // find index of free palette entry
    for (size_t i = 0; i < palette_.size(); i++)
      if (palette_[i].refcount == 0) // empty or uninitialized entry
        return i;

    // grow palette if no free entry
    growPalette();
  }
}

template<typename T, size_t Size>
void Palette<T, Size>::growPalette()
{
  // decode indices (index into palette_)
  std::vector<unsigned> indices;
  indices.resize(Size);
  for (size_t i = 0; i < Size; i++)
    indices[i] = data_.GetSequence(i * paletteEntryLength_, paletteEntryLength_);

  // double length of palette
  //paletteEntryLength_ <<= 1;
  paletteEntryLength_++;
  palette_.resize(1 << paletteEntryLength_);
  
  // increase length of bitset to accommodate extra bit
  data_.Resize(Size * paletteEntryLength_);

  // encode previous indices with extended length
  for (size_t i = 0; i < indices.size(); i++)
    data_.SetSequence(i * paletteEntryLength_, paletteEntryLength_, indices[i]);
}


template<typename T, size_t Size>
inline void Palette<T, Size>::fitPalette()
{
  auto palettePrevSize = palette_.size();
  // Remove old entries
  for (int i = 0; i < palette_.size(); i++)
  {
    if (palette_[i].refcount == 0)
    {
      palette_[i] = {}; // zero-initialize contents of palette entry
    }
  }
  
  // TODO: this logic
  auto powerOfTwo = [] (int num)
  {
    int inc = 1, count = 0;
    for (; inc < num; count++)
      inc <<= 1;
    return count;
  };
  // Is the palette less than half of its closest power-of-two?
  if (palette_.size() > powerOfTwo(palettePrevSize) / 2)
  {
    // NO: The palette cannot be shrunk!
    return;
  }

  // decode all indices
  std::vector<int> indices(Size);
  for (int i = 0; i < indices.size(); i++)
  {
    indices[i] = data_.GetSequence(i * paletteEntryLength_, paletteEntryLength_);
  }

  // Create new palette, halfing it in size
  paletteEntryLength_ >>= 1;
  std::vector<PaletteEntry> newPalette(1 << paletteEntryLength_);

  // We gotta compress the palette entries!
  int paletteCounter = 0;
  for (int pi = 0; pi < palette_.size(); pi++, paletteCounter++)
  {
    PaletteEntry entry = newPalette[paletteCounter] = palette_[pi];

    // Re-encode the indices (find and replace; with limit)
    for (int di = 0, fc = 0; di < indices.size() && fc < entry.refcount; di++)
    {
      if (pi == indices[di])
      {
        indices[di] = paletteCounter;
        fc += 1;
      }
    }
  }

  // Allocate new BitBuffer
  data_ = BitArray(Size * paletteEntryLength_); // the length is in bits, not bytes!

  // Encode the indices
  for (int i = 0; i < indices.size(); i++)
  {
    data_.SetSequence(i * paletteEntryLength_, paletteEntryLength_, indices[i]);
  }
}

#pragma warning(pop)