#pragma once
#include <Utilities/BitArray.h>
#include <vector>
#include <cereal/types/vector.hpp>
#include <shared_mutex>

// fixed-size array optimized for space
template<typename T, unsigned _Size>
class Palette
{
public:
	Palette();
	~Palette();
	Palette(const Palette&);
	Palette& operator=(const Palette&);

	void SetVal(int index, T);
	T GetVal(int index) const;

private:
	friend class cereal::access;

	struct PaletteEntry
	{
		T type;
		int refcount = 0;

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
	unsigned paletteEntryLength_ = 1;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(data_, palette_, paletteEntryLength_);
	}
};


// thread-safe variation of the palette
template<typename T, unsigned _Size>
class ConcurrentPalette : public Palette<T, _Size>
{
public:

	ConcurrentPalette<T, _Size>& operator=(const ConcurrentPalette<T, _Size>& other)
	{
		std::lock_guard w(mtx);
		Palette<T, _Size>::operator=(other);
		return *this;
	}

	void SetVal(int index, T val)
	{
		std::lock_guard w(mtx);
		Palette<T, _Size>::SetVal(index, val);
	}

	T GetVal(int index) const
	{
		std::shared_lock r(mtx);
		return Palette<T, _Size>::GetVal(index);
	}

private:
	// writes are exclusive, reads are shared
	mutable std::shared_mutex mtx;
};

#include "Palette.inl"