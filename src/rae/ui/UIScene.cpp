#include "rae/ui/UISystem.hpp"

#include "loguru/loguru.hpp"
#include "rae/core/Utils.hpp"
#include "rae/core/Time.hpp"
#include "rae/core/ScreenSystem.hpp"
#include "rae/ui/Input.hpp"
#include "rae/asset/AssetSystem.hpp"
#include "rae/ui/DebugSystem.hpp"
#include "rae/visual/Box.hpp"

#include "rae/ui/UIRenderer.hpp"

using namespace rae;

UIScene::UIScene(
	const String& name,
	const Time& time,
	Input& input,
	ScreenSystem& screenSystem,
	DebugSystem& debugSystem,
	AssetSystem& assetSystem) :
		ISystem(name),
		m_entitySystem("UISystem"),
		m_transformSystem(),
		m_animationSystem(time, m_transformSystem),
		m_selectionSystem(m_transformSystem),
		m_input(input),
		m_screenSystem(screenSystem),
		m_debugSystem(debugSystem),
		m_assetSystem(assetSystem)
{
	addTable(m_windows);
	addTable(m_texts);
	addTable(m_buttons);
	addTable(m_commands);
	addTable(m_colors);
	addTable(m_actives);
	addTable(m_viewports);
	addTable(m_panels);
	addTable(m_keylines);
	addTable(m_keylineLinks);
	addTable(m_stackLayouts);
	addTable(m_imageLinks);
	addTable(m_draggables);

	createDefaultTheme();
}

void UIScene::createDefaultTheme()
{
	m_buttonThemeColors.resize((size_t)ButtonThemeColorKey::Count);
	m_buttonThemeColors[(size_t)ButtonThemeColorKey::Background]		= Color(0.1f, 0.1f, 0.1f, 1.0f);
	m_buttonThemeColors[(size_t)ButtonThemeColorKey::Hover]				= Color(0.3f, 0.4f, 0.45f, 1.0f);
	m_buttonThemeColors[(size_t)ButtonThemeColorKey::Active]			= Color(0.0f, 0.921f, 0.862f, 1.0f);
	m_buttonThemeColors[(size_t)ButtonThemeColorKey::ActiveHover]		= Color(0.619f, 1.0f, 0.976f, 1.0f);

	m_buttonThemeColors[(size_t)ButtonThemeColorKey::Text]				= Color(1.0f, 1.0f, 1.0f, 1.0f);
	m_buttonThemeColors[(size_t)ButtonThemeColorKey::HoverText]			= Color(0.0f, 0.0f, 0.0f, 1.0f);
	m_buttonThemeColors[(size_t)ButtonThemeColorKey::ActiveText]		= Color(0.0f, 0.0f, 0.0f, 1.0f);
	m_buttonThemeColors[(size_t)ButtonThemeColorKey::ActiveHoverText]	= Color(0.0f, 0.0f, 0.0f, 1.0f);

	m_panelThemeColors.resize((size_t)PanelThemeColorKey::Count);
	m_panelThemeColors[(size_t)PanelThemeColorKey::Background]	= Utils::createColor8bit(54, 68, 75, 235);
	m_panelThemeColors[(size_t)PanelThemeColorKey::Hover]		= Utils::createColor8bit(52, 61, 70, 255);

	m_viewportThemeColors.resize((size_t)ViewportThemeColorKey::Count);
	m_viewportThemeColors[(size_t)ViewportThemeColorKey::Line]			= Utils::createColor8bit(64, 64, 64, 255);
	m_viewportThemeColors[(size_t)ViewportThemeColorKey::LineActive]	= Utils::createColor8bit(135, 135, 135, 255);
}

void UIScene::handleInput(const Array<InputEvent>& events)
{
	m_inputState.hadEvents = !events.empty();

	if (!m_inputState.hadEvents)
		return;

	m_inputState.clear();

	if (m_inputState.isGrabbed() && !m_input.mouse.anyButtonDown())
	{
		m_inputState.clearGrab();
	}

	// Copy global mouse button state, because the window state is not always up to date.
	for (int i = 0; i < (int)MouseButton::Count; ++i)
	{
		m_inputState.mouse.buttonState[i] = m_input.mouse.isButtonDown((MouseButton)i);
	}

	m_inputState.handleEvents(events);

	if (!m_inputState.isGrabbed() && !m_inputState.mouseInside)
	{
		// Need to clear hovers, so that none are left behind.
		m_selectionSystem.clearHovers();
	}

	Id hovered = m_selectionSystem.hovered();

	if (hovered != InvalidId)
	{
		if (m_transformSystem.hasLocalTransform(hovered))
		{
			const Box& box = m_transformSystem.getBox(hovered);
			const Transform& transform = m_transformSystem.getLocalTransform(hovered);
			const Pivot& pivot = m_transformSystem.getPivot(hovered);
			Box tbox = box;
			tbox.transform(transform);
			tbox.translatePivot(pivot);

			vec3 mousePositionMM = m_screenSystem.pixelsToMM(m_inputState.mouse.position);

			//LOG_F(INFO, "hovered 3D scene. mouse %f %f, tbox left:%f down:%f width: %f height: %f",
			//	mousePositionMM.x, mousePositionMM.y,
			//	tbox.left(), tbox.down(), tbox.width(), tbox.height());

			m_inputState.mouse.localPositionNormalized = vec3(
				(mousePositionMM.x - tbox.left()) / tbox.width(),
				(mousePositionMM.y - tbox.down()) / tbox.height(), // Ugh, down is the new up. Fix this somehow. Z-up. Something about Y down Y up in UIs and in 3D.
				0.0f);
		}

		if (m_inputState.mouse.buttonClicked[(int)MouseButton::First])
		{
			//LOG_F(INFO, "firstMouseClicked on UIScene name: %s", name().c_str());

			if (m_buttons.check(hovered) && m_commands.check(hovered))
			{
				auto& command = modifyCommand(hovered);
				command.executeAsync();
			}
		}

		if (m_inputState.mouse.buttonClicked[(int)MouseButton::Middle])
		{
			//LOG_F(INFO, "middleMouseClicked on UIScene name: %s", name().c_str());

			if (m_buttons.check(hovered))
			{
				m_selectionSystem.toggleSelected(hovered);
			}
		}

		if (m_inputState.mouse.anyButtonDown())
		{
			m_inputState.grab(hovered);
		}

		// Draggables (this should actually work with selected, not with hovered.)
		if (m_inputState.mouse.isButtonDown(MouseButton::First))
		{
			Array<Id> ids;

			query<Draggable>(m_draggables, [&](Id id)
			{
				bool hovered = m_selectionSystem.isHovered(id);
				if (hovered)
				{
					ids.emplace_back(id);
				}
			});

			if (!ids.empty())
			{
				m_transformSystem.translate(ids, m_screenSystem.pixelsToMM(m_inputState.mouse.delta));
			}
		}

		if (m_viewports.check(hovered))
		{
			Viewport& viewport = modifyViewport(hovered);
			m_eventsForSceneIndex = viewport.sceneIndex;
		}
		else
		{
			m_eventsForSceneIndex = -1;
		}
	}
}

