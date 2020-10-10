#pragma once
#include <cereal/types/vector.hpp>

// specialized wrapper around std::vector<bool>
// or any other dynamic bitset container
// allows getting and setting of sequences
class BitArray
{
public:
  BitArray(size_t size = 0);
  void Resize(size_t newSize);
  void SetSequence(int index, int len, uint32_t bitfield);
  uint32_t GetSequence(int index, int len) const;
  void EraseSequence(int index, int len);
  size_t size() const { return data_.size(); }

  // erases all sequences of the given bitfield of length "len"
  size_t EraseAll(int len, uint32_t bitfield);
  
  template <class Archive>
  void serialize(Archive& ar)
  {
    ar(data_);
  }

private:
  std::vector<bool> data_;
};


inline BitArray::BitArray(size_t size)
{
  data_.resize(size, 0);
}

inline void BitArray::Resize(size_t newSize)
{
  data_.resize(newSize, 0);
}

inline void BitArray::SetSequence(int index, int len, uint32_t bitfield)
{
  ASSERT(index + len <= data_.size());
  for (int i = index; i < index + len; i++)
  {
    data_[i] = bitfield & 1;
    bitfield >>= 1;
  }
}

inline uint32_t BitArray::GetSequence(int index, int len) const
{
  ASSERT(index + len <= data_.size());
  unsigned bitfield = 0;
  for (int i = index; i < index + len; i++)
  {
    bitfield |= data_[i] << (i - index);
  }
  return bitfield;
}

inline void BitArray::EraseSequence(int index, int len)
{
  ASSERT(index + len <= data_.size());
  data_.erase(data_.begin() + index, data_.begin() + index + len);
}

// returns num sequences removed
inline size_t BitArray::EraseAll(int len, uint32_t bitfield)
{
  ASSERT(data_.size() % len == 0); // no dangling bits allowed
  size_t count = 0;
  for (int i = 0; i < data_.size() / len; i++)
  {
    if (GetSequence(i * len, len) == bitfield)
    {
      EraseSequence(i * len, len);
      i--;
      count++;
    }
  }
  return count;
}

