#include "rae/ui/UIRenderer.hpp"

using namespace rae;
using namespace rae::UIRenderer;

void UIRenderer::renderLineNano(NVGcontext* vg, const vec2& from, const vec2& to,
	const Color& color)
{
	nvgSave(vg);

	NVGcolor strokeColor = nvgRGBAf(color.r, color.g, color.b, color.a);

	nvgBeginPath(vg);
	nvgStrokeColor(vg, strokeColor);
	nvgStrokeWidth(vg, 1.0f);

	nvgMoveTo(vg, from.x, from.y);
	nvgLineTo(vg, to.x, to.y);
	nvgStroke(vg);

	nvgRestore(vg);
}

void UIRenderer::renderCircleNano(NVGcontext* vg, const vec2& position,
	float diameter, const Color& color)
{
	nvgSave(vg);

	NVGcolor fillColor = nvgRGBAf(color.r, color.g, color.b, color.a);

	nvgBeginPath(vg);
	nvgCircle(vg, position.x, position.y, diameter * 0.5f);
	nvgFillColor(vg, fillColor);
	nvgFill(vg);
	//nvgStrokeColor(vg, strokeColor);
	//nvgStrokeWidth(vg, 1.0f);
	//nvgStroke(vg);

	nvgRestore(vg);
}

void UIRenderer::renderArcNano(NVGcontext* vg, const vec2& origin, float fromAngleRad, float toAngleRad,
	float diameter, float thickness, const Color& color)
{
	NVGcolor fillColor = nvgRGBAf(color.r, color.g, color.b, color.a);
	//NVGcolor strokeColor = nvgRGBAf(0.0f, 0.0f, 0.0f, color.a);

	vec2 arcVec1 = vec2(cos(fromAngleRad), sin(fromAngleRad));
	vec2 arcVec2 = vec2(cos(toAngleRad), sin(toAngleRad));

	vec2 radii = vec2((diameter * 0.5f) - thickness, diameter * 0.5f);

	vec2 pos1 = origin + (arcVec1 * radii.x);
	vec2 pos2 = origin + (arcVec1 * radii.y);

	vec2 pos3 = origin + (arcVec2 * radii.x);
	vec2 pos4 = origin + (arcVec2 * radii.y);

	nvgSave(vg);
	nvgBeginPath(vg);
	nvgMoveTo(vg, pos1.x, pos1.y);
	nvgLineTo(vg, pos2.x, pos2.y);
	nvgArc(vg, origin.x, origin.y, radii.x, fromAngleRad, toAngleRad, NVG_CW);
	nvgLineTo(vg, pos3.x, pos3.y);
	nvgArc(vg, origin.x, origin.y, radii.y, toAngleRad, fromAngleRad, NVG_CCW);
	nvgClosePath(vg);

	//nvgStrokeColor(vg, strokeColor);
	//nvgStroke(vg);

	nvgFillColor(vg, fillColor);
	nvgFill(vg);

	nvgRestore(vg);
}

void UIRenderer::renderBorderNano(
	NVGcontext* vg,
	const Rectangle& rectangle,
	const Color& color,
	float cornerRadius,
	float thickness)
{
	// No negatives please:
	//if (w < 5.0f) w = 5.0f;
	//if (h < 5.0f) h = 5.0f;

	nvgSave(vg);

	NVGcolor strokeColor = nvgRGBAf(color.r, color.g, color.b, color.a);

	nvgBeginPath(vg);
	nvgRoundedRect(vg, rectangle.x, rectangle.y, rectangle.width, rectangle.height, cornerRadius);
	nvgStrokeColor(vg, strokeColor);
	nvgStrokeWidth(vg, thickness);
	nvgStroke(vg);

	nvgRestore(vg);
}

