#pragma once

#include <guider/base.hpp>
#include <algorithm>
#include <functional>

#include <parselib/XML/xml.hpp>

#include <guider/styles.hpp>

namespace Guider
{
	namespace XML
	{
		using namespace ParseLib::XML;
	}

	/// @brief Container for id mapping.
	class ComponentBindings
	{
	public:
		void registerElement(const std::string name, const Component::Type& component);
		Component::Type getElementById(const std::string& name);
	private:
		std::unordered_map<std::string, Component::Type> idMapping;
	};

	/// @brief Resource manager.
	///
	/// It is responsible for handling styles and resources,
	/// creating layouts from xml and finding elements in layouts.
	class Manager : public ComponentBindings
	{
	public:
		/// @brief Returns drawable that is mapped to the specified id.
		/// @param id Resource id.
		std::shared_ptr<Resources::Drawable> getDrawableById(uint64_t id);
		/// @brief Retunrs drawable that is mapped to the specified name.
		/// @param name Resource name.
		std::shared_ptr<Resources::Drawable> getDrawableByName(const std::string& name);
		/// @brief Returns drawable based on property string.
		/// 
		/// May return stored resource or solid color rect based on input.
		/// @param text Property string.
		std::shared_ptr<Resources::Drawable> getDrawableByText(const std::string& text);
		/// @brief Returns font that is mapped to the specified name.
		/// @param name Font name.
		std::shared_ptr<Resources::FontResource> getFontByName(const std::string& name);

		/// @brief Registers drawable with given id and name.
		/// @param drawable Resource.
		/// @param id Resource id.
		/// @param name Resource name(not required).
		void registerDrawable(const std::shared_ptr<Resources::Drawable>& drawable, uint64_t id, const std::string& name = "");
		/// @brief Loads drawable from file.
		/// @param filename File name.
		/// @param id Resource id.
		/// @param name Resource name(optional).
		void loadDrawable(const std::string& filename, uint64_t id, const std::string& name = "");
		/// @brief Loads drawable from file.
		/// @param filename File name.
		/// @param name Resoure name.
		void loadDrawable(const std::string& filename, const std::string& name);
		/// @brief Loads font from file.
		/// @param filename File name.
		/// @param name Resource name.
		void loadFont(const std::string& filename, const std::string& name);

		/// @brief Generates styling info from xml.
		/// 
		/// When parsing xml styling info, registered properties for given
		/// component type are used.
		/// @param config xml data.
		/// @param parent Parent styling info.
		StylingPack generateStyleInfo(const XML::Tag& config, const Theme& parent = Theme());

		/// @brief Handles common attributes.
		/// @param c Component.
		/// @param config xml.
		/// @param style Styling info.
		static void handleDefaultArguments(Component& c, const XML::Tag& config, const Style& style);

		/// @brief Registers creator for given type.
		/// @param f Factory function.
		/// @param name Alias for type.
		void registerTypeCreator(const std::function<Component::Type (Manager&, const XML::Tag&,ComponentBindings&,const StylingPack&)>& f, const std::string& name);

		/// @brief Registers properties for type.
		/// 
		/// Uses registerProperties(manager, name) static method from T. 
		/// @tparam T Components type.
		/// @param name Alias for type.
		template<typename T> void registerTypeProperties(const std::string& name)
		{
			T::registerProperties(*this, name);
		}
		/// @brief Registers type.
		/// 
		/// Uses registerProperties(manager, name) static method from T and
		/// T(manager, xml, styling info) constructor.
		/// @tparam T Components type.
		/// @param name Alias for type.
		template<typename T> void registerType(const std::string& name)
		{
			registerTypeCreator(creator<T>,name);
			registerTypeProperties<T>(name);
		}
		/// @brief Registers global property.
		/// @tparam T Property type.
		/// @param name Property name.
		template<typename T> void registerProperty(const std::string& name)
		{
			if (!propertyDefinitions.count(name))
				propertyDefinitions.emplace(name,Styles::ValueDefinition::create<T>());
		}

		/// @brief Registers global text property.
		/// @param name Property name.
		void registerStringProperty(const std::string& name);
		/// @brief Registers text property for component.
		/// @param component Component.
		/// @param name Property name.
		void registerStringProperty(const std::string& component, const std::string& name);
		/// @brief Registers global numeric property.
		/// @param name Property name.
		void registerNumericProperty(const std::string& name);
		/// @brief Registers numeric property for component.
		/// @param component Component.
		/// @param name Property name.
		void registerNumericProperty(const std::string& component, const std::string& name);
		/// @brief Registers global drawable property.
		/// @param name Property name.
		void registerDrawableProperty(const std::string& name);
		/// @brief Registers drawable property for component.
		/// @param component Component.
		/// @param name Property name.
		void registerDrawableProperty(const std::string& component, const std::string& name);
		/// @brief Registers global color property.
		/// @param name Property name.
		void registerColorProperty(const std::string& name);
		/// @brief Registers color property for component.
		/// @param component Component.
		/// @param name Property name.
		void registerColorProperty(const std::string& component, const std::string& name);

		/// @brief Registers creator for global property.
		/// @tparam T Property type.
		/// @param name Property name.
		/// @param f Factory function.
		template<typename T> void registerProperty(const std::string& name, const std::function<T(const std::string&)>& f)
		{
			propertyDefinitions.emplace(name, Styles::ValueDefinition([f](const std::string& str) 
				{
					Styles::Value r = Styles::Value(typeid(T), sizeof(T));
					r.prepare<T>(f(str));
					return r;
				}));
		}
		/// @brief Registers creator for property for component.
		/// @tparam T Property type.
		/// @param component Component.
		/// @param name Property name.
		/// @param f Factory function.
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

		/// @brief Sets default style for component.
		/// @param component Component.
		/// @param style Style.
		void setDefaultStyle(const std::string& component, const Style& style);
		/// @brief Loads style from xml.
		/// @param xml Xml.
		void loadStyleFromXml(const XML::Tag& xml);
		/// @brief Loads styles from xml file.
		/// @param filename File name.
		void loadStylesFromXmlFile(const std::string& filename);

		/// @brief Returns default style for component.
		/// @param component Component.
		Style getDefaultStyleFor(const std::string& component);

		/// @brief Registers theme.
		/// @param name Themes name.
		/// @param theme Theme.
		void registerTheme(const std::string& name, const Theme& theme);
		/// @brief Registers extended theme.
		/// @param name Themes name.
		/// @param theme Theme.
		void registerTheme(const std::string& name, const StylingPack& theme);
		/// @brief Loads theme from xml.
		/// @param xml Xml.
		void loadThemeFromXml(const XML::Tag& xml);
		/// @brief Loads themes from xml file.
		/// @param filename File name.
		void loadThemesFromXmlFile(const std::string& filename);

		/// @brief Returns theme by name.
		/// @param theme Name.
		StylingPack getTheme(const std::string& theme);

		/// @brief Instatiates gui structure.
		/// @param xml Xml source.
		/// @param bindings Bindings to capture ids.
		/// @param parentTheme Parent styling info.
		/// @return Root element of created structure.
		Component::Type instantiate(const XML::Tag& xml,ComponentBindings& bindings, const Theme& parentTheme = Theme());
		/// @brief Instatiates gui structure.
		/// @param xml Xml source.
		/// @param parentStyle Parent styling info.
		/// @return Root element of created structure.
		inline Component::Type instantiate(const XML::Tag& xml, const Theme& parentStyle = Theme())
		{
			return instantiate(xml, *this, parentStyle);
		}

		/// @brief Returns drawing backend.
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