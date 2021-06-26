#pragma once
#include <CoreEngine/GAssert.h>

struct SerializableBitArray
{
  uint32_t numBits{};
  std::vector<uint8_t> bytes{};
};

// specialized wrapper around std::vector<bool>
// or any other dynamic bitset container
// allows getting and setting of sequences
class BitArray
{
public:
  BitArray(size_t size = 0);
  BitArray(const SerializableBitArray& data);
  void Resize(size_t newSize);
  void SetSequence(int index, int len, uint32_t bitfield);
  uint32_t GetSequence(int index, int len) const;
  void EraseSequence(int index, int len);
  size_t size() const { return data_.size(); }
  bool operator==(const BitArray&) const = default;

  // erases all sequences of the given bitfield of length "len"
  size_t EraseAll(int len, uint32_t bitfield);

  // returns a BitArray containing all elements for which predicate returned true
  template<typename Pred>
  BitArray FindAll(int groupSize, Pred predicate);
  
  SerializableBitArray ByteRepresentation() const;

private:
  std::vector<bool> data_;
};


inline BitArray::BitArray(size_t size)
{
  data_.resize(size, 0);
}

inline BitArray::BitArray(const SerializableBitArray& data)
{
  data_.reserve(data.numBits);
  uint32_t i = 0;
  for (auto byte : data.bytes)
  {
    for (uint8_t curBit = 0; curBit < 8; curBit++, i++)
    {
      // branch could be optimized
      if (i >= data.numBits)
        break;
      data_.push_back(byte & (1 << curBit));
    }
  }
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

inline SerializableBitArray BitArray::ByteRepresentation() const
{
  SerializableBitArray arr;
  arr.numBits = data_.size();

  uint8_t curbyte = 0;
  int curbit = 0;
  for (const auto bit : data_)
  {
    curbyte |= bit << curbit++;
    if (curbit == 8)
    {
      arr.bytes.push_back(curbyte);
      curbit = 0;
      curbyte = 0;
    }
  }
  if (curbit != 0)
    arr.bytes.push_back(curbyte);
  
  return arr;
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
