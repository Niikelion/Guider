#include <guider/definitions.hpp>
#include <climits>

namespace Guider
{
	constexpr float feps = std::numeric_limits<float>::epsilon();


	Vec2& Vec2::operator += (const Vec2& t) noexcept
	{
		x += t.x;
		y += t.y;
		return *this;
	}

	Vec2 Vec2::operator + (const Vec2& t) const noexcept
	{
		return Vec2(*this) += t;
	}

	Vec2& Vec2::operator -= (const Vec2& t) noexcept
	{
		x -= t.x;
		y -= t.y;
		return *this;
	}

	Vec2 Vec2::operator - (const Vec2& t) const noexcept
	{
		return Vec2(*this) -= t;
	}

	Vec2::Vec2()
	{
		x = 0;
		y = 0;
	}

	Vec2::Vec2(float w, float h)
	{
		x = w;
		y = h;
	}


	bool Rect::operator == (const Rect& t) const noexcept
	{
		return
			feps >= std::abs(left - t.left) &&
			feps >= std::abs(width - t.width) &&
			feps >= std::abs(top - t.top) &&
			feps >= std::abs(height - t.height);
	}

	bool Rect::operator != (const Rect& t) const noexcept
	{
		return
			feps < std::abs(left - t.left) ||
			feps < std::abs(width - t.width) ||
			feps < std::abs(top - t.top) ||
			feps < std::abs(height - t.height);
	}

	Rect& Rect::operator += (const Vec2& offset)noexcept
	{
		left += offset.x;
		top += offset.y;
		return *this;
	}

	Rect& Rect::operator -= (const Vec2& offset) noexcept
	{
		left -= offset.x;
		top -= offset.y;
		return *this;
	}

	Rect Rect::operator + (const Vec2& offset) const noexcept
	{
		Rect tmp = *this;
		tmp += offset;
		return tmp;
	}

	Rect Rect::operator - (const Vec2& offset) const noexcept
	{
		Rect tmp = *this;
		tmp -= offset;
		return tmp;
	}

	bool Rect::intersects(const Rect& rect) const noexcept
	{
		return	(rect.left <= left + width && rect.left + rect.width >= left) &&
			(rect.top <= top + height && rect.top + rect.height >= top);
	}

	bool Rect::contains(const Vec2& pos) const noexcept
	{
		return left < pos.x&& top < pos.y&& left + width > pos.x&& top + height > pos.y;
	}

	Rect Rect::at(const Vec2& pos) const noexcept
	{
		return Rect(pos.x, pos.y, width, height);
	}

	Vec2 Rect::position() const noexcept
	{
		return Vec2(left, top);
	}

	Vec2 Rect::size() const noexcept
	{
		return Vec2(width, height);
	}

	Rect Rect::limit(const Rect& rect) const noexcept
	{
		float	l = std::max(left, rect.left),
			t = std::max(top, rect.top),
			r = std::min(left + width, rect.left + rect.width),
			b = std::min(top + height, rect.top + rect.height);
		Rect ret(l, t, r - l, b - t);
		return ret;
	}

	Rect::Rect()
	{
		left = 0;
		top = 0;
		width = 0;
		height = 0;
	}

	Rect::Rect(float l, float t, float w, float h) : left(l), top(t), width(w), height(h)
	{
	}


	bool Color::operator == (const Color& color) const noexcept
	{
		return value == color.value;
	}

	bool Color::operator != (const Color& color) const noexcept
	{
		return value != color.value;
	}

	uint32_t Color::hex() const noexcept
	{
		return r << 24 | g << 16 | b << 8 | a;
	}

	Color::Color() : r(0), g(0), b(0), a(255)
	{
	}

	Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}

	Color::Color(uint32_t v)
	{
		a = v & 0xFF;
		v >>= 8;
		b = v & 0xFF;
		v >>= 8;
		g = v & 0xFF;
		v >>= 8;
		r = v & 0xFF;
	}

	Color::Color(Name name) : r(0), g(0), b(0), a(255)
	{
		switch (name)
		{
		case Guider::Color::Name::Black:
		{
			value = Color(0, 0, 0).value;
			break;
		}
		case Guider::Color::Name::White:
		{
			value = Color(255, 255, 255).value;
			break;
		}
		case Guider::Color::Name::Red:
		{
			value = Color(255, 0, 0).value;
			break;
		}
		case Guider::Color::Name::Green:
		{
			value = Color(0, 255, 0).value;
			break;
		}
		case Guider::Color::Name::Blue:
		{
			value = Color(0, 0, 255).value;
			break;
		}
		}
	}

	Color::Color(const Color& t) : value(t.value)
	{
	}


	Rect Padding::calcContentArea(const Rect& bounds) const
	{
		Rect ret = Rect(bounds.left + left, bounds.top + top, bounds.width - left - right, bounds.height - top - bottom);

		if (ret.width < 0)
		{
			ret.width = 0;
			ret.left = (2 * bounds.left + left + bounds.width - right) / 2;
		}

		if (ret.height < 0)
		{
			ret.height = 0;
			ret.top = (2 * bounds.top + top + bounds.height - bottom) / 2;
		}

		return ret;
	}

	Padding::Padding() : left(0), right(0), top(0), bottom(0)
	{
	}

	Padding::Padding(float l, float r, float t, float b) : left(l), right(r), top(t), bottom(b)
	{
	}
}