void UIRenderer::renderRectangleNano(NVGcontext* vg, const Rectangle& rectangle,
	float cornerRadius, const Color& color)
{
	NVGpaint shadowPaint;
	NVGpaint headerPaint;

	// No negatives please:
	//if (w < 5.0f) w = 5.0f;
	//if (h < 5.0f) h = 5.0f;

	nvgSave(vg);

	headerPaint = nvgLinearGradient(vg, rectangle.x, rectangle.y, rectangle.x, rectangle.y + 15,
		nvgRGBAf(color.r, color.g, color.b, color.a),
		nvgRGBAf(color.r, color.g, color.b, color.a));

	nvgBeginPath(vg);
	nvgRoundedRect(vg, rectangle.x, rectangle.y, rectangle.width, rectangle.height, cornerRadius);
	nvgFillPaint(vg, headerPaint);
	nvgFill(vg);

	// Drop shadow
	shadowPaint = nvgBoxGradient(vg, rectangle.x, rectangle.y+5, rectangle.width, rectangle.height,
		cornerRadius, 20,
		nvgRGBAf(0.0f, 0.0f, 0.0f, 0.5f),
		nvgRGBAf(0.0f, 0.0f, 0.0f, 0.0f));
	nvgBeginPath(vg);
	nvgRect(vg, rectangle.x - 60, rectangle.y - 60, rectangle.width + 120, rectangle.height + 120);
	nvgRoundedRect(vg, rectangle.x, rectangle.y, rectangle.width, rectangle.height, cornerRadius);
	nvgPathWinding(vg, NVG_HOLE);
	nvgFillPaint(vg, shadowPaint);
	nvgFill(vg);

	nvgRestore(vg);
}

void UIRenderer::renderWindowNano(NVGcontext* vg, const String& title, const Rectangle& rectangle,
							float cornerRadius, const Color& color)
{
	//float cornerRadius = 30.0f;
	NVGpaint shadowPaint;
	NVGpaint headerPaint;

	// No negative windows please:
	/*
	if (w < 30.0f)
	{
		w = 30.0f;
	}

	if (h < 30.0f)
	{
		h = 30.0f;
	}
	*/

	nvgSave(vg);
//	nvgClearState(vg);

	// Window

	headerPaint = nvgLinearGradient(vg, rectangle.x, rectangle.y, rectangle.x, rectangle.y+15,
		nvgRGBAf(color.r + 0.5f, color.g + 0.5f, color.b + 0.5f, color.a - 0.3f),
		nvgRGBAf(color.r, color.g, color.b, color.a));

	nvgBeginPath(vg);
	nvgRoundedRect(vg, rectangle.x, rectangle.y, rectangle.width, rectangle.height, cornerRadius);
	//nvgFillColor(vg, nvgRGBA(28,30,34,192));
	//	nvgFillColor(vg, nvgRGBA(0,0,0,128));
	//nvgFill(vg);

	//nvgFillColor(vg, nvgRGBA(155,155,155,255));
	nvgFillPaint(vg, headerPaint);
	nvgFill(vg);
	// RAE_TODO nvgStrokeColor(vg, nvgRGBAf(0.2f,0.2f,0.2f,0.75f * a()));
	nvgStrokeColor(vg, nvgRGBAf(0.2f,0.2f,0.2f,0.75f));
	nvgStrokeWidth(vg, 1.0f);
	nvgStroke(vg);

	// Drop shadow
	// RAE_TODO shadowPaint = nvgBoxGradient(vg, x,y+5, w,h, cornerRadius, 20, nvgRGBAf(0.0f,0.0f,0.0f,0.5f*a()), nvgRGBAf(0.0f,0.0f,0.0f,0.0f));
	shadowPaint = nvgBoxGradient(vg, rectangle.x, rectangle.y+5, rectangle.width, rectangle.height,
		cornerRadius, 20,
		nvgRGBAf(0.0f,0.0f,0.0f,0.5f), nvgRGBAf(0.0f,0.0f,0.0f,0.0f));
	nvgBeginPath(vg);
	nvgRect(vg, rectangle.x-60, rectangle.y-60, rectangle.width+120, rectangle.height+120);
	nvgRoundedRect(vg, rectangle.x, rectangle.y, rectangle.width, rectangle.height, cornerRadius);
	nvgPathWinding(vg, NVG_HOLE);
	nvgFillPaint(vg, shadowPaint);
	nvgFill(vg);
/*
	// Header
	headerPaint = nvgLinearGradient(vg, x,y,x,y+15, nvgRGBA(255,255,255,8), nvgRGBA(0,0,0,16));
	nvgBeginPath(vg);
	nvgRoundedRect(vg, x+1,y+1, w-2,30, cornerRadius-1);
	nvgFillPaint(vg, headerPaint);
	nvgFill(vg);
	nvgBeginPath(vg);
	nvgMoveTo(vg, x+0.5f, y+0.5f+30);
	nvgLineTo(vg, x+0.5f+w-1, y+0.5f+30);
	nvgStrokeColor(vg, nvgRGBA(0,0,0,32));
	nvgStroke(vg);
*/
	nvgFontSize(vg, 18.0f);
	nvgFontFace(vg, "sans-bold");
	nvgTextAlign(vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);

	// Shadow
	nvgFontBlur(vg,2);
	// RAE_TODO nvgFillColor(vg, nvgRGBAf(0.0f,0.0f,0.0f,0.5f*a()));
	nvgFillColor(vg, nvgRGBAf(0.0f,0.0f,0.0f,0.5f));
	nvgText(vg, rectangle.x + rectangle.width/2, rectangle.y+16+1, title.c_str(), nullptr);

	// Actual title text
	nvgFontBlur(vg,0);
	//textcolor:
	//nvgFillColor(vg, nvgRGBA(220,220,220,160));
	// RAE_TODO nvgFillColor(vg, nvgRGBAf(1.0f,1.0f,1.0f,a()));
	nvgFillColor(vg, nvgRGBAf(1.0f,1.0f,1.0f,1.0f));
	nvgText(vg, rectangle.x + rectangle.width/2, rectangle.y+16, title.c_str(), nullptr);

	nvgRestore(vg);
}

