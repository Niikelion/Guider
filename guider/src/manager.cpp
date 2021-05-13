#include <guider/manager.hpp>
#include <fstream>

namespace Guider
{
	void ComponentBindings::registerElement(const std::string name, const Component::Type& component)
	{
		idMapping.emplace(name, component);
	}

	Component::Type ComponentBindings::getElementById(const std::string& name)
	{
		auto it = idMapping.find(name);
		if (it != idMapping.end())
			return it->second;
		return Component::Type();
	}

	std::shared_ptr<Resources::Drawable> Manager::getDrawableById(uint64_t id)
	{
		auto it = drawablesById.find(id);
		if (it != drawablesById.end())
		{
			return it->second;
		}
		return std::shared_ptr<Resources::Drawable>();
	}

	std::shared_ptr<Resources::Drawable> Manager::getDrawableByName(const std::string& name)
	{
		auto it = drawableNameToIdMapping.find(name);
		if (it != drawableNameToIdMapping.end())
		{
			return getDrawableById(it->second);
		}
		return std::shared_ptr<Resources::Drawable>();
	}

	std::shared_ptr<Resources::Drawable> Manager::getDrawableByText(const std::string& text)
	{
		std::string t = Styles::trim(text);

		if (!t.empty())
		{
			if (t[0] == '@') //resource handler
			{
				return getDrawableByName(t.substr(1));
			}
			else
			{
				bool failed = false;
				Color c = Styles::strToColor(t, failed);
				if (!failed)
				{
					return backend.createRectangle(Vec2(0, 0), c);
				}
			}
		}
		return std::shared_ptr<Resources::Drawable>();
	}

	std::shared_ptr<Resources::FontResource> Manager::getFontByName(const std::string& name)
	{
		auto it = fontsByNames.find(name);
		if (it != fontsByNames.end())
			return it->second;
		return std::shared_ptr<Resources::FontResource>();
	}

	void Manager::registerDrawable(const std::shared_ptr<Resources::Drawable>& drawable, uint64_t id, const std::string& name)
	{
		drawablesById.emplace(id, drawable);
		if (!name.empty())
		{
			drawableNameToIdMapping.emplace(name, id);
		}
	}

	void Manager::loadDrawable(const std::string& filename, uint64_t id, const std::string& name)
	{
		//TODO: deduce drawable type from file extension
		std::shared_ptr<Resources::Drawable> image = backend.loadImageFromFile(filename);
		registerDrawable(image, id, name);
	}

	void Manager::loadDrawable(const std::string& filename, const std::string& name)
	{
		uint64_t id = std::hash<std::string>()(name);
		if (drawablesById.count(id))
		{
			//TODO:resolve collision
			throw std::logic_error("Hash collision");
		}
		loadDrawable(filename, id, name);
	}

	void Manager::loadFont(const std::string& filename, const std::string& name)
	{
		fontsByNames.emplace(name, backend.loadFontFromFile(filename, name));
	}

	StylingPack Manager::generateStyleInfo(const XML::Tag& config, const Theme& parent)
	{
		Style s;

		//clone default style
		{
			auto it = defaultStyles.find(config.name);
			if (it != defaultStyles.end())
			{
				s.inheritAttributes(it->second);
			}
		}
		Theme theme = parent;
		auto t = config.getAttribute("theme");
		if (t.exists())
		{
			std::string v = Styles::trim(t.val);
			if (v.find('?') == 0)
				throw std::runtime_error("theme attribute cannot be a reference");
			auto it = themes.find(v);
			if (it == themes.end())
			{
				//error or smth
			}
			else
			{
				Theme tmp = it->second.theme;
				std::swap(tmp, theme);
				theme.inheritVariables(tmp);
				Style tmp1 = s;
				s = it->second.style;
				s.inheritAttributes(tmp1);
			}
		}

		std::unordered_set<std::string> propertiesToRemove;

		for (auto& attr : s.attributes)
		{
			if (attr.first != "theme")
			{
				if (attr.second->checkType<Styles::UnresolvedValue>())
				{
					attr.second = createValueForProperty(config.name, attr.first, attr.second->as<Styles::UnresolvedValue>().getValue());
					if (!attr.second)
					{
						propertiesToRemove.insert(attr.first);
						continue;
					}
				}
				if (attr.second->checkType<Styles::VariableReference>())
				{
					Styles::VariableReference& ref = attr.second->as<Styles::VariableReference>();
					auto var = theme.dereferenceVariable(ref.getName());
					if (var)
					{
						auto value = createValueForProperty(config.name, attr.first, var->getValue());

						if (value)
							s.setAttribute(attr.first, value);
						else
						{
							//missing variable
						}
					}
					else
					{
						propertiesToRemove.insert(attr.first);
					}
				}
			}
		}

		//override by explicitly specified styles

		for (const auto& i : config.attributes)
		{
			std::string v = Styles::trim(i.second.val);
			if (!v.empty() && v[0] == '?')
			{
				v.erase(0, 1);
				auto var = theme.dereferenceVariable(v);

				if (var)
				{
					v = var->getValue();
				}
				else
				{
					//missing variable
				}
			}

			auto value = createValueForProperty(config.name, i.first, v);

			propertiesToRemove.erase(i.first);

			if (value)
				s.setAttribute(i.first, value);
			else
			{
				propertiesToRemove.insert(i.first);
			}
		}

		for (const auto& i : propertiesToRemove)
			s.attributes.erase(i);

		return { s, theme };
	}

