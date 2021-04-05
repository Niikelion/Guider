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
		std::string trim(const std::string& s)
		{
			std::string t = s;
			t.erase(0, t.find_first_not_of("\t\n\v\f\r "));
			size_t p = t.find_last_not_of("\t\n\v\f\r ");
			if (p != std::string::npos && p < t.size() - 1)
				t.erase(p + 1);
			return t;
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


	bool Style::VariableReference::detached() const noexcept
	{
		return static_cast<bool>(cache);
	}

	void Style::VariableReference::attach(const std::shared_ptr<Value>& value)
	{
		cache = value;
	}

	std::string Style::VariableReference::getName() const
	{
		return name;
	}

	std::shared_ptr<Style::Value> Style::VariableReference::getValue() const
	{
		return cache;
	}

	void Style::inheritAll(const Style& parentStyle)
	{
		inheritVariables(parentStyle);
		inheritAttributes(parentStyle);
	}
	void Style::inheritVariables(const Style& parentStyle)
	{
		for (const auto& i : parentStyle.variables)
		{
			auto v = variables.find(i.first);
			if (v == variables.end())
				setVariable(i.first,i.second);
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
	
	std::shared_ptr<Style::Value> Style::getAttribute(const std::string& name) const
	{
		return dereference(getRawAttribute(name));
	}
	std::shared_ptr<Style::Value> Style::getVariable(const std::string& name) const
	{
		return dereference(getRawVariable(name));
	}
	std::shared_ptr<Style::Value> Style::getRawAttribute(const std::string& name) const
	{
		auto attr = attributes.find(name);
		if (attr != attributes.end())
			return attr->second;
		return std::shared_ptr<Value>();
	}
	std::shared_ptr<Style::Value> Style::getRawVariable(const std::string& name) const
	{
		auto var = variables.find(name);
		if (var != variables.end())
			return var->second;
		return std::shared_ptr<Value>();
	}
	std::shared_ptr<Style::Value> Style::dereference(const std::shared_ptr<Value>& value) const
	{
		std::shared_ptr<Value> current = value;
		while (value && value->checkType<VariableReference>())
		{
			VariableReference& v = current->as<VariableReference>();
			if (v.detached() || true) //TODO: add proper handling later
			{
				auto var = variables.find(v.getName());
				if (var != variables.end())
					v.attach(var->second);
			}
			current = v.detached() ? std::shared_ptr<Value>() : v.getValue();
		}
		return current;
	}
	void Style::setVariable(const std::string& name, const std::shared_ptr<Value>& value)
	{
		variables[name] = value;
	}
	void Style::setAttribute(const std::string& name, const std::shared_ptr<Value>& value)
	{
		attributes[name] = value;
	}
	void Style::setAttribute(const std::string& name, const std::string& variable)
	{
		auto it = attributes.find(name);
		auto var = variables.find(variable);
		if (var == variables.end())
		{
			attributes[name] = Value::ofType<VariableReference>(variable);
		}
		else
		{
			attributes[name] = Value::ofType<VariableReference>(variable,var->second);
		}
	}
	void Style::addDependency(const std::string& attribute, const std::string& variable)
	{
		auto deps = variableDeps.emplace(variable,std::unordered_set<std::string>());
		deps.first->second.emplace(variable);
	}
	void Style::removeDependency(const std::string& attribute, const std::string& variable)
	{
		auto deps = variableDeps.find(variable);
		if (deps != variableDeps.end())
			deps->second.erase(attribute);
	}
}