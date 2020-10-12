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

  // returns a BitArray containing all elements for which predicate returned true
  template<typename Pred>
  BitArray FindAll(int groupSize, Pred predicate);
  
  std::vector<uint8_t> ByteRepresentation();

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

inline std::vector<uint8_t> BitArray::ByteRepresentation()
{
  std::vector<uint8_t> bytes;
  uint8_t curbyte = 0;
  int curbit = 0;
  for (auto bit : data_)
  {
    curbyte |= bit << curbit++;
    if (curbit == 8)
    {
      bytes.push_back(curbyte);
      curbit = 0;
      curbyte = 0;
    }
  }
  return bytes;
}

template<typename Pred>
inline BitArray BitArray::FindAll(int groupSize, Pred predicate)
{
  ASSERT(data_.size() % groupSize == 0); // no dangling bits allowed
  BitArray arr(data_.size());
  int a = 0;
  for (int i = 0; i < data_.size() / groupSize; i++)
  {
    if (auto bits = GetSequence(i * groupSize, groupSize); predicate(bits) == true)
    {
      arr.SetSequence(a++ * groupSize, groupSize, bits);
    }
  }
  arr.Resize(a * groupSize);
  return arr;
}
