#include "../../stdafx.h"
#include "utilities.h"

namespace Utils
{
	/**
	 * Converts an RGB color value to HSL. Conversion formula
	 * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
	 * Assumes r, g, and b are contained in the set [0, 255] and
	 * returns h, s, and l in the set [0, 1].
	 *
	 * @param   {number}  r       The red color value
	 * @param   {number}  g       The green color value
	 * @param   {number}  b       The blue color value
	 * @return  {Array}           The HSL representation
	 */
	glm::vec3 RGBtoHSL(glm::vec3 rgb)
	{
		rgb.r /= 255;
		rgb.g /= 255;
		rgb.b /= 255;
		float max = Utils::max3(rgb.r, rgb.g, rgb.b);
		float min = Utils::min3(rgb.r, rgb.g, rgb.b);

		float h;
		float s;
		float l = (max + min) / 2;

		if (max == min)
		{
			h = s = 0; // achromatic
		}
		else
		{
			float d = max - min;
			s = l > 0.5 ? d / (2 - max - min) : d / (max + min);

			if (max == rgb.r)
				h = (rgb.g - rgb.b) / d + (rgb.g < rgb.b ? 6 : 0);
			else if (max == rgb.g)
				h = (rgb.b - rgb.r) / d + 2;
			else if (max == rgb.b)
				h = (rgb.r - rgb.g) / d + 4;

			h /= 6;
		}

		return glm::vec3(h, s, l);
	}

	// helper for function below
	static float hue2rgb(float p, float q, float t)
	{
		if (t < 0) t += 1;
		if (t > 1) t -= 1;
		if (t < 1.f / 6.f) return p + (q - p) * 6.f * t;
		if (t < 1.f / 2.f) return q;
		if (t < 2.f / 3.f) return p + (q - p) * (2.f / 3.f - t) * 6.f;
		return p;
	}

	/**
	 * Converts an HSL color value to RGB. Conversion formula
	 * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
	 * Assumes h, s, and l are contained in the set [0, 1] and
	 * returns r, g, and b in the set [0, 255].
	 *
	 * @param   {number}  h       The hue
	 * @param   {number}  s       The saturation
	 * @param   {number}  l       The lightness
	 * @return  {Array}           The RGB representation
	 */
	glm::vec3 HSLtoRGB(glm::vec3 hsl)
	{
		glm::vec3 rgb;

		if (hsl.y == 0)
		{
			rgb.r = rgb.g = rgb.b = hsl.z; // achromatic
		}
		else
		{
			float q = hsl.z < 0.5 ? hsl.z * (1 + hsl.y) : hsl.z + hsl.y - hsl.z * hsl.y;
			float p = 2 * hsl.z - q;
			rgb.r = hue2rgb(p, q, hsl.x + 1.f / 3.f);
			rgb.g = hue2rgb(p, q, hsl.x);
			rgb.b = hue2rgb(p, q, hsl.x - 1.f / 3.f);
		}

		return glm::vec3(rgb.r * 255, rgb.g * 255, rgb.b * 255);
	}
}