void UIScene::viewportHandleInput(const InputState& inputState)
{

}

UpdateStatus UIScene::update()
{
	static int frameCount = 0;

	query<Command>(m_commands, [&](Id id, Command& command)
	{
		command.update();
	});

	query<UIWidgetUpdater>(m_uiWidgetUpdaters, [&](Id id, UIWidgetUpdater& updater)
	{
		updater.update(id);
	});

	m_animationSystem.update(); // RAE_TODO return value.
	// TransformSystem update is not called for now, as we just call the sync func after any changes...
	// Need to think about this.
	// m_transformSystem.update(); // RAE_TODO return value.
	m_transformSystem.syncLocalAndWorldTransforms();

	doLayout();

	m_transformSystem.syncLocalAndWorldTransforms();

	if (!m_inputState.isGrabbed() && m_inputState.mouseInside && m_inputState.hadEvents)
	{
		hover();
	}

	frameCount++;

	return UpdateStatus::NotChanged;
}

void UIScene::doLayout()
{
	query<StackLayout>(m_stackLayouts, [&](Id layoutId, StackLayout& layout)
	{
		if (m_transformSystem.hasChildren(layoutId))
		{
			auto& children = m_transformSystem.getChildren(layoutId);
			//Array<Id> children(childrenSet.begin(), childrenSet.end());
			//layout.doLayout(children);

			//RAE_CHECK IF NEEDED const vec3& parentPos = m_transformSystem.getWorldPosition(layoutId);
			const Pivot& parentPivot = m_transformSystem.getPivot(layoutId);
			Box parentBox = m_transformSystem.getBox(layoutId);
			parentBox.translatePivot(parentPivot);

			// RAE_TODO Some kind of margin: float marginMM = 6.0f;
			float someIter = /*RAE_CHECK parentPos.y +*/ parentBox.min().y;
			for (auto&& childId : children)
			{
				vec3 pos = m_transformSystem.getLocalPosition(childId);
				const Pivot& pivot = m_transformSystem.getPivot(childId);
				Box tbox = m_transformSystem.getBox(childId);
				tbox.translatePivot(pivot);

				pos.x = (/*RAE_CHECK parentPos.x +*/ parentBox.min().x) - tbox.min().x;// + marginMM;
				pos.y = someIter - tbox.min().y;// + marginMM;
				m_transformSystem.setLocalPosition(childId, pos);
				someIter = someIter + tbox.dimensions().y;
			}
		}

		//RAE_TODO use owner owned? or layoutParent layoutChildren?
		// or some other type of additional hierarchy, so that layout could even be a member of the parent panel here
		// and it would do layout on its siblings.
	});

	query<KeylineLink>(m_keylineLinks, [&](Id id, const KeylineLink& keylineLink)
	{
		if (m_keylines.check(keylineLink.keylineId))
		{
			auto& keyline = getKeyline(keylineLink.keylineId);
			vec3 origPos = m_transformSystem.getWorldPosition(id);

			float posX = m_transformSystem.getBox(m_rootId).dimensions().x * keyline.relativePosition;
			m_transformSystem.setWorldPosition(id, vec3(posX, origPos.y, origPos.z));
		}
	});

	updateMaximizers();
}

void UIScene::hover()
{
	// This is not a good way to hover. We should process the hierarchy instead of a flat list.

	m_selectionSystem.clearHovers();

	Id topMostId = InvalidId;

	query<Box>(m_transformSystem.boxes(), [&](Id id, const Box& box)
	{
		// RAE_TODO: Generic way to disable hit testing. Possibly DisableHover component?
		if (m_panels.check(id) && m_panels.getF(id).visible == false)
		{
			//continue and skip the hit test.
		}
		else
		{
			if (m_transformSystem.hasWorldTransform(id))
			{
				const Transform& transform = m_transformSystem.getWorldTransform(id);
				const Pivot& pivot = m_transformSystem.getPivot(id);
				Box tbox = box;
				tbox.transform(transform);
				tbox.translatePivot(pivot);

				if (tbox.hit(vec2(m_input.mouse.xMM, m_input.mouse.yMM)))
				{
					topMostId = id;
				}
			}
		}
	});

	if (topMostId != InvalidId)
	{
		m_selectionSystem.setHovered(topMostId, true);
	}
}

