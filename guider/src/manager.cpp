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
		drawablesById.emplace(id,drawable);
		if (!name.empty())
		{
			drawableNameToIdMapping.emplace(name,id);
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
		fontsByNames.emplace(name,backend.loadFontFromFile(filename, name));
	}

	Style Manager::generateStyle(const XML::Tag& config, const Style& parent)
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
		//add values not set by default styles
		s.inheritVariables(parent); 
		
		//override by explicitly specified styles

		for (const auto& i : config.attributes)
		{
			std::string v = Styles::trim(i.second.val);
			if (!v.empty() && v[0] == '?')
			{
				v.erase(0, 1);
				s.setAttribute(v, i.first);
			}
			else
			{
				auto it = propertyDefinitions.find(i.first);
				if (it != propertyDefinitions.end())
				{

					s.setAttribute(i.first, std::make_shared<Style::Value>(it->second.value(v)));
				}
			}
		}

		return s;
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

		c.setSize(w.first,h.first);
		c.setSizingMode(w.second, h.second);
	}
	
	void Manager::registerTypeCreator(const std::function<Component::Type (Manager&, const XML::Tag&, ComponentBindings&,const Style&)>& f, const std::string& name)
	{
		creators.emplace(name, f);
	}

	void Manager::registerStringProperty(const std::string& name)
	{
		registerProperty<std::string>(name, [](const std::string& s) { return s; });
	}

	void Manager::registerDrawableProperty(const std::string& name)
	{
		registerProperty<std::shared_ptr<Resources::Drawable>>(name,std::bind(std::mem_fn(&Manager::getDrawableByText),this,std::placeholders::_1));
	}

	void Manager::registerColorProperty(const std::string& name)
	{
		registerProperty<Color>(name,(Color(*)(const std::string&))Styles::strToColor);
	}

	void Manager::setDefaultStyle(const std::string& component, const Style& style)
	{
		defaultStyles[component] = style;
	}

	Component::Type Manager::instantiate(const XML::Tag& xml,ComponentBindings& bindings, const Style& parentStyle)
	{
		auto it = creators.find(xml.name);
		if (it == creators.end())
		{
			throw std::logic_error("Component not supported");
			return Component::Type();
		}

		Style style = generateStyle(xml,parentStyle);

		return Component::Type(it->second(*this, xml,bindings,style));
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
}