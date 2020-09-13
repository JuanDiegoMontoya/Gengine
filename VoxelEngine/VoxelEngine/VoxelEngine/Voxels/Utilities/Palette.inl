#pragma once
#include <Utilities/Palette.h>
#include <shared_mutex>

template<typename T, unsigned _Size>
Palette<T, _Size>::Palette()
{
	data_.Resize(_Size * paletteEntryLength_);
	palette_.resize(1u << paletteEntryLength_);
	palette_[0].refcount = _Size;
}

template<typename T, unsigned _Size>
Palette<T, _Size>::~Palette()
{}

template<typename T, unsigned _Size>
Palette<T, _Size>::Palette(const Palette& other)
{
	*this = other;
}

template<typename T, unsigned _Size>
Palette<T, _Size>& Palette<T, _Size>::operator=(const Palette& other)
{
	this->data_ = other.data_;
	this->palette_ = other.palette_;
	this->paletteEntryLength_ = other.paletteEntryLength_;
	return *this;
}

template<typename T, unsigned _Size>
void Palette<T, _Size>::SetVal(int index, T type)
{
	//std::unique_lock w(mtx);

	unsigned paletteIndex = data_.GetSequence(index * paletteEntryLength_, paletteEntryLength_);
	auto& current = palette_[paletteIndex]; // compiler forces me to make this auto

	// remove reference to block that is already there
	current.refcount--;

	// check if block type is already in palette
	int replaceIndex = -1;
	for (int i = 0; i < palette_.size(); i++)
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

template<typename T, unsigned _Size>
T Palette<T, _Size>::GetVal(int index) const
{
	//std::shared_lock r(mtx);
	unsigned paletteIndex = data_.GetSequence(index * paletteEntryLength_, paletteEntryLength_);
	T ret = palette_[paletteIndex].type;
	return ret;
}

template<typename T, unsigned _Size>
unsigned Palette<T, _Size>::newPaletteEntry()
{
	while (1)
	{
		// find index of free palette entry
		for (int i = 0; i < palette_.size(); i++)
			if (palette_[i].refcount == 0) // empty or uninitialized entry
				return i;

		// grow palette if no free entry
		growPalette();
	}
}

template<typename T, unsigned _Size>
void Palette<T, _Size>::growPalette()
{
	// decode indices (index into palette_)
	std::vector<unsigned> indices;
	indices.resize(_Size);
	for (int i = 0; i < _Size; i++)
		indices[i] = data_.GetSequence(i * paletteEntryLength_, paletteEntryLength_);

	// double length of palette
	//paletteEntryLength_ <<= 1;
	paletteEntryLength_++;
	palette_.resize(1u << paletteEntryLength_);

	// increase length of bitset to accommodate extra bit
	data_.Resize(_Size * paletteEntryLength_);

	// encode previous indices with extended length
	for (int i = 0; i < indices.size(); i++)
		data_.SetSequence(i * paletteEntryLength_, paletteEntryLength_, indices[i]);
}

template<typename T, unsigned _Size>
inline void Palette<T, _Size>::fitPalette()
{
	// Remove old entries...
	for (int i = 0; i < palette_.size(); i++) {
		if (palette_[i].refcount == 0)
		{
			palette[i] = null;
			paletteCount -= 1;
		}
	}

	// Is the palette less than half of its closest power-of-two?
	if (paletteCount > powerOfTwo(paletteCount) / 2) {
		// NO: The palette cannot be shrunk!
		return;
	}

	// decode all indices
	int[] indices = new int[size];
	for (int i = 0; i < indices.length; i++) {
		indices[i] = data.get(i * indicesLength, indicesLength);
	}

	// Create new palette, halfing it in size
	indicesLength = indicesLength >> 1;
	PaletteEntry[] newPalette = new PaletteEntry[2 pow indicesLength];

	// We gotta compress the palette entries!
	int paletteCounter = 0;
	for (int pi = 0; pi < palette.length; pi++, paletteCounter++) {
		PaletteEntry entry = newPalette[paletteCounter] = palette[pi];

		// Re-encode the indices (find and replace; with limit)
		for (int di = 0, fc = 0; di < indices.length && fc < entry.refcount; di++) {
			if (pi == indices[di]) {
				indices[di] = paletteCounter;
				fc += 1;
			}
		}
	}

	// Allocate new BitBuffer
	data = new BitBuffer(size * indicesLength); // the length is in bits, not bytes!

	// Encode the indices
	for (int i = 0; i < indices.length; i++) {
		data.set(i * indicesLength, indicesLength, indices[i]);
	}
}
