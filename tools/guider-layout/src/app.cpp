#include <app.hpp>

#include <SFML/OpenGL.hpp>
#include <filesystem>
#include <deque>

void App::createWindow(size_t width, size_t height, const std::string& title)
{
	window = std::make_shared<sf::RenderWindow>(
		sf::VideoMode(static_cast<unsigned int>(width),
			static_cast<unsigned int>(height)),
		"Guider layout creator",
		sf::Style::Default,
		sf::ContextSettings(0, 8, 4)
	);
}

void App::initManager()
{
	if (window)
	{
		sf::Vector2u size = window->getSize();
		buffer = std::make_shared<sf::RenderTexture>();
		buffer->create(size.x, size.y, sf::ContextSettings(0, 8, 4));
		backend = std::make_shared<Gui::SfmlBackend>(*buffer);
		guiManager = std::make_shared<Gui::Manager>(*backend);
		engine = std::make_shared<Gui::Engine>(*backend);
		guiSprite = std::make_shared<sf::Sprite>();
		guiSprite->setTexture(buffer->getTexture());
	}
}

void App::loadResources()
{
	namespace fs = std::filesystem;
	if (guiManager)
	{
		fs::path resourcesPath(RESOURCE_FOLDER);
		//fonts
		{
			fs::path fontsPath = resourcesPath / "fonts";
			guiManager->loadFont((fontsPath / "Arimo-Regular.ttf").string(), "");
			for (auto& f : fs::recursive_directory_iterator(fontsPath))
			{
				if (f.is_regular_file())
				{
					auto rp = fs::relative(f.path(), fontsPath);
					guiManager->loadFont(f.path().string(),
						(rp.parent_path() / rp.stem()).string());
				}
			}
		}
		//styles
		{
			fs::path stylesPath = resourcesPath / "styles";
			for (auto& f : fs::recursive_directory_iterator(stylesPath))
			{
				if (f.is_regular_file())
					guiManager->loadStylesFromXmlFile(f.path().string());
			}
		}
		//themes
		{
			fs::path themesPath = resourcesPath / "themes";
			for (auto& f : fs::recursive_directory_iterator(themesPath))
			{
				if (f.is_regular_file())
					guiManager->loadThemesFromXmlFile(f.path().string());
			}
		}
	}
}

void App::addDefinitions()
{
	if (guiManager)
	{
		guiManager->registerTypeCreator([](Guider::Manager& m, const Guider::XML::Tag& config, Guider::ComponentBindings& bindings, const Guider::StylingPack& style)
			{
				std::shared_ptr<Guider::ConstraintsContainer> ret = std::make_shared<Guider::ConstraintsContainer>();
				ret->postXmlConstruction(m, config, style);
				Guider::XML::Value tmp = config.getAttribute("id");
				if (tmp.exists())
				{
					bindings.registerElement(tmp.val, std::static_pointer_cast<Guider::Component>(ret));
				}
				return ret;
			}, "containers.ConstraintsContainer");
		guiManager->registerTypeProperties<Guider::ConstraintsContainer>("containers.ConstraintsContainer");
		guiManager->registerType<Guider::ListContainer>("containers.ListContainer");
		guiManager->registerType<Guider::RectangleShapeComponent>("shapes.Rectangle");
		guiManager->registerType<Guider::EmptyComponent>("common.Guide");
		guiManager->registerType<Guider::TextComponent>("common.Text");
		guiManager->registerTypeProperties<Guider::TextComponent>("common.Text");
		guiManager->registerType<Guider::BasicButtonComponent>("common.Button");
		guiManager->registerTypeProperties<Guider::BasicButtonComponent>("common.Button");
	}
}

void App::showWindow(bool show)
{
	if (window)
		window->setVisible(show);
}

void App::run()
{
	if (window)
	{
		bool first = true;
		while (window->isOpen())
		{
			sf::Event event;
			std::deque<sf::Event> events;
			while (window->pollEvent(event))
			{
				switch (event.type)
				{
				case sf::Event::Closed:
				{
					window->close();
					break;
				}
				case sf::Event::Resized:
				{
					events.push_back(event);
					sf::Vector2f size(static_cast<float>(event.size.width), static_cast<float>(event.size.height));

					buffer->create(event.size.width, event.size.height, sf::ContextSettings(0, 8, 4));

					sf::FloatRect visibleArea(0, 0, size.x, size.y);
					window->setView(sf::View(visibleArea));

					guiSprite->setTexture(buffer->getTexture(), true);
					break;
				}
				case sf::Event::MouseMoved:
				case sf::Event::MouseButtonPressed:
				case sf::Event::MouseButtonReleased:
				{
					events.push_back(event);
					break;
				}
				case sf::Event::KeyPressed:
				{
					if (event.key.code == sf::Keyboard::Escape)
						window->close();
					break;
				}
				default:
					break;
				}
			}

			timer.start();

			for (const auto& e : events)
			{
				switch (e.type)
				{
				case sf::Event::Resized:
				{
					sf::Vector2f size(static_cast<float>(e.size.width), static_cast<float>(e.size.height));

					engine->resize(Gui::Vec2(size.x, size.y));
					break;
				}
				case sf::Event::MouseMoved:
				{
					engine->handleEvent(Guider::Event::createMouseEvent(Guider::Event::MouseEvent::Subtype::Moved, static_cast<float>(event.mouseMove.x), static_cast<float>(event.mouseMove.y), 0));
					break;
				}
				case sf::Event::MouseButtonPressed:
				{
					engine->handleEvent(Guider::Event::createMouseEvent(Guider::Event::MouseEvent::Subtype::ButtonDown, static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y), event.mouseButton.button));
					break;
				}
				case sf::Event::MouseButtonReleased:
				{
					engine->handleEvent(Guider::Event::createMouseEvent(Guider::Event::MouseEvent::Subtype::ButtonUp, static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y), event.mouseButton.button));
					break;
				}
				default:
					break;
				}
			}

			buffer->setActive();
			engine->update();
			engine->draw();

			buffer->display();

			if (first)
			{
				first = false;
				timer.start();
			}

			timer.sample();

			window->clear(sf::Color(200, 200, 200));
			window->setActive();
			window->draw(*guiSprite);
			window->display();
			
			getManager()->getElementById("time")->as<Guider::TextComponent>().setText(std::to_string(timer.getMax()) + " / " + std::to_string(timer.getAverage()));
		}
	}
}