	void Manager::handleDefaultArguments(Component& c, const XML::Tag& config, const Style& style)
	{
		std::pair<float, Component::SizingMode> w, h;

		auto widthP = style.getAttribute("width");
		if (widthP)
			w = widthP->as<decltype(w)>();
		auto heightP = style.getAttribute("height");
		if (heightP)
			h = heightP->as<decltype(h)>();

		c.setSize(w.first, h.first);
		c.setSizingMode(w.second, h.second);
	}

	void Manager::registerTypeCreator(const std::function<Component::Type(Manager&, const XML::Tag&, ComponentBindings&, const StylingPack&)>& f, const std::string& name)
	{
		creators.emplace(name, f);
	}

	void Manager::registerStringProperty(const std::string& name)
	{
		registerProperty<std::string>(name, [](const std::string& s) { return s; });
	}

	void Manager::registerStringProperty(const std::string& component, const std::string& name)
	{
		registerPropertyForComponent<std::string>(component, name, [](const std::string& s) { return s; });
	}

	void Manager::registerDrawableProperty(const std::string& name)
	{
		registerProperty<std::shared_ptr<Resources::Drawable>>(name, std::bind(std::mem_fn(&Manager::getDrawableByText), this, std::placeholders::_1));
	}

	void Manager::registerDrawableProperty(const std::string& component, const std::string& name)
	{
		registerPropertyForComponent<std::shared_ptr<Resources::Drawable>>(component, name, std::bind(std::mem_fn(&Manager::getDrawableByText), this, std::placeholders::_1));
	}

	void Manager::registerColorProperty(const std::string& name)
	{
		registerProperty<Color>(name, [](const std::string& value) {
			bool failed = false;
			Color ret = Styles::strToColor(value);
			if (failed)
				throw std::invalid_argument("invalid color format");
			return ret;
			});
	}

	void Manager::registerColorProperty(const std::string& component, const std::string& name)
	{
		registerPropertyForComponent<Color>(component, name, [](const std::string& value) {
			bool failed = false;
			Color ret = Styles::strToColor(value);
			if (failed)
				throw std::invalid_argument("invalid color format");
			return ret;
			});
	}

	void Manager::setDefaultStyle(const std::string& component, const Style& style)
	{
		defaultStyles[component] = style;
	}

	void Manager::loadStyleFromXml(const XML::Tag& xml)
	{
		Style style;

		for (const auto& node : xml.children)
		{
			if (!node->isTextNode())
			{
				XML::Tag* tag = static_cast<XML::Tag*>(node.get());

				if (tag->name == "attr")
				{
					auto name = tag->getAttribute("name");
					auto value = tag->getAttribute("value");

					if (name.exists() && value.exists())
					{
						std::string v = Styles::trim(value.val);

						if (!v.empty() && v[0] == '?')
						{
							v.erase(0, 1);

							style.setAttribute(name.val, v);
						}
						else
						{
							auto value = createValueForProperty(xml.name, name.val, v);
							if (value)
								style.setAttribute(name.val, value);
							else
							{
								//missing variable
							}
						}
					}
				}
			}
		}

		setDefaultStyle(xml.name, style);
	}

	void Manager::loadStylesFromXmlFile(const std::string& filename)
	{
		std::string content;
		{
			std::ifstream t(filename);

			if (t.is_open())
			{
				t.seekg(0, std::ios::end);
				content.reserve(t.tellg());
				t.seekg(0, std::ios::beg);

				content.assign((std::istreambuf_iterator<char>(t)),
					std::istreambuf_iterator<char>());
			}
		}

		auto xmlRoot = Guider::XML::parse(content);
		for (const auto& s : xmlRoot->children)
		{
			if (!s->isTextNode())
			{
				XML::Tag* tag = static_cast<XML::Tag*>(s.get());

				if (tag->name == "Styles")
				{
					for (const auto& t : tag->children)
					{
						if (!t->isTextNode())
						{
							loadStyleFromXml(*static_cast<XML::Tag*>(t.get()));
						}
					}
				}
			}
		}
	}

	Style Manager::getDefaultStyleFor(const std::string& component)
	{
		auto it = defaultStyles.find(component);
		if (it != defaultStyles.end())
			return it->second;
		return Style();
	}

	void Manager::registerTheme(const std::string& name, const Theme& theme)
	{
		themes[name] = { Style(), theme };
	}

	void Manager::registerTheme(const std::string& name, const StylingPack& theme)
	{
		themes[name] = theme;
	}

