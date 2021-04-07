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
		void registerElement(const std::string name, Component::Type& component);
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

		Style generateStyle(const XML::Tag& config, const Style& parent = Style());

		static void handleDefaultArguments(Component& c, const XML::Tag& config, const Style& style);

		void registerTypeCreator(const std::function<Component::Type (Manager&, const XML::Tag&,ComponentBindings&,const Style&)>& f, const std::string& name);

		template<typename T> void registerType(const std::string& name)
		{
			registerTypeCreator(creator<T>,name);
		}

		template<typename T> void registerProperty(const std::string& name)
		{
			if (!propertyDefinitions.count(name))
				propertyDefinitions.emplace(name,Style::ValueDefinition::create<T>());
		}

		void registerStringProperty(const std::string& name);
		void registerDrawableProperty(const std::string& name);
		void registerColorProperty(const std::string& name);

		template<typename T> void registerProperty(const std::string& name, const std::function<T(const std::string&)>& f)
		{
			propertyDefinitions.emplace(name, Style::ValueDefinition([f](const std::string& str) 
				{
					Style::Value r = Style::Value(typeid(T), sizeof(T));
					r.prepare<T>(f(str));
					return r;
				}));
		}

		void setDefaultStyle(const std::string& component, const Style& style);

		Component::Type instantiate(const XML::Tag& xml,ComponentBindings& bindings, const Style& parentStyle = Style());
		inline Component::Type instantiate(const XML::Tag& xml, const Style& parentStyle = Style())
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
		//TODO: extract data from XML and store it in styles in elements tree
		std::unordered_map<std::string, std::function<Component::Type(Manager&, const XML::Tag&, ComponentBindings&,const Style&)>> creators;

		template<typename T> static Component::Type creator(Manager& m, const XML::Tag& config, ComponentBindings& bindings, const Style& style)
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

		std::unordered_map<std::string, Style::ValueDefinition> propertyDefinitions;
		std::unordered_map<std::string, Style> defaultStyles;

		std::unordered_map<std::string, std::shared_ptr<Resources::FontResource>> fontsByNames;

		void initDefaultProperties();
	};
}