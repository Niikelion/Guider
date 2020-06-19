#pragma once
#include <guider/base.hpp>


namespace Guider
{
	class RectangleShape : public Component
	{
	private:
		Color color;
	public:
		void onDraw(RenderBackend& renderer) const override;

		RectangleShape() : color(255, 255, 255, 255) {}
		RectangleShape(SizingMode mode, const Color& c)
		{
			setSizingMode(mode, mode);
			color = c;
		}
		RectangleShape(float w, float h, const Color& c) : Component()
		{
			color = c;
			setSize(Vec2(w, h));
			setSizingMode(SizingMode::OwnSize, SizingMode::OwnSize);
		}
	};
}