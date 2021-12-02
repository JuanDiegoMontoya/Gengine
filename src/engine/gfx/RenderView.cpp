#include "../PCH.h"
#include "RenderView.h"
#include <tuple>

namespace GFX
{
  namespace
  {
    template <class T>
    inline void hash_combine(std::size_t& seed, T const& v)
    {
      seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    // Recursive template code derived from Matthieu M.
    template <class Tuple, size_t Index = std::tuple_size<Tuple>::value - 1>
    struct HashValueImpl
    {
      static void apply(size_t& seed, Tuple const& tuple)
      {
        HashValueImpl<Tuple, Index - 1>::apply(seed, tuple);
        hash_combine(seed, std::get<Index>(tuple));
      }
    };

    template <class Tuple>
    struct HashValueImpl<Tuple, 0>
    {
      static void apply(size_t& seed, Tuple const& tuple)
      {
        hash_combine(seed, std::get<0>(tuple));
      }
    };

    template <typename ... TT>
    struct hash<std::tuple<TT...>>
    {
      size_t operator()(std::tuple<TT...> const& tt) const
      {
        size_t seed = 0;
        HashValueImpl<std::tuple<TT...> >::apply(seed, tt);
        return seed;
      }
    };
  }


  std::size_t hash<RenderView>::operator()(const RenderView& a) const noexcept
  {
    // TODO: hash RenderInfo struct
    auto tup = std::make_tuple(static_cast<void*>(a.camera), static_cast<uint32_t>(a.mask), a.renderInfo.offset.x, a.renderInfo.offset.y, a.renderInfo.size.width, a.renderInfo.size.height);
    hash<decltype(tup)> hasher;
    return hasher(tup);
  }

  bool RenderView::operator==(const RenderView& b) const
  {
    // TODO: compare renderInfo
    return camera == b.camera && mask == b.mask;
  }
}