void UIScene::render2D(NVGcontext* nanoVG, const AssetSystem& assetSystem)
{
	m_nanoVG = nanoVG;

	for (Id id : m_entitySystem.entities())
	{
		if (!m_transformSystem.hasParent(id))
		{
			m_transformSystem.processHierarchy(id, [this](Id id)
			{
				if (m_uiWidgetRenderers.check(id))
				{
					const auto& renderer = m_uiWidgetRenderers.get(id);
					renderer.render(id);
				}
			});
		}
	}

	/*
	// Another previous way to render all UIWidgetRenderers in one go. This won't work
	// because it relies on the children to come after the parents, so the user has to
	// define them in the right order, which is not good.
	query<UIWidgetRenderer>(m_uiWidgetRenderers, [&](Id id, const UIWidgetRenderer& renderer)
	{
		renderer.render(id);
	});
	*/

	/*
	const Color& buttonBackgroundColor = m_buttonThemeColors[(size_t)ButtonThemeColorKey::Background];
	const Color& buttonHoverColor = m_buttonThemeColors[(size_t)ButtonThemeColorKey::Hover];
	const Color& buttonActiveColor = m_buttonThemeColors[(size_t)ButtonThemeColorKey::Active];
	const Color& buttonActiveHoverColor = m_buttonThemeColors[(size_t)ButtonThemeColorKey::ActiveHover];

	const Color& buttonTextColor = m_buttonThemeColors[(size_t)ButtonThemeColorKey::Text];
	const Color& buttonHoverTextColor = m_buttonThemeColors[(size_t)ButtonThemeColorKey::HoverText];
	const Color& buttonActiveTextColor = m_buttonThemeColors[(size_t)ButtonThemeColorKey::ActiveText];
	const Color& buttonActiveHoverTextColor = m_buttonThemeColors[(size_t)ButtonThemeColorKey::ActiveHoverText];

	const Color& viewportLineColor = m_viewportThemeColors[(size_t)ViewportThemeColorKey::Line];
	const Color& viewportLineActiveColor = m_viewportThemeColors[(size_t)ViewportThemeColorKey::LineActive];

	query<Viewport>(m_viewports, [&](Id id, const Viewport& viewport)
	{
		if (m_transformSystem.hasTransform(id) and
			m_transformSystem.hasBox(id))
		{
			const Transform& transform = m_transformSystem.getTransform(id);
			const Box& box = m_transformSystem.getBox(id);
			const Pivot& pivot = m_transformSystem.getPivot(id);

			bool hasColor = m_colors.check(id);
			//bool hovered = m_selectionSystem.isHovered(id);

			renderBorder(transform, box, pivot,
				viewport.active ? viewportLineActiveColor :
				hasColor ? getColor(id) :
				viewportLineColor);
		}
	});

	query<ImageLink>(m_imageLinks, [&](Id id, const ImageLink& imageLink)
	{
		if (m_transformSystem.hasTransform(id) and
			m_transformSystem.hasBox(id))
		{
			const Transform& transform = m_transformSystem.getTransform(id);
			const Box& box = m_transformSystem.getBox(id);
			const Pivot& pivot = m_transformSystem.getPivot(id);

			renderImage(imageLink, transform, box, pivot, assetSystem);
		}
	});

	query<Panel>(m_panels, [&](Id id, const Panel& panel)
	{
		if (m_transformSystem.hasTransform(id) and
			m_transformSystem.hasBox(id))
		{
			const Transform& transform = m_transformSystem.getTransform(id);
			const Box& box = m_transformSystem.getBox(id);
			const Pivot& pivot = m_transformSystem.getPivot(id);

			bool hasColor = m_colors.check(id);
			bool hovered = m_selectionSystem.isHovered(id);

			renderRectangle(transform, box, pivot,
					hovered ? panelHoverColor :
					hasColor ? getColor(id) :
					panelBackgroundColor);
		}
	});

	query<Button>(m_buttons, [&](Id id, const Button& button)
	{
		if (m_transformSystem.hasTransform(id) and
			m_transformSystem.hasBox(id))
		{
			const Transform& transform = m_transformSystem.getTransform(id);
			const Box& box = m_transformSystem.getBox(id);
			const Pivot& pivot = m_transformSystem.getPivot(id);

			bool hasColor = m_colors.check(id);
			bool active = isActive(id) || m_selectionSystem.isSelected(id);
			bool hovered = m_selectionSystem.isHovered(id);
			// More like a debug or editor feature
			//bool selected = m_selectionSystem.isSelected(id);

			renderButton(button.text(), transform, box, pivot,
				(hovered && active ? buttonActiveHoverColor :
					hovered ? buttonHoverColor :
					active ? buttonActiveColor :
					hasColor ? getColor(id) :
					buttonBackgroundColor),
				(hovered && active ? buttonActiveHoverTextColor :
					hovered ? buttonHoverTextColor :
					active ? buttonActiveTextColor :
					buttonTextColor));
		}
	});

	query<Text>(m_texts, [&](Id id, const Text& text)
	{
		if (m_transformSystem.hasTransform(id) and
			m_transformSystem.hasBox(id))
		{
			const Transform& transform = m_transformSystem.getTransform(id);
			const Box& box = m_transformSystem.getBox(id);
			const Pivot& pivot = m_transformSystem.getPivot(id);

			bool hasColor = m_colors.check(id);
			bool active = isActive(id) || m_selectionSystem.isSelected(id);
			bool hovered = m_selectionSystem.isHovered(id);
			// More like a debug or editor feature
			//bool selected = m_selectionSystem.isSelected(id);

			renderText(text.text, transform, box, pivot,
				(hasColor ? getColor(id) : buttonTextColor));
		}
	});
	*/

	if (m_debugSystem.isEnabled())
	{
		// Debug rendering border for WindowEntities.
		query<WindowEntity>(m_windows, [&](Id id)
		{
			if (m_transformSystem.hasWorldTransform(id) and
				m_transformSystem.hasBox(id))
			{
				const Transform& transform = m_transformSystem.getWorldTransform(id);
				const Box& box = m_transformSystem.getBox(id);
				const Pivot& pivot = m_transformSystem.getPivot(id);

				bool hasColor = m_colors.check(id);
				bool hovered = m_selectionSystem.isHovered(id);

				float cornerRadius = 0.0f;
				float thicknessMM = 5.0f;
				renderBorder(transform, box, pivot,
					hovered ? Colors::magenta * 1.5f :
					hasColor ? getColor(id) :
					Colors::magenta * 0.5f,
					cornerRadius, thicknessMM);
			}
		});

		query<Keyline>(m_keylines, [&](Id id, const Keyline& keyline)
		{
			float fromX = m_screenSystem.mmToPixels(
				m_transformSystem.getBox(
					m_rootId).dimensions().x * keyline.relativePosition);
			float toX = fromX;
			float fromY = 0.0f;
			float toY = 800.0f;

			UIRenderer::renderLineNano(m_nanoVG, vec2(fromX, fromY), vec2(toX, toY), Colors::orange);
		});

		// Debug position visualization
		query<Transform>(m_transformSystem.worldTransforms(), [&](Id id, const Transform& transform)
		{
			float diameter = 1.0f;
			renderCircle(transform, diameter, Colors::cyan);
		});

		if (m_inputState.isGrabbed() || m_inputState.mouseInside)
		{
			auto mouseButtonColor = [this]() -> Color
			{
				if (m_input.mouse.isButtonDown(MouseButton::First))
					return Colors::red;
				else if (m_input.mouse.isButtonDown(MouseButton::Second))
					return Colors::green;
				else if (m_input.mouse.isButtonDown(MouseButton::Middle))
					return Colors::blue;
				return Colors::lightGray * Color(1.0f, 1.0f, 1.0f, 0.5f);
			};

			// Render mouse cursor as a circle (actually with arc function)
			renderArc(
				vec2(m_input.mouse.xMM, m_input.mouse.yMM),
				0.0f,				// fromAngleRad
				Math::Tau * 1.0f,	// toAngleRad
				20.0f,				// diameter
				1.0f,				// thickness
				mouseButtonColor());
		}

		// Debug hover visualization
		query<Hover>(m_selectionSystem.hovers(), [&](Id id)
		{
			if (m_transformSystem.hasWorldTransform(id) &&
				m_transformSystem.hasBox(id))
			{
				const Transform& transform = m_transformSystem.getWorldTransform(id);
				const Box& box = m_transformSystem.getBox(id);
				const Pivot& pivot = m_transformSystem.getPivot(id);

				bool hasColor = m_colors.check(id);
				bool hovered = m_selectionSystem.isHovered(id);

				float cornerRadius = 0.0f;
				float thicknessMM = 1.0f;
				renderBorder(transform, box, pivot,
					Colors::magenta, cornerRadius, thicknessMM);
				renderMultilineTextGeneric(m_transformSystem.toString(id), transform, box, pivot, 14.0f,
					Colors::orange, false);
			}
		});
	}
	else
	{
		UIRenderer::renderTextNano(
			m_nanoVG,
			"F1 debug info",
			Rectangle(0.0f, 0.0f, 100.0f, 32.0f),
			16.0f, Colors::white);
	}
}

