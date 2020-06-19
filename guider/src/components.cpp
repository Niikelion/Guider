#include <guider/components.hpp>

namespace Guider
{
	void RectangleShape::onDraw(RenderBackend& renderer) const
	{
		Rect bounds = getBounds();
		renderer.setColor(color);
		renderer.drawRectangle(Rect(0, 0, bounds.width, bounds.height));
	}
}