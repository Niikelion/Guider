#include <guider/styles.hpp>
#include <sstream>

namespace Guider
{
	namespace Styles
	{
		bool isDigitInBase(char c, unsigned base)
		{
			return base <= 10 ? (c >= '0' && c < '0' + static_cast<int>(base)) : ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'A' + static_cast<int>(base) - 10) || (c >= 'a' && c < 'a' + static_cast<int>(base) - 10));
		}
		unsigned digitFromChar(char c)
		{
			return (c >= '0' && c <= '9') ? (c - '0') : ((c >= 'a' && c <= 'z') ? (c - 'a' + 10) : ((c >= 'A' && c <= 'Z') ? (c - 'A' + 10) : 0));
		}
		uint64_t strToInt(const std::string& str, bool& failed, unsigned base)
		{
			unsigned offset = 0;
			uint64_t value = 0;
			failed = true;
			while (str.size() > offset && str[offset] == ' ' && str[offset] == '\t')
				offset++;

			if (offset == str.size())
				return 0;

			for (; offset < str.size(); ++offset)
			{
				if (!isDigitInBase(str[offset], base))
				{
					break;
				}

				value *= base;
				value += digitFromChar(str[offset]);
			}

			for (; offset < str.size(); ++offset)
			{
				if (str[offset] != ' ' && str[offset] != '\t')
				{
					failed = true;
					return 0;
				}
			}

			failed = false;
			return value;
		}
		float strToFloat(const std::string& str, bool& failed)
		{
			failed = true;
			char* end = nullptr;
			const char* c = str.c_str();
			float ret = std::strtof(c, &end);

			if (end == c)
				return 0;

			failed = false;
			return ret;
		}
		bool strToBool(const std::string& str, bool& failed)
		{
			failed = false;
			if (str == "true" || str == "TRUE")
			{
				return true;
			}
			else if (str == "false" || str == "FALSE")
			{
				return false;
			}
			failed = true;
			return false;
		}
		Color strToColor(const std::string& str, bool& failed)
		{
			if (str.empty())
				return Color(255,255,255);

			if (str[0] == '#')
			{
				if (str.size() % 2 == 0 || str.size() == 1)
					return Color(255, 255, 255);
				uint32_t value = static_cast<uint32_t>(strToInt(str.substr(1), failed, 16));

				switch ((str.size() - 1) / 2)
				{
				case 1:
				{
					uint8_t v8 = static_cast<uint8_t>(value);
					value = v8 * ( 0x01010100) | 0xFF;
					break;
				}
				case 2:
				{
					uint8_t v8 = static_cast<uint8_t>(value >> 8);
					uint8_t a = static_cast<uint8_t>(value & 0xFF);
					value = v8 * (0x01010100) | a;
					break;
				}
				case 3:
				{
					value = value << 8 | 0xFF;
					break;
				}
				}

				if (!failed)
					return Color(value);
			}
			else
			{
				//color name lookup
			}

			return Color(255, 255, 255);
		}

		std::vector<std::string> splitString(const std::string& str)
		{
			std::stringstream ss(str);
			std::istream_iterator<std::string> begin(ss);
			std::istream_iterator<std::string> end;

			return std::vector<std::string>(begin, end);
		}
	}

	void Style::inherit(const Style& parentStyle)
	{
		for (const auto& value : parentStyle.values)
			if (!values.count(value.first))
				values.emplace(value);
	}
	std::shared_ptr<Style::Value> Style::getValue(const std::string& name) const
	{
		auto it = values.find(name);
		if (it != values.end())
			return it->second;
		return std::shared_ptr<Value>();
	}
	void Style::setValue(const std::string& name, const std::shared_ptr<Value>& value)
	{
		values[name] = value;
	}
}