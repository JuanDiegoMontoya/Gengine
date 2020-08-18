/*HEADER_GOES_HERE*/
#ifndef BidirectionalMap_Guard
#define BidirectionalMap_Guard

#include <iostream>
#include <set>
#include <utility>
#include <cstddef>
#include <memory>
#include <cassert>
#include <vector>

namespace Internal
{
  template<typename T> struct PtrComparator
  {
    inline bool operator()(const T* mA, const T* mB) const noexcept { return *mA < *mB; }
  };
}

template<typename T1, typename T2> class bimap
{
public:
  using BMPair = std::pair<T1, T2>;
  using Storage = std::vector<std::unique_ptr<BMPair>>;
  template<typename T> using PtrSet = std::set<const T*, Internal::PtrComparator<T>>;

  using iterator = typename Storage::iterator;
  using const_iterator = typename Storage::const_iterator;
  using reverse_iterator = typename Storage::reverse_iterator;
  using const_reverse_iterator = typename Storage::const_reverse_iterator;

private:
  Storage storage;
  PtrSet<T1> set1;
  PtrSet<T2> set2;

  template<typename T> inline BMPair* getPairImpl(const T* mPtr) const noexcept
  {
    return const_cast<BMPair*>(reinterpret_cast<const BMPair*>(mPtr));
  }

  inline const char* getPairBasePtr(const T2* mItem) const noexcept
  {
    //static_assert(std::is_standard_layout<BMPair>::value, "BMPair must have standard layout");
    return reinterpret_cast<const char*>(mItem) - offsetof(BMPair, second);
  }

  inline BMPair* getPairPtr(const T1* mItem) const noexcept { return getPairImpl(mItem); }
  inline BMPair* getPairPtr(const T2* mItem) const noexcept { return getPairImpl(getPairBasePtr(mItem)); }

  inline T2& getImpl(const T1* mItem) noexcept { return getPairPtr(mItem)->second; }
  inline T1& getImpl(const T2* mItem) noexcept { return getPairPtr(mItem)->first; }

  inline const T2& getImpl(const T1* mItem) const noexcept { return getPairPtr(mItem)->second; }
  inline const T1& getImpl(const T2* mItem) const noexcept { return getPairPtr(mItem)->first; }

public:
  template<typename TA1, typename TA2> inline void emplace(TA1&& mArg1, TA2&& mArg2)
  {
    //assert(!this->has(mArg1) && !this->has(mArg2));

    auto pair(std::make_unique<std::pair<T1, T2>>(std::forward<TA1>(mArg1), std::forward<TA2>(mArg2)));
    set1.emplace(&(pair->first));
    set2.emplace(&(pair->second));
    storage.emplace_back(std::move(pair));
  }

  //template<typename TA1, typename TA2>
  //inline void remove(
  //  const std::_Vector_iterator<std::_Vector_val<std::_Simple_types<std::unique_ptr<std::pair<TA1 const, TA2 const>, std::default_delete<std::pair<TA1 const, TA2 const>>>>>>
  //  iterator)
  //{
  //  set1.erase(&iterator->get()->first);
  //  set2.erase(&iterator->get()->second);
  //  storage.erase(iterator);
  //}

  inline void insert(const BMPair& mPair)
  {
    this->emplace(mPair.first, mPair.second);
  }

  //template<typename T> inline void erase(const T& mKey)
  //{
  //  assert(this->has(mKey));

  //  const auto& pairPtr(getPairPtr(&mKey));

  //  set1.erase(&(pairPtr->first));
  //  set2.erase(&(pairPtr->second));

  //  ssvu::eraseRemoveIf(storage, [&pairPtr](const ssvu::Uptr<std::pair<T1, T2>>& mI) { return mI.get() == pairPtr; });

  //  assert(!this->has(mKey));
  //}

  inline const T2& at(const T1& mKey) const noexcept
  {
    const auto& itr(set1.find(&mKey));
    if (itr == std::end(set1)) throw std::out_of_range{ "mKey was not found in set1" };
    return getImpl(*itr);
  }
  inline const T1& at(const T2& mKey) const noexcept
  {
    const auto& itr(set2.find(&mKey));
    if (itr == std::end(set2)) throw std::out_of_range{ "mKey was not found in set2" };
    return getImpl(*itr);
  }

  inline T2& operator[](const T1& mKey) noexcept { assert(this->has(mKey)); return getImpl(*set1.find(&mKey)); }
  inline T1& operator[](const T2& mKey) noexcept { assert(this->has(mKey)); return getImpl(*set2.find(&mKey)); }

  inline const T2& operator[](const T1& mKey) const noexcept { assert(this->has(mKey)); return getImpl(*set1.find(&mKey)); }
  inline const T1& operator[](const T2& mKey) const noexcept { assert(this->has(mKey)); return getImpl(*set2.find(&mKey)); }

  inline void clear() noexcept { storage.clear(); set1.clear(); set2.clear(); }

  inline bool empty() const noexcept { return storage.empty(); }
  inline auto size() const noexcept -> decltype(storage.size()) { return storage.size(); }

  inline auto count(const T1& mKey) const noexcept -> decltype(set1.count(&mKey)) { return set1.count(&mKey); }
  inline auto count(const T2& mKey) const noexcept -> decltype(set2.count(&mKey)) { return set2.count(&mKey); }

  inline auto find(const T1& mKey) const noexcept -> decltype(set1.find(&mKey)) { return set1.find(&mKey); }
  inline auto find(const T2& mKey) const noexcept -> decltype(set2.find(&mKey)) { return set2.find(&mKey); }

  inline bool has(const T1& mKey) const noexcept { return this->find(mKey) != std::end(set1); }
  inline bool has(const T2& mKey) const noexcept { return this->find(mKey) != std::end(set2); }

  inline auto begin()     noexcept        -> decltype(storage.begin()) { return storage.begin(); }
  inline auto end()       noexcept        -> decltype(storage.end()) { return storage.end(); }
  inline auto begin()     const noexcept  -> decltype(storage.begin()) { return storage.begin(); }
  inline auto end()       const noexcept  -> decltype(storage.end()) { return storage.end(); }
  inline auto cbegin()    const noexcept  -> decltype(storage.cbegin()) { return storage.cbegin(); }
  inline auto cend()      const noexcept  -> decltype(storage.cend()) { return storage.cend(); }
  inline auto rbegin()    noexcept        -> decltype(storage.rbegin()) { return storage.rbegin(); }
  inline auto rend()      noexcept        -> decltype(storage.rend()) { return storage.rend(); }
  inline auto crbegin()   const noexcept  -> decltype(storage.crbegin()) { return storage.crbegin(); }
  inline auto crend()     const noexcept  -> decltype(storage.crend()) { return storage.crend(); }
};

#endif // !BidirectionalMap_Guard