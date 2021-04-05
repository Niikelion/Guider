#include <iostream>
#include <cstdlib>

#include <backend.hpp>
#include <guider/base.hpp>
#include <guider/containers.hpp>
#include <guider/components.hpp>

#include <string>
#include <fstream>

using namespace sf;
using namespace std;

namespace Gui
{
	using namespace Guider;
}

using Orientation = Gui::ConstraintsContainer::Constraint::Orientation;
using SizingMode = Gui::Component::SizingMode;

int main(int argc, char* argv[])
{
	unsigned width = 800;
	unsigned height = 450;

	RenderWindow window(VideoMode(width, height), "Guider layout creator", Style::Default, ContextSettings(0, 8, 2));

	sf::RenderTexture guiTexture;
	guiTexture.create(width, height, sf::ContextSettings(0, 8, 8));
	Gui::SfmlBackend renderer(guiTexture);

	Guider::Manager resourceManager(renderer);
	Guider::BasicButtonComponent::defineProperties(resourceManager);

	resourceManager.loadFont("resources/fonts/Arimo-Regular.ttf","arimo");

	Gui::Engine engine(renderer);
	engine.resize(Gui::Vec2((float)width, (float)height));

	resourceManager.registerTypeCreator([](Guider::Manager& m, const Guider::XML::Tag& config,Guider::ComponentBindings& bindings, const Guider::Style& style)
	{
		std::shared_ptr<Guider::ConstraintsContainer> ret = std::make_shared<Guider::ConstraintsContainer>();
		ret->postXmlConstruction(m, config, style);
		Guider::XML::Value tmp = config.getAttribute("id");
		if (tmp.exists())
		{
			bindings.registerElement(tmp.val,std::static_pointer_cast<Guider::Component>(ret));
		}
		return ret;
	}, "containers.ConstraintsContainer");
	resourceManager.registerType<Guider::ListContainer>("containers.ListContainer");
	resourceManager.registerType<Guider::RectangleShapeComponent>("shapes.Rectangle");
	resourceManager.registerType<Guider::EmptyComponent>("common.Guide");
	resourceManager.registerType<Guider::TextComponent>("common.Text");
	resourceManager.registerType<Guider::BasicButtonComponent>("common.Button");

	std::string mainLayout;
	{
		std::ifstream t("resources/layouts/main_layout.xml");

		if (t.is_open())
		{
			t.seekg(0, std::ios::end);
			mainLayout.reserve(t.tellg());
			t.seekg(0, std::ios::beg);

			mainLayout.assign((std::istreambuf_iterator<char>(t)),
				std::istreambuf_iterator<char>());
		}
		else
		{
			std::cout << argv[0] << std::endl;
		}
	}

	auto xmlRoot = Guider::XML::parse(mainLayout);

	Guider::Component::Type root = resourceManager.instantiate(*static_cast<Guider::XML::Tag*>(xmlRoot->children[0].get()));

	std::shared_ptr<Guider::BasicButtonComponent> branchButton = static_pointer_cast<Guider::BasicButtonComponent>(resourceManager.getElementById("branch_button"));
	branchButton->setText("none");
	
	engine.addChild(root);
	Sprite gui;
	gui.setTexture(guiTexture.getTexture());

	while (window.isOpen())
	{
		Event event;
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
			case Event::Closed:
			{
				window.close();
				break;
			}
			case Event::Resized:
			{
				Vector2f size(static_cast<float>(event.size.width), static_cast<float>(event.size.height));
			
				guiTexture.create(event.size.width, event.size.height,sf::ContextSettings(0,8,8));

				sf::FloatRect visibleArea(0, 0, size.x, size.y);
				window.setView(sf::View(visibleArea));
				engine.resize(Gui::Vec2(size.x, size.y));
			
				gui.setTexture(guiTexture.getTexture(), true);
				break;
			}
			case Event::MouseMoved:
			{
				engine.handleEvent(Guider::Event::createMouseEvent(Guider::Event::MouseEvent::Subtype::Moved,static_cast<float>(event.mouseMove.x),static_cast<float>(event.mouseMove.y),0));
				break;
			}
			case Event::MouseButtonPressed:
			{
				engine.handleEvent(Guider::Event::createMouseEvent(Guider::Event::MouseEvent::Subtype::ButtonDown, static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y), event.mouseButton.button));
				break;
			}
			case Event::MouseButtonReleased:
			{
				engine.handleEvent(Guider::Event::createMouseEvent(Guider::Event::MouseEvent::Subtype::ButtonUp, static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y), event.mouseButton.button));
				break;
			}
			case Event::KeyPressed:
			{
				if (event.key.code == Keyboard::Escape)
					window.close();
			}
			default:
				break;
			}
		}

		window.clear(Color(200, 200, 200));

		guiTexture.setActive();
		engine.update();
		engine.draw();

		guiTexture.display();

		window.setActive();
		window.draw(gui);
		window.display();
	}
}