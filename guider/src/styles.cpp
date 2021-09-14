#include <guider/styles.hpp>
#include <sstream>
#include <iterator>

namespace Guider
{
	namespace Styles
	{

		bool Variable::isReference() const
		{
			return reference;
		}

		String Variable::getValue() const
		{
			return value;
		}

		Variable::Variable(const String& value, bool reference) : reference(reference), value(value)
		{
		}

		bool VariableReference::detached() const noexcept
		{
			return static_cast<bool>(cache);
		}

		void VariableReference::attach(const std::shared_ptr<Value>& value)
		{
			cache = value;
		}

		String VariableReference::getName() const
		{
			return name;
		}

		std::shared_ptr<Value> VariableReference::getValue() const
		{
			return cache;
		}

		bool isDigitInBase(char c, unsigned base)
		{
			return base <= 10 ? (c >= '0' && c < '0' + static_cast<int>(base)) : ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'A' + static_cast<int>(base) - 10) || (c >= 'a' && c < 'a' + static_cast<int>(base) - 10));
		}
		unsigned digitFromChar(char c)
		{
			return (c >= '0' && c <= '9') ? (c - '0') : ((c >= 'a' && c <= 'z') ? (c - 'a' + 10) : ((c >= 'A' && c <= 'Z') ? (c - 'A' + 10) : 0));
		}
		String trim(const String& s)
		{
			String t = s;
			t.erase(0, t.find_first_not_of("\t\n\v\f\r "));
			size_t p = t.find_last_not_of("\t\n\v\f\r ");
			if (p != String::npos && p < t.size() - 1)
				t.erase(p + 1);
			return t;
		}
		uint64_t strToInt(const String& str, bool& failed, unsigned base)
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
		float strToFloat(const String& str, bool& failed)
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
		bool strToBool(const String& str, bool& failed)
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
		Color strToColor(const String& str, bool& failed)
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

		Vector<String> splitString(const String& str)
		{
			std::stringstream ss(str);
			std::istream_iterator<String> begin(ss);
			std::istream_iterator<String> end;

			return Vector<String>(begin, end);
		}
		
		String UnresolvedValue::getValue()
		{
			return value;
		}

		UnresolvedValue::UnresolvedValue(const String& value) : value(value)
		{
		}
}

	void Style::inheritAttributes(const Style& parentStyle)
	{
		for (const auto& i : parentStyle.attributes)
		{
			auto a = attributes.find(i.first);
			if (a == attributes.end())
				setAttribute(i.first,i.second);
		}
	}
	
	std::shared_ptr<Styles::Value> Style::getAttribute(const String& name) const
	{
		auto attr = attributes.find(name);
		if (attr != attributes.end())
			return attr->second;
		return std::shared_ptr<Styles::Value>();
	}
	
	
	void Style::setAttribute(const String& name, const std::shared_ptr<Styles::Value>& value)
	{
		attributes[name] = value;
	}
	void Style::setAttribute(const String& name, const String& variable)
	{
		auto it = attributes.find(name);
		attributes[name] = Styles::Value::ofType<Styles::VariableReference>(variable);
	}

	void Style::removeAttribute(const String& name)
	{
		attributes.erase(name);
	}

	std::shared_ptr<Styles::Variable> Theme::getVariable(const String& name) const
	{
		auto it = variables.find(name);
		if (it != variables.end())
			return it->second;
		return std::shared_ptr<Styles::Variable>();
	}
	std::shared_ptr<Styles::Variable> Theme::dereferenceVariable(const String& name) const
	{
		auto var = getVariable(name);
		std::unordered_set<Styles::Variable*> visited;
		while (var && var->isReference() && !visited.count(var.get()))
		{
			visited.insert(var.get());
			var = getVariable(var->getValue());
		}
		return var;
	}
	void Theme::setVariable(const String& name, const std::shared_ptr<Styles::Variable>& value)
	{
		variables[name] = value;
	}
	void Theme::inheritVariables(const Theme& parentStyle)
	{
		for (const auto& i : parentStyle.variables)
		{
			auto v = variables.find(i.first);
			if (v == variables.end())
				setVariable(i.first, i.second);
		}
	}
}