Rectangle UIScene::convertToPixelRectangle(
	const Transform& transform,
	const Box& box,
	const Pivot& pivot) const
{
	vec3 dimensions = box.dimensions();
	Box pivotedBox = box;
	pivotedBox.translatePivot(pivot);

	return Rectangle(
		m_screenSystem.mmToPixels(transform.position.x + pivotedBox.left()),
		m_screenSystem.mmToPixels(transform.position.y + pivotedBox.down()), // Aaargh, Y down vs Y up issue. That's why this is down. Or should it be up?
		m_screenSystem.mmToPixels(dimensions.x),
		m_screenSystem.mmToPixels(dimensions.y));
}

void UIScene::renderViewportLine(Id id) const
{
	if (!m_viewports.check(id))
		return;

	const Color& viewportLineColor = m_viewportThemeColors[(size_t)ViewportThemeColorKey::Line];
	const Color& viewportLineActiveColor = m_viewportThemeColors[(size_t)ViewportThemeColorKey::LineActive];

	if (m_transformSystem.hasWorldTransform(id) and
		m_transformSystem.hasBox(id))
	{
		const Viewport& viewport = m_viewports.get(id);
		const Transform& transform = m_transformSystem.getWorldTransform(id);
		const Box& box = m_transformSystem.getBox(id);
		const Pivot& pivot = m_transformSystem.getPivot(id);

		bool hasColor = m_colors.check(id);
		//bool hovered = m_selectionSystem.isHovered(id);

		renderBorder(transform, box, pivot,
			viewport.active ? viewportLineActiveColor :
			hasColor ? getColor(id) :
			viewportLineColor);
	}
}