	void Manager::loadThemeFromXml(const XML::Tag& xml)
	{
		StylingPack pack;
		auto parent = xml.getAttribute("extends");
		if (parent.exists())
		{
			auto it = themes.find(parent.val);
			if (it != themes.end())
			{
				pack.style.inheritAttributes(it->second.style);
				pack.theme.inheritVariables(it->second.theme);
			}
		}
		for (const auto& attr : xml.children)
		{
			if (!attr->isTextNode())
			{
				XML::Tag* tag = static_cast<XML::Tag*>(attr.get());
				auto name = tag->getAttribute("name");
				auto value = tag->getAttribute("value");
				if (tag->name == "var" && name.exists() && value.exists())
				{
					std::string v = Styles::trim(value.val);
					bool reference = v.find('?') == 0;
					if (reference)
						v.erase(0, 1);
					pack.theme.setVariable(name.val, std::make_shared<Styles::Variable>(v, reference));
				}
				else if (tag->name == "attr" && name.exists() && value.exists())
				{
					std::string v = Styles::trim(value.val);

					if (!v.empty() && v[0] == '?')
					{
						v.erase(0, 1);

						pack.style.setAttribute(name.val, v);
					}
					else
					{
						auto value = createValueForProperty("", name.val, v);
						if (value)
							pack.style.setAttribute(name.val, value);
						else
						{
							pack.style.setAttribute(name.val, Styles::Value::ofType<Styles::UnresolvedValue>(v));
						}
					}

				}
			}
		}
		themes.emplace(xml.name, pack);
	}

	void Manager::loadThemesFromXmlFile(const std::string& filename)
	{
		std::string content;
		{
			std::ifstream t(filename);

			if (t.is_open())
			{
				t.seekg(0, std::ios::end);
				content.reserve(t.tellg());
				t.seekg(0, std::ios::beg);

				content.assign((std::istreambuf_iterator<char>(t)),
					std::istreambuf_iterator<char>());
			}
		}

		auto xmlRoot = Guider::XML::parse(content);
		for (const auto& s : xmlRoot->children)
		{
			if (!s->isTextNode())
			{
				XML::Tag* tag = static_cast<XML::Tag*>(s.get());

				if (tag->name == "Themes")
				{
					for (const auto& t : tag->children)
					{
						if (!t->isTextNode())
						{
							loadThemeFromXml(*static_cast<XML::Tag*>(t.get()));
						}
					}
				}
			}
		}
	}

	StylingPack Manager::getTheme(const std::string& theme)
	{
		auto it = themes.find(theme);
		if (it != themes.end())
			return it->second;
		return StylingPack();
	}

	Component::Type Manager::instantiate(const XML::Tag& xml, ComponentBindings& bindings, const Theme& parentTheme)
	{
		auto it = creators.find(xml.name);
		if (it == creators.end())
		{
			throw std::logic_error("Component not supported");
			return Component::Type();
		}

		StylingPack style = generateStyleInfo(xml, parentTheme);

		return Component::Type(it->second(*this, xml, bindings, style));
	}

	std::pair<float, Component::SizingMode> strToMeasure(const std::string& str)
	{
		Component::SizingMode mode = Component::SizingMode::GivenSize;
		float value = 0;
		if (str.size())
		{
			if (str == "match_parent")
				mode = Component::SizingMode::MatchParent;
			else if (str == "wrap_content")
				mode = Component::SizingMode::WrapContent;
			else if (str == "own_size")
				mode = Component::SizingMode::OwnSize;
			else if (str == "given_size")
				mode = Component::SizingMode::GivenSize;
			else
			{
				//TODO: add str to float conversion
				value = Styles::strToFloat(str);
				mode = Component::SizingMode::OwnSize;
			}
		}
		return { value, mode };
	}

	void Manager::initDefaultProperties()
	{
		registerStringProperty("id");
		registerDrawableProperty("background");
		registerColorProperty("color");
		registerProperty<std::pair<float, Component::SizingMode>>("width", strToMeasure);
		registerProperty<std::pair<float, Component::SizingMode>>("height", strToMeasure);
	}
	std::shared_ptr<Styles::Value> Manager::createValueForProperty(const std::string& component, const std::string& name, const std::string& value)
	{
		std::string val = Styles::trim(value);
		if (!val.empty() && val[0] == '?')
		{
			return Styles::Value::ofType<Styles::VariableReference>(val.substr(1));
		}
		auto comDef = componentsPropertyDefinitions.find(component);
		if (comDef != componentsPropertyDefinitions.end())
		{
			auto propDef = comDef->second.find(name);
			if (propDef != comDef->second.end())
				return std::make_shared<Styles::Value>(propDef->second.value(val));
		}

		auto propDef = propertyDefinitions.find(name);
		if (propDef != propertyDefinitions.end())
			return std::make_shared<Styles::Value>(propDef->second.value(val));
		return std::shared_ptr<Styles::Value>();
	}
}