void UIRenderer::renderButtonNano(NVGcontext* vg, const String& text, const Rectangle& rectangle,
							float cornerRadius, const Color& color, const Color& textColor)
{
	NVGpaint shadowPaint;
	NVGpaint headerPaint;

	// No negative buttons please:
	//if (w < 5.0f) w = 5.0f;
	//if (h < 5.0f) h = 5.0f;

	nvgSave(vg);

	headerPaint = nvgLinearGradient(vg, rectangle.x, rectangle.y, rectangle.x, rectangle.y + 15,
		nvgRGBAf(color.r, color.g, color.b, color.a),
		nvgRGBAf(color.r, color.g, color.b, color.a));

	nvgBeginPath(vg);
	nvgRoundedRect(vg, rectangle.x, rectangle.y, rectangle.width, rectangle.height, cornerRadius);
	nvgFillPaint(vg, headerPaint);
	nvgFill(vg);

	/*
	// Outline
	nvgStrokeColor(vg, nvgRGBAf(0.2f,0.2f,0.2f,0.75f));
	nvgStrokeWidth(vg, 1.0f);
	nvgStroke(vg);
	*/

	// Drop shadow
	shadowPaint = nvgBoxGradient(vg, rectangle.x, rectangle.y+5, rectangle.width, rectangle.height,
		cornerRadius, 20,
		nvgRGBAf(0.0f, 0.0f, 0.0f, 0.5f), nvgRGBAf(0.0f, 0.0f, 0.0f, 0.0f));
	nvgBeginPath(vg);
	nvgRect(vg, rectangle.x - 60, rectangle.y - 60, rectangle.width + 120, rectangle.height + 120);
	nvgRoundedRect(vg, rectangle.x, rectangle.y, rectangle.width, rectangle.height, cornerRadius);
	nvgPathWinding(vg, NVG_HOLE);
	nvgFillPaint(vg, shadowPaint);
	nvgFill(vg);

	nvgFontSize(vg, 18.0f);
	nvgFontFace(vg, "sans-bold");
	nvgTextAlign(vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);

	// Text shadow
	nvgFontBlur(vg,2);
	nvgFillColor(vg, nvgRGBAf(0.0f, 0.0f, 0.0f, 0.5f));

	const float shadowOffset = 1.0f;
	const float rectangleHalfWidth = rectangle.width * 0.5f;
	const float rectangleHalfHeight = rectangle.height * 0.5f;

	nvgText(vg, rectangle.x + rectangleHalfWidth, rectangle.y + rectangleHalfHeight + shadowOffset, text.c_str(), nullptr);

	// Actual text
	nvgFontBlur(vg,0);
	nvgFillColor(vg, nvgRGBAf(textColor.r, textColor.g, textColor.b, textColor.a));
	nvgText(vg, rectangle.x + rectangleHalfWidth, rectangle.y + rectangleHalfHeight, text.c_str(), nullptr);

	nvgRestore(vg);
}

