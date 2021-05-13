#pragma once

#include <guider/base.hpp>
#include <algorithm>
#include <functional>

#include <parselib/XML/xml.hpp>

#include <Guider/styles.hpp>

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
		void registerElement(const std::string name, const Component::Type& component);
		Component::Type getElementById(const std::string& name);
	};

	class Manager : public ComponentBindings
	{
	public:
		std::shared_ptr<Resources::Drawable> getDrawableById(uint64_t id);
		std::shared_ptr<Resources::Drawable> getDrawableByName(const std::string& name);
		std::shared_ptr<Resources::Drawable> getDrawableByText(const std::string& text);

		std::shared_ptr<Resources::FontResource> getFontByName(const std::string& name);

		void registerDrawable(const std::shared_ptr<Resources::Drawable>& drawable, uint64_t id, const std::string& name = "");
		void loadDrawable(const std::string& filename, uint64_t id, const std::string& name = "");
		void loadDrawable(const std::string& filename, const std::string& name);
		void loadFont(const std::string& filename, const std::string& name);

		StylingPack generateStyleInfo(const XML::Tag& config, const Theme& parent = Theme());

		static void handleDefaultArguments(Component& c, const XML::Tag& config, const Style& style);

		void registerTypeCreator(const std::function<Component::Type (Manager&, const XML::Tag&,ComponentBindings&,const StylingPack&)>& f, const std::string& name);

		template<typename T> void registerType(const std::string& name)
		{
			registerTypeCreator(creator<T>,name);
		}

		template<typename T> void registerTypeProperties(const std::string& name)
		{
			T::registerProperties(*this, name);
		}

		template<typename T> void registerProperty(const std::string& name)
		{
			if (!propertyDefinitions.count(name))
				propertyDefinitions.emplace(name,Style::ValueDefinition::create<T>());
		}

		void registerStringProperty(const std::string& name);
		void registerStringProperty(const std::string& component, const std::string& name);
		void registerDrawableProperty(const std::string& name);
		void registerDrawableProperty(const std::string& component, const std::string& name);
		void registerColorProperty(const std::string& name);
		void registerColorProperty(const std::string& component, const std::string& name);

		template<typename T> void registerProperty(const std::string& name, const std::function<T(const std::string&)>& f)
		{
			propertyDefinitions.emplace(name, Styles::ValueDefinition([f](const std::string& str) 
				{
					Styles::Value r = Styles::Value(typeid(T), sizeof(T));
					r.prepare<T>(f(str));
					return r;
				}));
		}

		template<typename T> void registerPropertyForComponent(const std::string& component, const std::string& name, const std::function<T(const std::string&)>& f)
		{
			auto it = componentsPropertyDefinitions.find(component);
			if (it == componentsPropertyDefinitions.end())
				it = componentsPropertyDefinitions.emplace(component, std::unordered_map<std::string, Styles::ValueDefinition>()).first;
			it->second.emplace(name, Styles::ValueDefinition([f](const std::string& str)
				{
					Styles::Value r = Styles::Value(typeid(T), sizeof(T));
					r.prepare<T>(f(str));
					return r;
				}));
		}

		void setDefaultStyle(const std::string& component, const Style& style);
		void loadStyleFromXml(const XML::Tag& xml);
		void loadStylesFromXmlFile(const std::string& filename);

		Style getDefaultStyleFor(const std::string& component);

		void registerTheme(const std::string& name, const Theme& theme);
		void registerTheme(const std::string& name, const StylingPack& theme);
		void loadThemeFromXml(const XML::Tag& xml);
		void loadThemesFromXmlFile(const std::string& filename);

		StylingPack getTheme(const std::string& theme);

		Component::Type instantiate(const XML::Tag& xml,ComponentBindings& bindings, const Theme& parentTheme = Theme());
		inline Component::Type instantiate(const XML::Tag& xml, const Theme& parentStyle = Theme())
		{
			return instantiate(xml, *this, parentStyle);
		}

		inline Backend& getBackend()
		{
			return backend;
		}

		Manager(Backend& b) : backend(b)
		{
			initDefaultProperties();
		}
	private:
		Backend& backend;
		std::unordered_map<std::string, std::function<Component::Type(Manager&, const XML::Tag&, ComponentBindings&,const StylingPack&)>> creators;

		template<typename T> static Component::Type creator(Manager& m, const XML::Tag& config, ComponentBindings& bindings, const StylingPack& style)
		{
			Component::Type ret = std::make_shared<T>(m, config, style);
			XML::Value tmp = config.getAttribute("id");
			if (tmp.exists())
			{
				bindings.registerElement(tmp.val, ret);
			}
			return ret;
		}
		std::unordered_map<std::string, Component::Type> idMapping;

		std::unordered_map<uint64_t, std::shared_ptr<Resources::Drawable>> drawablesById;
		std::unordered_map<std::string, uint64_t> drawableNameToIdMapping;

		std::unordered_map<std::string, Styles::ValueDefinition> propertyDefinitions;
		std::unordered_map<std::string, std::unordered_map<std::string, Styles::ValueDefinition>> componentsPropertyDefinitions;
		std::unordered_map<std::string, Style> defaultStyles;
		std::unordered_map<std::string, StylingPack> themes;

		std::unordered_map<std::string, std::shared_ptr<Resources::FontResource>> fontsByNames;

		void initDefaultProperties();
		std::shared_ptr<Styles::Value> createValueForProperty(const std::string& component, const std::string& name, const std::string& value);
	};
}