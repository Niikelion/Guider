#include <guider/components.hpp>
#include <guider/containers.hpp>
#include <guider/base.hpp>
#include <guider/shortcuts.hpp>

#include <SFML/Graphics.hpp>
#include <backend.hpp>

#include <filesystem>
#include <iostream>
#include <array>

const std::filesystem::path resourcePath(RESOURCE_FOLDER);

int main()
{
	// start size for window
	constexpr unsigned startWidth = 640, startHeight = 400;

	// prepare ogl context settings
	sf::ContextSettings settings(0, 8, 4);
	// create sfml window
	sf::RenderWindow window(sf::VideoMode(startWidth, startHeight), "Common elements", sf::Style::Default, settings);
	// create render target
	sf::RenderTexture target;
	target.create(startWidth, startHeight, settings);
	// create gui sprite
	sf::Sprite gui;
	gui.setTexture(target.getTexture());
	// create backend
	Guider::SfmlBackend backend(target);
	// create engine
	Guider::Engine engine(backend);
	engine.resize({
		static_cast<float>(startWidth),
		static_cast<float>(startHeight)
		});
	/*
		// load needed resources
		backend.loadFontFromFile((resourcePath / "fonts/Arimo-Regular.ttf").string(), "");

		// create root container
		std::shared_ptr<Guider::ConstraintsContainer> root = std::make_shared<Guider::ConstraintsContainer>();
		// fill window
		root->setSizingMode(Guider::SizingMode::MatchParent, Guider::SizingMode::MatchParent);
		// its good practice to give root element opaque background
		root->setBackgroundColor(Guider::Color::White);

		// create container for buttons
		std::shared_ptr<Guider::ListContainer> buttonList = std::make_shared<Guider::ListContainer>();
		// 200px width and height just enough to contain child elements
		buttonList->setSizingMode(Guider::SizingMode::OwnSize, Guider::SizingMode::WrapContent);
		// make list vertical
		buttonList->setOrientation(Guider::Orientation::Vertical);
		// set width to 200
		buttonList->setWidth(200);

		// add list as child of root container
		root->addChild(buttonList);
		{
			// setup horizontal centering constraint
			auto h = root->addConstraint(Guider::Orientation::Horizontal, buttonList, true);
			h->attachBetween(root, true, root, false, 0);
			h->setFlow(0.5);

			// setup vertical centering constraint
			auto v = root->addConstraint(Guider::Orientation::Vertical, buttonList, true);
			v->attachBetween(root, true, root, false, 0);
			v->setFlow(0.5);
		}

		// counter variable
		size_t counter = 0;

		// counter variable display
		std::shared_ptr<Guider::TextComponent> text = std::make_shared<Guider::TextComponent>();
		// tak just enough space to fit text
		text->setSizingMode(Guider::SizingMode::WrapContent, Guider::SizingMode::WrapContent);
		// set current counter value as text
		text->setText(std::to_string(counter));
		// center text
		text->setTextAlignment(Guider::Gravity::Center, Guider::Gravity::Center);
		// set size to 20
		text->setTextSize(20);
		// set font to "", our only font
		text->setFont("");
		// set color to black
		text->setTextColor(Guider::Color(0, 0, 0));
		root->addChild(text);
		{
			// setup horizontal centering between right edge of list and right edge of parent
			auto h = root->addConstraint(Guider::Orientation::Horizontal, text, true);
			h->attachBetween(buttonList, false, root, false, true);
			h->setFlow(0.5);

			// stick to the top of button list
			auto v = root->addConstraint(Guider::Orientation::Vertical, text, true);
			v->attachTopTo(buttonList, true, 0);
			v->attachBottomTo(root, false, 0);
			v->setFlow(0);
		}

		std::shared_ptr<Guider::BasicButtonComponent> upButton = std::make_shared<Guider::BasicButtonComponent>();
		upButton->setBackgroundDrawable(
			Guider::BasicButtonComponent::ButtonState::Default,
			backend.createRectangle({}, Guider::Color(200, 200, 200))
		);
		upButton->setBackgroundDrawable(
			Guider::BasicButtonComponent::ButtonState::Hovered,
			backend.createRectangle({}, Guider::Color(150, 150, 150))
		);
		upButton->setBackgroundDrawable(
			Guider::BasicButtonComponent::ButtonState::Clicked,
			backend.createRectangle({}, Guider::Color(100, 100, 100))
		);
		upButton->setSizingMode(Guider::SizingMode::MatchParent, Guider::SizingMode::WrapContent);
		upButton->setText("Increase");
		upButton->setTextAlignment(Guider::Gravity::Center, Guider::Gravity::Center);
		upButton->setTextSize(20);
		upButton->setFont("");
		upButton->setTextColor(Guider::Color(255, 0, 0));

		upButton->setOnClickCallback([&counter, &text](auto&)
			{
				++counter;
				std::cout << counter << "\n";
				text->setText(std::to_string(counter));
			});

		buttonList->addChild(upButton);

		std::shared_ptr<Guider::BasicButtonComponent> downButton = std::make_shared<Guider::BasicButtonComponent>();
		downButton->setBackgroundDrawable(
			Guider::BasicButtonComponent::ButtonState::Default,
			backend.createRectangle({}, Guider::Color(200, 200, 200))
		);
		downButton->setBackgroundDrawable(
			Guider::BasicButtonComponent::ButtonState::Hovered,
			backend.createRectangle({}, Guider::Color(150, 150, 150))
		);
		downButton->setBackgroundDrawable(
			Guider::BasicButtonComponent::ButtonState::Clicked,
			backend.createRectangle({}, Guider::Color(100, 100, 100))
		);
		downButton->setSizingMode(Guider::SizingMode::MatchParent, Guider::SizingMode::WrapContent);
		downButton->setText("Decrease");
		downButton->setTextAlignment(Guider::Gravity::Center, Guider::Gravity::Center);
		downButton->setTextSize(20);
		downButton->setFont("");
		downButton->setTextColor(Guider::Color(255, 0, 0));

		downButton->setOnClickCallback([&counter, &text](auto&)
			{
				if (counter > 0)
					--counter;
				std::cout << counter << "\n";
				text->setText(std::to_string(counter));
			});

		buttonList->addChild(downButton);


		//attach root element to engine
		engine.addChild(root);
		*/
		//view for caching
	sf::View view = window.getDefaultView();

	int n = 100;

	//main loop
	while (window.isOpen())
	{
		//event loop
		sf::Event event;
		while (window.pollEvent(event))
		{
			//default event handling
			Guider::SfmlBackend::handleEvent(engine, event);
			//additional cases not covered by default handling
			switch (event.type)
			{
			case sf::Event::KeyPressed:
			{
				//fast close, who have time to click close button with mouse?
				if (event.key.code == sf::Keyboard::Escape)
					window.close();
				break;
			}
			case sf::Event::Closed:
			{
				//standard for sfml
				window.close();
				break;
			}
			/*
			case sf::Event::Resized:
			{
				//default sfml backend implementation does not resize target
				//we have to explicitly handle it

				//get new window size
				sf::Vector2f size = {
					static_cast<float>(event.size.width),
					static_cast<float>(event.size.height)
				};
				//sfmlbackend does not resize given texture, we have to do it here
				target.create(event.size.width, event.size.height, settings);
				//reset rect, can be done better but this is easier
				gui.setTexture(target.getTexture(), true);
				//resize view
				view.setSize(size);
				view.setCenter(size.x / 2, size.y / 2);
				//apply changes
				window.setView(view);
				break;
			}
			*/
			default:
				break;
			}
		}
		//activate texture for rendering
		target.setActive();
		//update gui
		engine.update();
		//draw gui
		engine.draw();
		//render
		target.display();

		//activate window for rendering
		window.setActive();
		//clear window
		window.clear();
		//draw gui texture on window
		window.draw(gui);
		//render
		window.display();
	}
	return 0;
}