void UIScene::renderPanel(Id id) const
{
	if (!m_panels.check(id) || m_panels.getF(id).visible == false)
		return;

	const Color& panelBackgroundColor = m_panelThemeColors[(size_t)PanelThemeColorKey::Background];
	const Color& panelHoverColor = m_panelThemeColors[(size_t)PanelThemeColorKey::Hover];

	if (m_transformSystem.hasWorldTransform(id) and
		m_transformSystem.hasBox(id))
	{
		const Transform& transform = m_transformSystem.getWorldTransform(id);
		const Box& box = m_transformSystem.getBox(id);
		const Pivot& pivot = m_transformSystem.getPivot(id);

		bool hasColor = m_colors.check(id);
		// Needs to be draggable in order to have hover effect. Possibly there could be a separate
		// Hoverable or HoverEffect component to control this.
		bool hovered = m_selectionSystem.isHovered(id) && m_draggables.check(id);

		renderRectangleGeneric(transform, box, pivot,
				hovered ? panelHoverColor :
				hasColor ? getColor(id) :
				panelBackgroundColor);
	}
}

void UIScene::renderRectangle(Id id) const
{
	if (m_transformSystem.hasWorldTransform(id) and
		m_transformSystem.hasBox(id))
	{
		renderRectangleGeneric(
			m_transformSystem.getWorldTransform(id),
			m_transformSystem.getBox(id),
			m_transformSystem.getPivot(id),
			getColor(id));
	}
}

void UIScene::renderRectangleGeneric(
	const Transform& transform,
	const Box& box,
	const Pivot& pivot,
	const Color& color) const
{
	UIRenderer::renderRectangleNano(m_nanoVG,
		convertToPixelRectangle(transform, box, pivot),
		0.0f, // cornerRadius
		color);
}

void UIScene::renderBorder(const Transform& transform, const Box& box, const Pivot& pivot, const Color& color,
	float cornerRadius, float thickness) const
{
	UIRenderer::renderBorderNano(m_nanoVG,
		convertToPixelRectangle(transform, box, pivot),
		color,
		m_screenSystem.mmToPixels(cornerRadius),
		thickness == 0.0f ? 1.0f : m_screenSystem.mmToPixels(thickness));
}

void UIScene::renderCircle(const Transform& transform, float diameter, const Color& color) const
{
	renderCircle(vec2(transform.position.x, transform.position.y), diameter, color);
}

void UIScene::renderCircle(const vec2& position, float diameter, const Color& color) const
{
	UIRenderer::renderCircleNano(m_nanoVG,
		m_screenSystem.mmToPixels(position),
		m_screenSystem.mmToPixels(diameter),
		color);
}

void UIScene::renderArc(const vec2& origin, float fromAngleRad, float toAngleRad,
	float diameter, float thickness, const Color& color) const
{
	UIRenderer::renderArcNano(m_nanoVG, m_screenSystem.mmToPixels(origin), fromAngleRad, toAngleRad,
		m_screenSystem.mmToPixels(diameter), m_screenSystem.mmToPixels(thickness), color);
}

void UIScene::renderButton(Id id) const
{
	if (!isButton(id))
		return;

	const Color& buttonBackgroundColor = m_buttonThemeColors[(size_t)ButtonThemeColorKey::Background];
	const Color& buttonHoverColor = m_buttonThemeColors[(size_t)ButtonThemeColorKey::Hover];
	const Color& buttonActiveColor = m_buttonThemeColors[(size_t)ButtonThemeColorKey::Active];
	const Color& buttonActiveHoverColor = m_buttonThemeColors[(size_t)ButtonThemeColorKey::ActiveHover];

	const Color& buttonTextColor = m_buttonThemeColors[(size_t)ButtonThemeColorKey::Text];
	const Color& buttonHoverTextColor = m_buttonThemeColors[(size_t)ButtonThemeColorKey::HoverText];
	const Color& buttonActiveTextColor = m_buttonThemeColors[(size_t)ButtonThemeColorKey::ActiveText];
	const Color& buttonActiveHoverTextColor = m_buttonThemeColors[(size_t)ButtonThemeColorKey::ActiveHoverText];

	if (m_transformSystem.hasWorldTransform(id) and
		m_transformSystem.hasBox(id))
	{
		const Button& button = getButton(id);
		const Transform& transform = m_transformSystem.getWorldTransform(id);
		const Box& box = m_transformSystem.getBox(id);
		const Pivot& pivot = m_transformSystem.getPivot(id);

		bool hasColor = m_colors.check(id);
		bool active = isActive(id) || m_selectionSystem.isSelected(id);
		bool hovered = m_selectionSystem.isHovered(id);
		// More like a debug or editor feature
		//bool selected = m_selectionSystem.isSelected(id);

		renderButtonGeneric(button.text(), transform, box, pivot,
			(hovered && active ? buttonActiveHoverColor :
				hovered ? buttonHoverColor :
				active ? buttonActiveColor :
				hasColor ? getColor(id) :
				buttonBackgroundColor),
			(hovered && active ? buttonActiveHoverTextColor :
				hovered ? buttonHoverTextColor :
				active ? buttonActiveTextColor :
				buttonTextColor));
	}
}

void UIScene::renderButtonGeneric(const String& text, const Transform& transform, const Box& box, const Pivot& pivot,
	const Color& color, const Color& textColor) const
{
	UIRenderer::renderButtonNano(m_nanoVG, text,
		convertToPixelRectangle(transform, box, pivot),
		m_screenSystem.mmToPixels(0.46f), // cornerRadius
		color,
		textColor);
}

void UIScene::renderText(Id id) const
{
	if (!m_texts.check(id))
		return;

	if (m_transformSystem.hasWorldTransform(id) and
		m_transformSystem.hasBox(id))
	{
		const Text& text = m_texts.get(id);
		const Transform& transform = m_transformSystem.getWorldTransform(id);
		const Box& box = m_transformSystem.getBox(id);
		const Pivot& pivot = m_transformSystem.getPivot(id);

		bool hasColor = m_colors.check(id);
		bool active = isActive(id) || m_selectionSystem.isSelected(id);
		bool hovered = m_selectionSystem.isHovered(id);
		// More like a debug or editor feature
		//bool selected = m_selectionSystem.isSelected(id);

		renderTextGeneric(text.text, transform, box, pivot, text.fontSize,
			(hasColor ? getColor(id) : Colors::white));
	}
}