void UIRenderer::renderTextNano(
	NVGcontext* vg,
	const String& text,
	const Rectangle& rectangle,
	const Color& textColor)
{
	nvgSave(vg);

	nvgFontSize(vg, 18.0f);
	nvgFontFace(vg, "sans-bold");
	nvgTextAlign(vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);

	// Text shadow
	nvgFontBlur(vg,2);
	nvgFillColor(vg, nvgRGBAf(0.0f, 0.0f, 0.0f, 0.5f));

	const float shadowOffset = 1.0f;
	const float rectangleHalfWidth = rectangle.width * 0.5f;
	const float rectangleHalfHeight = rectangle.height * 0.5f;

	nvgText(vg,
		rectangle.x + rectangleHalfWidth,
		rectangle.y + rectangleHalfHeight + shadowOffset,
		text.c_str(), nullptr);

	// Actual text
	nvgFontBlur(vg,0);
	nvgFillColor(vg, nvgRGBAf(textColor.r, textColor.g, textColor.b, textColor.a));
	nvgText(vg,
		rectangle.x + rectangleHalfWidth,
		rectangle.y + rectangleHalfHeight,
		text.c_str(), nullptr);

	nvgRestore(vg);
}

void UIRenderer::renderGrid(NVGcontext* vg, float width, float height, float pixelStep, vec2 startPos)
{
	Color color = Colors::darkGray;
	Color midColor = Colors::gray;
	Color accentColor = Colors::lightGray;

	nvgSave(vg);

	//nvgShapeAntiAlias(vg, 0); // Should be fixed in later NanoVG: https://github.com/memononen/nanovg/issues/471

	NVGcolor strokeColor = nvgRGBAf(color.r, color.g, color.b, color.a);
	NVGcolor midColor2 = nvgRGBAf(midColor.r, midColor.g, midColor.b, midColor.a);
	NVGcolor accColor = nvgRGBAf(accentColor.r, accentColor.g, accentColor.b, accentColor.a);
	nvgStrokeColor(vg, strokeColor);
	nvgStrokeWidth(vg, 1.0f);

	auto drawLines = [&strokeColor, &midColor2, &accColor]
		(NVGcontext* vg, int lineCount, vec2 pos, vec2 xIter, vec2 yIter)
	{
		for (int i = 0; i < lineCount; ++i)
		{
			if (i % 100 == 0)
			{
				nvgStrokeColor(vg, accColor);
			}
			else if (i % 10 == 0)
			{
				nvgStrokeColor(vg, midColor2);
			}
			else
			{
				nvgStrokeColor(vg, strokeColor);
			}

			nvgBeginPath(vg);
			nvgMoveTo(vg, pos.x, pos.y);
			pos += xIter;
			nvgLineTo(vg, pos.x, pos.y);
			nvgStroke(vg);
			pos -= xIter;
			pos += yIter;
		}
	};

	// Horizontal lines
	drawLines(vg, 1+(int)height/pixelStep, startPos, vec2(width, 0.0f), vec2(0.0f, pixelStep));
	// Vertical lines
	drawLines(vg, 1+(int)width/pixelStep, startPos, vec2(0.0f, height), vec2(pixelStep, 0.0f));

	nvgRestore(vg);
}
