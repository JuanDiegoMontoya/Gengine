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
  
  template <class Archive>
  void serialize(Archive& ar)
  {
    ar(data_);
  }

private:
  // TODO: make this less vector<bool>
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
  for (int i = index; i < index + len; i++)
  {
    data_[i] = bitfield & 1;
    bitfield >>= 1;
  }
}

inline uint32_t BitArray::GetSequence(int index, int len) const
{
  unsigned bitfield = 0;
  for (int i = index; i < index + len; i++)
  {
    bitfield |= data_[i] << (i - index);
  }
  return bitfield;
}