void UIScene::renderTextGeneric(const String& text, const Transform& transform, const Box& box,
	const Pivot& pivot, float fontSize, const Color& color) const
{
	UIRenderer::renderTextNano(m_nanoVG, text,
		convertToPixelRectangle(transform, box, pivot),
		fontSize, color);
}

void UIScene::renderMultilineText(Id id) const
{
	if (!m_texts.check(id))
		return;

	if (m_transformSystem.hasWorldTransform(id) and
		m_transformSystem.hasBox(id))
	{
		const Text& text = m_texts.get(id);
		const Transform& transform = m_transformSystem.getWorldTransform(id);
		const Box& box = m_transformSystem.getBox(id);
		const Pivot& pivot = m_transformSystem.getPivot(id);

		bool hasColor = m_colors.check(id);
		bool limitToBoxWidth = true;

		renderMultilineTextGeneric(text.text, transform, box, pivot, text.fontSize,
			(hasColor ? getColor(id) : Colors::white), limitToBoxWidth);
	}
}

void UIScene::renderMultilineTextGeneric(const String& text, const Transform& transform, const Box& box,
	const Pivot& pivot, float fontSize, const Color& color, bool limitToBoxWidth) const
{
	if (limitToBoxWidth)
	{
		UIRenderer::renderMultilineTextNano(m_nanoVG, text,
			convertToPixelRectangle(transform, box, pivot),
			fontSize, color);
	}
	else
	{
		Rectangle rectangle = convertToPixelRectangle(transform, box, pivot);
		// Just make the rectangle bigger than normal screens.
		rectangle.width = 32000.0f;
		UIRenderer::renderMultilineTextNano(m_nanoVG, text,
			rectangle,
			fontSize, color);
	}
}

void UIScene::renderImage(Id id) const
{
	if (!m_imageLinks.check(id))
		return;

	if (m_transformSystem.hasWorldTransform(id) and
		m_transformSystem.hasBox(id))
	{
		const ImageLink& imageLink = m_imageLinks.get(id);
		const Transform& transform = m_transformSystem.getWorldTransform(id);
		const Box& box = m_transformSystem.getBox(id);
		const Pivot& pivot = m_transformSystem.getPivot(id);

		renderImageGeneric(imageLink, transform, box, pivot, m_assetSystem);
	}
}

void UIScene::renderImageGeneric(ImageLink imageLink, const Transform& transform, const Box& box,
	const Pivot& pivot, const AssetSystem& assetSystem) const
{
	const auto& image = assetSystem.getImage(imageLink);

	auto rect = convertToPixelRectangle(transform, box, pivot);

	renderImageNano(m_nanoVG, image.imageId(), rect.x, rect.y, rect.width, rect.height);
}

Id UIScene::connectToWindow(const Window& window)
{
	Id id = m_entitySystem.createEntity();
	m_rootId = id;
	m_windows.assign(id, WindowEntity());

	vec3 position = vec3(0.0f, 0.0f, 0.0f);
	m_transformSystem.addTransform(id, Transform(position));
	updateWindowSize(window);
	m_transformSystem.addPivot(id, Pivot(-1.0f, -1.0f, 0.0f));

	return id;
}

void UIScene::updateWindowSize(const Window& window)
{
	vec3 halfExtents = m_screenSystem.pixelsToMM(vec3(window.width(), window.height(), 1.0f) / 2.0f);
	m_transformSystem.addBox(m_rootId, Box(-(halfExtents), halfExtents));
}

Id UIScene::createButton(const String& text, std::function<void()> handler)
{
	return createButton(text, vec3(), vec3(32.0f, 16.0f, 1.0f), handler);
}

Id UIScene::createButton(const String& text, const Rectangle& rectangle, std::function<void()> handler)
{
	Id button = createButton(
		text,
		vec3(rectangle.x, rectangle.y, 0.0f),
		vec3(rectangle.width, rectangle.height, 1.0f),
		handler);

	m_transformSystem.addPivot(button, Pivots::TopLeft2D);

	return button;
}

Id UIScene::createButton(const String& text, const vec3& position, const vec3& extents, std::function<void()> handler)
{
	Id id = m_entitySystem.createEntity();
	m_transformSystem.addTransform(id, Transform(position));

	vec3 halfExtents = extents / 2.0f;
	m_transformSystem.addBox(id, Box(-(halfExtents), halfExtents));
	addButton(id, Button(text));
	addCommand(id, Command(handler));

	m_uiWidgetRenderers.assign(
		id,
		UIWidgetRenderer(std::bind(&UIScene::renderButton, this, std::placeholders::_1)));

	return id;
}

Id UIScene::createToggleButton(const String& text, const vec3& position, const vec3& extents, Bool& property)
{
	Id id = m_entitySystem.createEntity();
	m_transformSystem.addTransform(id, Transform(position));

	vec3 halfExtents = extents / 2.0f;
	m_transformSystem.addBox(id, Box(-(halfExtents), halfExtents));
	addButton(id, Button(text));

	// When button gets clicked, change property
	addCommand(id, Command([this, &property]()
	{
		property = !property;
		//LOG_F(INFO, "Set property to: %s", Utils::toString(property));
	}));

	bindActive(id, property);

	m_uiWidgetRenderers.assign(
		id,
		UIWidgetRenderer(std::bind(&UIScene::renderButton, this, std::placeholders::_1)));

	return id;
}

