#pragma once

#ifdef __APPLE__
#	define GLFW_INCLUDE_GLCOREARB
#endif
#include <GL/glew.h>

#include "nanovg.h"
#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"

#include "rae/core/Types.hpp"
#include "rae/ui/UITypes.hpp"

namespace rae
{

namespace UIRenderer
{
	// NanoVG takes input in pixels, and so do these helper functions:
	void renderLineNano(NVGcontext* vg, const vec2& from, const vec2& to,
			const Color& color = Color(0.1f, 0.1f, 0.1f, 1.0f));
	void renderBorderNano(NVGcontext* vg,
			const Rectangle& rectangle,
			const Color& color = Color(0.1f, 0.1f, 0.1f, 1.0f),
			float cornerRadius = 0.0f,
			float thickness = 1.0f);
	void renderCircleNano(NVGcontext* vg, const vec2& position, float diameter,
			const Color& color = Color(0.1f, 0.1f, 0.1f, 1.0f));
	void renderArcNano(NVGcontext* vg, const vec2& origin, float fromAngleRad, float toAngleRad,
		float diameter, float thickness, const Color& color = Color(0.1f, 0.1f, 0.1f, 1.0f));
	void renderRectangleNano(NVGcontext* vg, const Rectangle& rectangle,
			float cornerRadius,
			const Color& color = Color(0.1f, 0.1f, 0.1f, 1.0f));
	void renderWindowNano(NVGcontext* vg, const String& title, const Rectangle& rectangle,
			float cornerRadius, const Color& color = Color(0.1f, 0.1f, 0.1f, 1.0f));
	void renderButtonNano(NVGcontext* vg, const String& text, const Rectangle& rectangle,
			float cornerRadius,
			const Color& color = Color(0.1f, 0.1f, 0.1f, 1.0f),
			const Color& textColor = Color(1.0f, 1.0f, 1.0f, 1.0f));
	void renderTextNano(NVGcontext* vg, const String& text, const Rectangle& rectangle,
			const Color& textColor = Color(1.0f, 1.0f, 1.0f, 1.0f));
	void renderGrid(
		NVGcontext* vg,
		float width = 1920.0f,
		float height = 1080.0f,
		float pixelStep = 10.0f,
		vec2 startPos = vec2(0.0f, 0.0f));
}

}
