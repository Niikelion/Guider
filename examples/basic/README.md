This example shows how to use Guider with SFML and create elements from code. In later examples you will learn how to use styles api and define layout of your application in separate resource files. Note, that this is not SFML tutorial. To learn more about SFML i recommend their official tutorials.

To demonstrate basic elements implemented, I have decided to create simple button list with two buttons: Increase and Decrease that increase and decrease counter respectively and some text to display current counter value.

After creating SFML window we need some texture to render to. (It is not necessary to render to separate texture. Rendering directly to window should still be possible.)

```c++
	// prepare ogl context settings
	sf::ContextSettings settings(0, 8, 4);
	// create render target
	sf::RenderTexture target;
	target.create(startWidth, startHeight, settings);
	// create gui sprite
	sf::Sprite gui;
	gui.setTexture(target.getTexture());
```

Now that we have texture, we have to create bridge between SFML and Guider - rendering backend. Current SfmlBackend implementation requires any valid sf::RenderTarget to work. While we are at it, let's create Engine instance and resize it to match window dimensions too.

```c++
	// create backend
	Guider::SfmlBackend backend(target);
	// create engine
	Guider::Engine engine(backend);
	engine.resize({
		static_cast<float>(startWidth),
		static_cast<float>(startHeight)
	});
```

Before we start adding some elements to draw, we need to add rendering of gui at the end of main loop

```c++
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
```

and add some event handling. Most events can be handled by SfmlBackend::handleEvent but we still need to do texture resizing.

```c++
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
			default:
				break;
			}
		}
```

It's finally time to add some elements! Let's start with root container. In this example it has the same size as our window and a white background. Just make sure to create it before main loop.

```c++
	// create root container
	std::shared_ptr<Guider::ConstraintsContainer> root = std::make_shared<Guider::ConstraintsContainer>();
	// fill window
	root->setSizingMode(Guider::SizingMode::MatchParent, Guider::SizingMode::MatchParent);
	// its good practice to give root element opaque background
	root->setBackgroundColor(Guider::Color(255, 255, 255));
```

Now, let's add vertical list and make it 200px wide.

```c++
	// create container for buttons
	std::shared_ptr<Guider::ListContainer> buttonList = std::make_shared<Guider::ListContainer>();
	// 200px width and height just enough to contain child elements
	buttonList->setSizingMode(Guider::SizingMode::OwnSize, Guider::SizingMode::WrapContent);
	// make list vertical
	buttonList->setOrientation(Guider::Orientation::Vertical);
	// set width to 200
	buttonList->setWidth(200);
```

Center it vertically and horizontally:

```c++
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
```

Container created, let's add some buttons!

```c++
	std::shared_ptr<Guider::BasicButtonComponent> upButton = std::make_shared<Guider::BasicButtonComponent>();
	upButton->setBackgroundDrawable(
		Guider::BasicButtonComponent::ButtonState::Default,
		backend.createRectangle({}, Guider::Color(200,200,200))
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
	upButton->setTextColor(Guider::Color(255,0,0));

	upButton->setOnClickCallback([&counter,&text](auto&)
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
```

If this block of code looks scary, don't warry. You will learn easier methods to define how elements look later on.

Before we forget, quickly add text to show counter value and attach root to our engine.

```c++
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
```

```c++
	//attach root element to engine
	engine.addChild(root);
```

And we are finally done! To learn more about Guider features, check out other examples.