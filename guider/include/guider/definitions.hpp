#pragma once

#include <unordered_map>
#include <unordered_set>
#include <cstdint>
#include <vector>
#include <string>
#include <list>

namespace Guider
{
	using String = std::string;
    template<
        typename T,
        typename Allocator = std::allocator<T>
    > using Vector = std::vector<T, Allocator>;

    template<
        typename T,
        typename Allocator = std::allocator<T>
    > using List = std::list<T, Allocator>;

    template<
        typename T1,
        typename T2
    > using Pair = std::pair<T1, T2>;

    template<
        typename Key,
        typename T,
        typename Hash = std::hash<Key>,
        typename KeyEqual = std::equal_to<Key>,
        typename Allocator = std::allocator< std::pair<const Key, T> >
    > using HashMap = std::unordered_map<Key, T, Hash, KeyEqual, Allocator>;

    template<
        typename T,
        typename Hash = std::hash<T>,
        typename Equal = std::equal_to<T>,
        typename Allocator = std::allocator<T>
    > using HashSet = std::unordered_set<T, Hash, Equal, Allocator>;

	/// @brief Basic class representing 2d point/vector.
	class Vec2
	{
	public:
		float x, y;

		Vec2& operator = (const Vec2&) = default;
		Vec2& operator = (Vec2&&) noexcept = default;

		Vec2& operator += (const Vec2& t) noexcept;
		Vec2 operator + (const Vec2& t) const noexcept;

		Vec2& operator -= (const Vec2& t) noexcept;
		Vec2 operator - (const Vec2& t) const noexcept;

		/// @brief Default constructor, initializes x and y to 0.
		Vec2();
		/// @brief Initializes x and y with provided values.
		/// @param w x coordinate.
		/// @param h y coordinate.
		Vec2(float w, float h);
		Vec2(const Vec2&) = default;
		Vec2(Vec2&&) noexcept = default;
	};
	/// @brief Basic class representing rectangle.
	class Rect
	{
	public:
		float left, top, width, height;

		Rect& operator = (const Rect&) = default;
		Rect& operator = (Rect&&) noexcept = default;

		bool operator == (const Rect&) const noexcept;
		bool operator != (const Rect&) const noexcept;

		Rect& operator += (const Vec2& offset) noexcept;
		Rect& operator -= (const Vec2& offset) noexcept;

		Rect operator + (const Vec2& offset) const noexcept;
		Rect operator - (const Vec2& offset) const noexcept;

		/// @brief Checks if 2 rects intersects.
		bool intersects(const Rect& rect) const noexcept;
		/// @brief Checks if rect cointains  point.
		bool contains(const Vec2& point) const noexcept;

		/// @brief Returns copy of rect with top left corner at pos.
		Rect at(const Vec2& pos) const noexcept;
		/// @brief Returns top left corner as point. 
		Vec2 position() const noexcept;
		/// @brief Returns size of the rect as vector.
		Vec2 size() const noexcept;
		/// @brief Returns rect cropped by other rect.
		Rect limit(const Rect& rect) const noexcept;

		Rect();
		Rect(float l, float t, float w, float h);
		Rect(const Rect&) = default;
		Rect(Rect&&) noexcept = default;
	};
	/// @brief Basic color class.
	class Color
	{
	public:
		enum Name
		{
			Black,
			White,

			Red,
			Green,
			Blue
		};

		union
		{
			uint32_t value;
			struct
			{
				uint8_t r, g, b, a;
			};
		};

		bool operator == (const Color& color) const noexcept;
		bool operator != (const Color& color) const noexcept;

		/// @brief Returns color as number in standard format.
		/// 
		/// Result is in 0xrrggbbaa format for easier printing and parsing.
		uint32_t hex() const noexcept;

		/// @brief Creates solid black color.
		Color();
		Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xFF);
		/// @brief Constructs color from hex reprezentation.
		Color(uint32_t v);
		Color(Name name);
		Color(const Color& t);
	};
	/// @brief Padding.
	class Padding
	{
	public:
		float left, right, top, bottom;

		/// @brief Returns adjusted content rect inside provided rect.
		Rect calcContentArea(const Rect& bounds) const;

		Padding();
		Padding(float l, float r, float t, float b);
		Padding(const Padding&) = default;
	};
	/// @brief Defines float directions.
	enum class Gravity
	{
		Start = 0,
		Left = Start,
		Top = Start,

		Middle = 1,
		Center = Middle,

		End = 2,
		Right = End,
		Bottom = End
	};
	/// @brief Defines orientation.
	enum class Orientation
	{
		Horizontal,
		Vertical
	};
}