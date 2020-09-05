#pragma once

#include <guider/base.hpp>
#include <algorithm>
#include <functional>

#include <parselib/XML/xml.hpp>

namespace Guider
{
	namespace XML
	{
		using namespace ParseLib::XML;
	}

	class ComponentBindings
	{
	private:
		std::unordered_map<std::string, Component::Type> idMapping;
	public:
		void registerElement(const std::string name, Component::Type& component);
		Component::Type getElementById(const std::string& name);
	};

	class Manager : public ComponentBindings
	{
	private:
		std::unordered_map<std::string, std::function<Component::Type (Manager&,const XML::Tag&,ComponentBindings&)>> creators;

		template<typename T> static Component::Type creator(Manager& m,const XML::Tag& config,ComponentBindings& bindings)
		{
			Component::Type ret = std::make_shared<T>(m, config);
			XML::Value tmp = config.getAttribute("id");
			if (tmp.exists())
			{
				bindings.registerElement(tmp.val, ret);
			}
			return ret;
		}
		std::unordered_map<std::string, Component::Type> idMapping;
	public:
		static void handleDefaultArguments(Component& c, const XML::Tag& config);
		static bool isDigitInBase(char c, unsigned base);
		static unsigned digitFromChar(char c);

		static uint64_t strToInt(const std::string& str, bool& failed, unsigned base = 10);
		static inline uint64_t strToInt(const std::string& str, unsigned base = 10)
		{
			bool a;
			return strToInt(str, a, base);
		}
		static float strToFloat(const std::string& str, bool& failed);
		static inline float strToFloat(const std::string& str)
		{
			bool failed = false;
			return strToFloat(str, failed);
		}
		static bool strToBool(const std::string& str, bool& failed);
		static inline bool strToBool(const std::string& str)
		{
			bool failed = false;
			return strToBool(str, failed);
		}
		static Color strToColor(const std::string& str,bool& failed);
		static inline Color strToColor(const std::string& str)
		{
			bool failed = false;
			return strToColor(str, failed);
		}
		static std::vector<std::string> splitString(const std::string& str);

		void registerTypeCreator(const std::function<Component::Type (Manager&, const XML::Tag&,ComponentBindings&)>& f, const std::string& name);

		template<typename T>void registerType(const std::string& name)
		{
			registerTypeCreator(creator<T>,name);
		}

		Component::Type instantiate(const XML::Tag& xml,ComponentBindings& bindings);
		inline Component::Type instantiate(const XML::Tag& xml)
		{
			return instantiate(xml, *this);
		}
	};
}