void UIScene::bindActive(Id id, Bool& property)
{
	// Set current active
	setActive(id, property);

	// When property changes, change active
	property.onChanged.connect(
	[this, id](bool isEnabled)
	{
		//LOG_F(INFO, "Changed to %s on: %i : %s", Utils::toString(isEnabled), id, getText(id).text);
		setActive(id, isEnabled);
	});
}

void UIScene::connectUpdater(Id id, std::function<void(Id)> updateFunction)
{
	m_uiWidgetUpdaters.assign(
		id,
		UIWidgetUpdater(updateFunction));
}

Id UIScene::createTextBox(const String& text, const vec3& position, const vec3& extents, float fontSize,
	bool multiline)
{
	Id id = m_entitySystem.createEntity();
	m_transformSystem.addTransform(id, Transform(position));
	m_transformSystem.setLocalPosition(id, position);

	vec3 halfExtents = extents / 2.0f;
	m_transformSystem.addBox(id, Box(-(halfExtents), halfExtents));
	addText(id, text, fontSize);

	if (multiline)
	{
		m_uiWidgetRenderers.assign(
			id,
			UIWidgetRenderer(std::bind(&UIScene::renderMultilineText, this, std::placeholders::_1)));
	}
	else
	{
		m_uiWidgetRenderers.assign(
			id,
			UIWidgetRenderer(std::bind(&UIScene::renderText, this, std::placeholders::_1)));
	}

	return id;
}

Id UIScene::createBox(const Rectangle& rectangle, const Color& color)
{
	Id button = createBox(
		vec3(rectangle.x, rectangle.y, 0.0f),
		vec3(rectangle.width, rectangle.height, 1.0f),
		color);
	m_transformSystem.addPivot(button, Pivots::TopLeft2D);
	return button;
}

Id UIScene::createBox(const vec3& position, const vec3& extents, const Color& color)
{
	Id id = m_entitySystem.createEntity();
	m_transformSystem.addTransform(id, Transform(position));

	vec3 halfExtents = extents / 2.0f;
	m_transformSystem.addBox(id, Box(-(halfExtents), halfExtents));
	setColor(id, color);

	m_uiWidgetRenderers.assign(
		id,
		UIWidgetRenderer(std::bind(&UIScene::renderRectangle, this, std::placeholders::_1)));

	return id;
}

Id UIScene::createViewport(int sceneIndex, const vec3& position, const vec3& extents)
{
	Id id = m_entitySystem.createEntity();
	m_transformSystem.addTransform(id, Transform(position));

	vec3 halfExtents = extents / 2.0f;
	m_transformSystem.addBox(id, Box(-(halfExtents), halfExtents));
	addViewport(id, Viewport(sceneIndex));

	m_uiWidgetRenderers.assign(
		id,
		UIWidgetRenderer(std::bind(&UIScene::renderViewportLine, this, std::placeholders::_1)));

	return id;
}

Id UIScene::createAdvancedViewport(int sceneIndex, const vec3& position, const vec3& extents)
{
	Id viewport = createViewport(sceneIndex, position, extents);
	addMaximizerAndButton(viewport);
	return viewport;
}

void UIScene::addViewport(Id id, const Viewport& entity)
{
	m_viewports.assign(id, entity);
}

const Viewport& UIScene::getViewport(Id id) const
{
	return m_viewports.get(id);
}

Viewport& UIScene::modifyViewport(Id id)
{
	return m_viewports.modify(id);
}

Rectangle UIScene::getViewportPixelRectangle(int sceneIndex) const
{
	Rectangle viewportRect;
	query<Viewport>(m_viewports, [&](Id id, const Viewport& viewport)
	{
		if (viewport.sceneIndex == sceneIndex and
			m_transformSystem.hasWorldTransform(id) and
			m_transformSystem.hasBox(id))
		{
			const Transform& transform = m_transformSystem.getWorldTransform(id);
			const Box& box = m_transformSystem.getBox(id);
			const Pivot& pivot = m_transformSystem.getPivot(id);

			viewportRect = convertToPixelRectangle(transform, box, pivot);
		}
	});
	return viewportRect;
}

void UIScene::activateViewportForSceneIndex(int sceneIndex)
{
	query<Viewport>(m_viewports, [&](Id id, Viewport& viewport)
	{
		if (viewport.sceneIndex == sceneIndex)
		{
			viewport.active = true;
		}
		else
		{
			viewport.active = false;
		}
	});
}

Id UIScene::createPanel(const Rectangle& rectangle, bool visible)
{
	Id entity = createPanel(
		vec3(rectangle.x, rectangle.y, 0.0f),
		vec3(rectangle.width, rectangle.height, 1.0f),
		visible);
	m_transformSystem.addPivot(entity, Pivots::TopLeft2D);
	return entity;
}

Id UIScene::createPanel(const vec3& position, const vec3& extents, bool visible)
{
	Id id = m_entitySystem.createEntity();
	m_transformSystem.addTransform(id, Transform(position));

	vec3 halfExtents = extents * 0.5f;
	m_transformSystem.addBox(id, Box(-(halfExtents), halfExtents));
	addPanel(id, Panel(visible));

	m_uiWidgetRenderers.assign(
		id,
		UIWidgetRenderer(std::bind(&UIScene::renderPanel, this, std::placeholders::_1)));

	return id;
}

void UIScene::addPanel(Id id, const Panel& panel)
{
	m_panels.assign(id, panel);
}

const Panel& UIScene::getPanel(Id id)
{
	return m_panels.get(id);
}

void UIScene::addStackLayout(Id id)
{
	m_stackLayouts.assign(id, StackLayout());
}

