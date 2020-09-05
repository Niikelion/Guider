#include <guider/manager.hpp>

namespace Guider
{
	void ComponentBindings::registerElement(const std::string name, Component::Type& component)
	{
		idMapping.emplace(name,component);
	}

	Component::Type ComponentBindings::getElementById(const std::string& name)
	{
		auto it = idMapping.find(name);
		if (it != idMapping.end())
			return it->second;
		return Component::Type();
	}

	void Manager::handleDefaultArguments(Component& c, const XML::Tag& config)
	{
		Component::SizingMode w = Component::SizingMode::OwnSize , h = Component::SizingMode::OwnSize;
		float ww = 0, hh = 0;

		XML::Value tmp = config.getAttribute("width");

		if (tmp.exists())
		{
			if (tmp.val == "match_parent")
				w = Component::SizingMode::MatchParent;
			else if (tmp.val == "wrap_content")
				w = Component::SizingMode::WrapContent;
			else if (tmp.val == "own_size")
				w = Component::SizingMode::OwnSize;
			else if (tmp.val == "given_size")
				w = Component::SizingMode::GivenSize;
			else
			{
				//TODO: add str to float conversion
				ww = Manager::strToFloat(tmp.val);
				w = Component::SizingMode::OwnSize;
			}
		}
		tmp = config.getAttribute("height");
		if (tmp.exists())
		{
			if (tmp.val == "match_parent")
				h = Component::SizingMode::MatchParent;
			else if (tmp.val == "wrap_content")
				h = Component::SizingMode::WrapContent;
			else if (tmp.val == "own_size")
				h = Component::SizingMode::OwnSize;
			else if (tmp.val == "given_size")
				h = Component::SizingMode::GivenSize;
			else
			{
				//TODO: add str to float conversion
				hh = Manager::strToFloat(tmp.val);
				h = Component::SizingMode::OwnSize;
			}
		}
		c.setSize(ww,hh);
		c.setSizingMode(w, h);
	}
	bool Manager::isDigitInBase(char c, unsigned base)
	{
		return base <= 10 ? (c >= '0' && c < '0' + static_cast<int>(base)) : ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'A'+static_cast<int>(base)-10) || (c >= 'a' && c < 'a'+static_cast<int>(base)-10));
	}
	unsigned Manager::digitFromChar(char c)
	{
		return (c >= '0' && c <= '9') ? (c - '0') : ((c >= 'a' && c <= 'z') ? (c - 'a' + 10) : ((c >= 'A' && c <= 'Z') ? (c - 'A' + 10) : 0));
	}
	uint64_t Manager::strToInt(const std::string& str, bool& failed, unsigned base)
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
	float Manager::strToFloat(const std::string& str, bool& failed)
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
	bool Manager::strToBool(const std::string& str, bool& failed)
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
	Color Manager::strToColor(const std::string& str, bool& failed)
	{
		uint32_t value = static_cast<uint32_t>(strToInt(str, failed, 16));

		if (failed)
		{
			//TODO: do color name lookup
		}

		return Color(value);
	}
	
	std::vector<std::string> Manager::splitString(const std::string& str)
	{
		std::stringstream ss(str);
		std::istream_iterator<std::string> begin(ss);
		std::istream_iterator<std::string> end;

		return std::vector<std::string>(begin,end);
	}
	void Manager::registerTypeCreator(const std::function<Component::Type (Manager&, const XML::Tag&, ComponentBindings&)>& f, const std::string& name)
	{
		creators.emplace(name, f);
	}

	Component::Type Manager::instantiate(const XML::Tag& xml,ComponentBindings& bindings)
	{
		auto it = creators.find(xml.name);
		if (it == creators.end())
		{
			//TODO: throw(type not supported)
			return Component::Type();
		}
		return Component::Type(it->second(*this, xml,bindings));
	}
}