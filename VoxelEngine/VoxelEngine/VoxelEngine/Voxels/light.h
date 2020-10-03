#pragma once
#include <Graphics/math_utils.h>

// describes the lighting level at a point in space, NOT the properties of a light emitter
typedef class Light
{
public:
	Light() : raw_(0) {}
	Light(const Light& o) : raw_(o.raw_) {}
	Light(glm::u8vec4 L) { Set(L); }

	uint16_t& Raw() { return raw_; }

	glm::u8vec4 Get() const { return { GetR(), GetG(), GetB(), GetS() }; }
	uint8_t GetR() const { return raw_ >> 12; }
	uint8_t GetG() const { return (raw_ >> 8) & 0b1111; }
	uint8_t GetB() const { return (raw_ >> 4) & 0b1111; }
	uint8_t GetS() const { return raw_ & 0b1111; }

	void Set(glm::u8vec4 L)
	{
		ASSERT(glm::all(glm::lessThan(L, glm::u8vec4(16))));
		raw_ = (L.r << 12) | (L.g << 8) | (L.b << 4) | L.a;
	}
	void SetR(uint8_t r) { ASSERT(r < 16); raw_ = (raw_ & 0x0FFF) | (r << 12); }
	void SetG(uint8_t g) { ASSERT(g < 16); raw_ = (raw_ & 0xF0FF) | (g << 8); }
	void SetB(uint8_t b) { ASSERT(b < 16); raw_ = (raw_ & 0xFF0F) | (b << 4); }
	void SetS(uint8_t s) { ASSERT(s < 16); raw_ = (raw_ & 0xFFF0) | s; }

	bool operator==(const Light& rhs) const { return this->raw_ == rhs.raw_; }
	Light& operator=(const Light& rhs) { this->raw_ = rhs.raw_; return *this; }
private:
	// 4 bits each of: red, green, blue, and sunlight
	uint16_t raw_;
}Light, *LightPtr;