/*RAE_TODO owner or layoutChildren thing
void UIScene::addToLayout(Id layoutId, Id childId)
{
	if (m_layouts.check(layoutId))
	{
		auto& layout = m_layouts.getF(layoutId);
		if (std::find(layout.children.begin(), layout.children.end(), childId) == layout.children.end())
		{
			layout.children.emplace_back(childId);
		}
	}
}
*/

void UIScene::addMaximizerAndButton(Id id)
{
	addMaximizer(id);

	Id maximizeButton = createButton("+", // Maximize icon: "🗖",
		vec3(0.0f, 35.0f, 0.0f),
		vec3(6.0f, 6.0f, 1.0f),
		[&, id]()
		{
			toggleMaximizer(id);
		});
	m_transformSystem.addChild(id, maximizeButton);
	addStackLayout(id);
}

void UIScene::addMaximizer(Id id)
{
	m_maximizers.assign(id, Maximizer());
}

void UIScene::toggleMaximizer(Id id)
{
	auto& maximizer = m_maximizers.modify(id);
	if (maximizer.maximizerState == MaximizerState::Normal)
	{
		maximizer.storedNormalStatePosition = m_transformSystem.getLocalPosition(id);
		maximizer.storedNormalStateBox = m_transformSystem.getBox(id);
		maximizer.storedNormalStatePivot = m_transformSystem.getPivot(id);
		maximizer.maximizerState = MaximizerState::Maximized;
	}
	else
	{
		maximizer.maximizerState = MaximizerState::Normal;
		m_transformSystem.setBox(id, maximizer.storedNormalStateBox);
		m_transformSystem.setLocalPosition(id, maximizer.storedNormalStatePosition);
		m_transformSystem.setPivot(id, maximizer.storedNormalStatePivot);
	}
}

void UIScene::updateMaximizers()
{
	const Box& window = m_transformSystem.getBox(m_rootId);

	query<Maximizer>(m_maximizers, [&](Id id, const Maximizer& maximizer)
	{
		if (maximizer.maximizerState == MaximizerState::Maximized)
		{
			auto halfExtents = window.dimensions() * 0.5f;
			m_transformSystem.setBox(id, Box(-halfExtents, halfExtents));

			const Pivot& pivot = m_transformSystem.getPivot(id);

			// I guess this could be generalized, but I'm not sure how. Probably something obvious.
			if (pivot == Pivots::Center)
			{
				m_transformSystem.setWorldPosition(id, vec3(halfExtents.x, halfExtents.y, 0.0f));
			}
			else if (pivot == Pivots::TopLeft2D)
			{
				m_transformSystem.setWorldPosition(id, vec3(0.0f, 0.0f, 0.0f));
			}
			else
			{
				m_transformSystem.setWorldPosition(id, vec3(0.0f, 0.0f, 0.0f));
				m_transformSystem.setPivot(id, Pivots::TopLeft2D);
			}
		}
	});
}

Id UIScene::createImageBox(asset::Id imageLink, const vec3& position, const vec3& extents)
{
	Id id = m_entitySystem.createEntity();
	m_transformSystem.addTransform(id, Transform(position));

	vec3 halfExtents = extents / 2.0f;
	m_transformSystem.addBox(id, Box(-(halfExtents), halfExtents));
	addImageLink(id, ImageLink(imageLink));

	m_uiWidgetRenderers.assign(
		id,
		UIWidgetRenderer(std::bind(&UIScene::renderImage, this, std::placeholders::_1)));

	return id;
}

void UIScene::addImageLink(Id id, ImageLink entity)
{
	m_imageLinks.assign(id, entity);
}

const ImageLink& UIScene::getImageLink(Id id)
{
	return m_imageLinks.get(id);
}

Id UIScene::createKeyline(const Keyline& element)
{
	Id id = m_entitySystem.createEntity();
	addKeyline(id, element);
	return id;
}

void UIScene::addKeyline(Id id, const Keyline& element)
{
	m_keylines.assign(id, element);
}

const Keyline& UIScene::getKeyline(Id id)
{
	return m_keylines.get(id);
}

void UIScene::addKeylineLink(Id childId, Id keylineId) //TODO anchor
{
	m_keylineLinks.assign(childId, KeylineLink(keylineId));
}

const KeylineLink& UIScene::getKeylineLink(Id id)
{
	return m_keylineLinks.get(id);
}

void UIScene::addText(Id id, const String& text, float fontSize)
{
	m_texts.assign(id, Text(text, fontSize));
}

void UIScene::addText(Id id, const Text& text)
{
	m_texts.assign(id, text);
}

const Text& UIScene::getText(Id id) const
{
	return m_texts.get(id);
}

Text& UIScene::modifyText(Id id)
{
	return m_texts.modify(id);
}

void UIScene::addButton(Id id, const Button& element)
{
	m_buttons.assign(id, element);
}

const Button& UIScene::getButton(Id id) const
{
	return m_buttons.get(id);
}

bool UIScene::isButton(Id id) const
{
	return m_buttons.check(id);
}

void UIScene::addColor(Id id, const Color& element)
{
	m_colors.assign(id, element);
}

void UIScene::setColor(Id id, const Color& element)
{
	m_colors.assign(id, element);
}

const Color& UIScene::getColor(Id id) const
{
	return m_colors.get(id);
}

void UIScene::addCommand(Id id, const Command& element)
{
	m_commands.assign(id, element);
}

const Command& UIScene::getCommand(Id id) const
{
	return m_commands.get(id);
}

Command& UIScene::modifyCommand(Id id)
{
	return m_commands.modify(id);
}

void UIScene::setActive(Id id, bool active)
{
	m_actives.assign(id, Active(active));
}

bool UIScene::isActive(Id id) const
{
	return m_actives.get(id);
}

void UIScene::addDraggable(Id id)
{
	m_draggables.assign(id, Draggable());
}
