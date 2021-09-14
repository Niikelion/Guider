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
		void registerElement(const String name, const Component::Type& component);
		Component::Type getElementById(const String& name);
	private:
		std::unordered_map<String, Component::Type> idMapping;
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
		std::shared_ptr<Resources::Drawable> getDrawableByName(const String& name);
		/// @brief Returns drawable based on property string.
		/// 
		/// May return stored resource or solid color rect based on input.
		/// @param text Property string.
		std::shared_ptr<Resources::Drawable> getDrawableByText(const String& text);
		/// @brief Returns font that is mapped to the specified name.
		/// @param name Font name.
		std::shared_ptr<Resources::FontResource> getFontByName(const String& name);

		/// @brief Registers drawable with given id and name.
		/// @param drawable Resource.
		/// @param id Resource id.
		/// @param name Resource name(not required).
		void registerDrawable(const std::shared_ptr<Resources::Drawable>& drawable, uint64_t id, const String& name = "");
		/// @brief Loads drawable from file.
		/// @param filename File name.
		/// @param id Resource id.
		/// @param name Resource name(optional).
		void loadDrawable(const String& filename, uint64_t id, const String& name = "");
		/// @brief Loads drawable from file.
		/// @param filename File name.
		/// @param name Resoure name.
		void loadDrawable(const String& filename, const String& name);
		/// @brief Loads font from file.
		/// @param filename File name.
		/// @param name Resource name.
		void loadFont(const String& filename, const String& name);

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
		void registerTypeCreator(const std::function<Component::Type (Manager&, const XML::Tag&,ComponentBindings&,const StylingPack&)>& f, const String& name);

		/// @brief Registers properties for type.
		/// 
		/// Uses registerProperties(manager, name) static method from T. 
		/// @tparam T Components type.
		/// @param name Alias for type.
		template<typename T> void registerTypeProperties(const String& name)
		{
			T::registerProperties(*this, name);
		}
		/// @brief Registers type.
		/// 
		/// Uses registerProperties(manager, name) static method from T and
		/// T(manager, xml, styling info) constructor.
		/// @tparam T Components type.
		/// @param name Alias for type.
		template<typename T> void registerType(const String& name)
		{
			registerTypeCreator(creator<T>,name);
			registerTypeProperties<T>(name);
		}
		/// @brief Registers global property.
		/// @tparam T Property type.
		/// @param name Property name.
		template<typename T> void registerProperty(const String& name)
		{
			if (!propertyDefinitions.count(name))
				propertyDefinitions.emplace(name,Styles::ValueDefinition::create<T>());
		}

		/// @brief Registers global text property.
		/// @param name Property name.
		void registerStringProperty(const String& name);
		/// @brief Registers text property for component.
		/// @param component Component.
		/// @param name Property name.
		void registerStringProperty(const String& component, const String& name);
		/// @brief Registers global numeric property.
		/// @param name Property name.
		void registerNumericProperty(const String& name);
		/// @brief Registers numeric property for component.
		/// @param component Component.
		/// @param name Property name.
		void registerNumericProperty(const String& component, const String& name);
		/// @brief Registers global drawable property.
		/// @param name Property name.
		void registerDrawableProperty(const String& name);
		/// @brief Registers drawable property for component.
		/// @param component Component.
		/// @param name Property name.
		void registerDrawableProperty(const String& component, const String& name);
		/// @brief Registers global color property.
		/// @param name Property name.
		void registerColorProperty(const String& name);
		/// @brief Registers color property for component.
		/// @param component Component.
		/// @param name Property name.
		void registerColorProperty(const String& component, const String& name);

		/// @brief Registers creator for global property.
		/// @tparam T Property type.
		/// @param name Property name.
		/// @param f Factory function.
		template<typename T> void registerProperty(const String& name, const std::function<T(const String&)>& f)
		{
			propertyDefinitions.emplace(name, Styles::ValueDefinition([f](const String& str) 
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
		template<typename T> void registerPropertyForComponent(const String& component, const String& name, const std::function<T(const String&)>& f)
		{
			auto it = componentsPropertyDefinitions.find(component);
			if (it == componentsPropertyDefinitions.end())
				it = componentsPropertyDefinitions.emplace(component, std::unordered_map<String, Styles::ValueDefinition>()).first;
			it->second.emplace(name, Styles::ValueDefinition([f](const String& str)
				{
					Styles::Value r = Styles::Value(typeid(T), sizeof(T));
					r.prepare<T>(f(str));
					return r;
				}));
		}

		/// @brief Sets default style for component.
		/// @param component Component.
		/// @param style Style.
		void setDefaultStyle(const String& component, const Style& style);
		/// @brief Loads style from xml.
		/// @param xml Xml.
		void loadStyleFromXml(const XML::Tag& xml);
		/// @brief Loads styles from xml file.
		/// @param filename File name.
		void loadStylesFromXmlFile(const String& filename);

		/// @brief Returns default style for component.
		/// @param component Component.
		Style getDefaultStyleFor(const String& component);

		/// @brief Registers theme.
		/// @param name Themes name.
		/// @param theme Theme.
		void registerTheme(const String& name, const Theme& theme);
		/// @brief Registers extended theme.
		/// @param name Themes name.
		/// @param theme Theme.
		void registerTheme(const String& name, const StylingPack& theme);
		/// @brief Loads theme from xml.
		/// @param xml Xml.
		void loadThemeFromXml(const XML::Tag& xml);
		/// @brief Loads themes from xml file.
		/// @param filename File name.
		void loadThemesFromXmlFile(const String& filename);

		/// @brief Returns theme by name.
		/// @param theme Name.
		StylingPack getTheme(const String& theme);

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
		std::unordered_map<String, std::function<Component::Type(Manager&, const XML::Tag&, ComponentBindings&,const StylingPack&)>> creators;

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
		std::unordered_map<String, Component::Type> idMapping;

		std::unordered_map<uint64_t, std::shared_ptr<Resources::Drawable>> drawablesById;
		std::unordered_map<String, uint64_t> drawableNameToIdMapping;

		std::unordered_map<String, Styles::ValueDefinition> propertyDefinitions;
		std::unordered_map<String, std::unordered_map<String, Styles::ValueDefinition>> componentsPropertyDefinitions;
		std::unordered_map<String, Style> defaultStyles;
		std::unordered_map<String, StylingPack> themes;

		std::unordered_map<String, std::shared_ptr<Resources::FontResource>> fontsByNames;

		void initDefaultProperties();
		std::shared_ptr<Styles::Value> createValueForProperty(const String& component, const String& name, const String& value);
	};
}