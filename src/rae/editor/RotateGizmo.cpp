#include "rae/editor/RotateGizmo.hpp"
#include "rae/visual/ShapeRenderer.hpp"
#include "rae/visual/Camera.hpp"
#include "rae/visual/Ray.hpp"
#include "rae/visual/Plane.hpp" // Circle
#include "rae/visual/RenderSystem.hpp"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

using namespace rae;

bool RotateGizmo::hover(const Ray& mouseRay, const Camera& camera)
{
	float gizmoCameraFactor = camera.screenSizeFactor(m_position) * m_gizmoSizeMultiplier;
	float radius = m_rotateGizmoRadius * gizmoCameraFactor;

	bool gotBestAxisHover = false;
	AxisHover bestAxisHover;
	vec3 bestIntersectionPosition = vec3(0.0f);
	qua bestTangentRotation;

	clearHovers();

	for (int i = 0; i < 3; ++i)
	{
		//RAE_TODO: if (!m_visibleAxes[i])
			//continue;

		Axis axis = Axis(i);

		qua diskRotation = m_rotation * axisRotation(axis);
		vec3 diskPosition = m_position;
		vec3 diskNormal = diskRotation * axisVector(Axis::X);

		auto circle = Circle(diskPosition, diskNormal, radius);
		vec3 closestPointOnEdge = circle.closestPointToEdge(camera.position());
		vec3 closestPointDirection = glm::normalize(closestPointOnEdge - diskPosition);

		vec3 diskForwardVec = diskRotation * axisVector(Axis::Z);
		float closestPointAngle = Math::signedAngle(diskForwardVec, closestPointDirection,
			diskNormal);

		float angleStart = Math::toRadians(-90.0f) + closestPointAngle;
		float angleEnd = Math::toRadians(90.0f) + closestPointAngle;

		float angle = (angleEnd - angleStart);

		const int pointCount = m_rotateGizmoHoverResolution + 1;
		Array<vec3> arcPoints;
		arcPoints.reserve(pointCount);
		for (int p = 0; p < pointCount; ++p)
		{
			float angleStep = angle * (float(p) / float(pointCount - 1));
			// Notice that we invert Y here
			vec3 direction = vec3(0.0f, -sinf(angleStep + angleStart), cosf(angleStep + angleStart));
			arcPoints.emplace_back(diskPosition + (diskRotation * (radius * direction)));
		}

		for (int p = 0; p < (int)(arcPoints.size())-1; ++p)
		{
			auto capsule = Capsule(arcPoints[p], arcPoints[p+1],
				m_rotateGizmoHandleThickness * m_proximityMultiplier * gizmoCameraFactor);

			bool intersects = capsule.intersects(mouseRay);
			if (intersects)
			{
				float distanceToRayOrigin = glm::length(capsule.center() - mouseRay.origin());
				AxisHover axisHover = AxisHover(axis, distanceToRayOrigin);
				if (!gotBestAxisHover || bestAxisHover.distance > axisHover.distance)
				{
					gotBestAxisHover = true;
					bestAxisHover = axisHover;
					bestIntersectionPosition = capsule.center();
					bestTangentRotation = Math::rotationOnto(
						axisVector(Axis::X), capsule.upDirection());
				}
			}

			// Debug visualization of the hover capsules
			if (m_debugRotateGizmoHoverAxis == 0 ||
				m_debugRotateGizmoHoverAxis-1 == int(axis))
			{
				Color color = intersects ? Colors::white : axisColor(axis);
				g_shapeRenderer->drawLineSegment((LineSegment)capsule, color);

				g_shapeRenderer->drawCylinder(
					capsule.center(),
					vec3(capsule.lineLength() * 0.5f, capsule.radius(), capsule.radius()),
					Math::rotationOnto(axisVector(Axis::X), capsule.upDirection()),
					color);
			}
		}
	}

	if (gotBestAxisHover)
	{
		setHovered(bestAxisHover.axis);
		m_tangentRelativePosition = bestIntersectionPosition - m_position;
		m_tangentRotation = bestTangentRotation;

		qua diskRotation = m_rotation * axisRotation(bestAxisHover.axis);
		vec3 diskNormal = glm::normalize(diskRotation * axisVector(Axis::X));
		vec3 diskForwardVec = glm::normalize(diskRotation * axisVector(Axis::Z));
		vec3 tangentPositionDirection = glm::normalize(m_tangentRelativePosition);
		m_tangentAngle = Math::signedAngle(diskForwardVec,
			tangentPositionDirection,
			diskNormal);
		m_originalActiveDiskRotation = diskRotation;

		return true;
	}

	return false;
}

void RotateGizmo::render3D(const Camera& camera, ShapeRenderer& shapeRenderer /*, const vec3& gizmoPosition, RenderSystem& renderSystem*/) const
{
	float gizmoCameraFactor = camera.screenSizeFactor(m_position) * m_gizmoSizeMultiplier;
	float radius = m_rotateGizmoRadius * gizmoCameraFactor;

	qua rotationTowardsCamera = Math::rotationOnto(
		axisVector(Axis::X),
		m_position - camera.position());

	int resolution = 128;

	// Draw a screen space circle to outline the gizmo sphere.
	shapeRenderer.drawCircle(m_position, radius, rotationTowardsCamera,
		Colors::gray, resolution);

	const auto hoverColor = Utils::createColor8bit(255, 165, 0);
	const auto activeColor = Utils::createColor8bit(255, 215, 0);

	for (int i = (int)Axis::X; i < (int)Axis::Count; ++i)
	{
		Axis axis = Axis(i);

		Color color = m_axisActives[i] ? activeColor : (m_axisHovers[i] ? hoverColor : axisColor(axis));

		qua diskRotation = m_rotation * axisRotation(axis);
		vec3 diskPosition = m_position; // RAE_TODO: gizmoPosition;
		vec3 diskNormal = diskRotation * axisVector(Axis::X);

		auto circle = Circle(diskPosition, diskNormal, radius);
		vec3 closestPointOnEdge = circle.closestPointToEdge(camera.position());
		vec3 closestPointDirection = glm::normalize(closestPointOnEdge - diskPosition);

		vec3 diskForwardVec = diskRotation * axisVector(Axis::Z);
		float closestPointAngle = Math::signedAngle(diskForwardVec, closestPointDirection, diskNormal);

		float angleStart = Math::toRadians(-90.0f) + closestPointAngle;
		float angleEnd = Math::toRadians(90.0f) + closestPointAngle;

		// RAE_TODO: Thickness with something like: m_rotateGizmoHandleThickness * gizmoCameraFactor
		shapeRenderer.drawArch(diskPosition, angleStart, angleEnd,
			radius, diskRotation, color, 32);
	}

	vec3 gizmoPosition = m_position;

	// Additional visualization like tangent arrows and accumulated rotation.
	for (int i = 0; i < 3; ++i)
	{
		Axis axis = static_cast<Axis>(i);

		if (isActiveAxis(axis))
		{
			// Tangent visualization
			//const Color TangentArrowColor = Colors::cyan;
			//Color tangentPassiveColor = TangentArrowColor;
			//RAE_TODO: Ideally with alpha: tangentPassiveColor.a = 0.2f;

			// Instead we'll just use two very different colors.
			const Color TangentArrowColor = Colors::magenta;
			const Color tangentPassiveColor = Colors::cyan;

			vec3 tangentPosition = getTangentPosition(gizmoPosition);
			vec3 tangentArrowDirection = (m_tangentRotation * vec3(1,0,0))
				* m_rotateGizmoTangentArrowLength * gizmoCameraFactor;
			vec3 tangentArrowEndPointA =  tangentArrowDirection + tangentPosition;
			vec3 tangentArrowEndPointB = -tangentArrowDirection + tangentPosition;

			auto drawArrow = [](ShapeRenderer& shapeRenderer,
				const vec3& startPoint, const vec3& endPoint, float thickness,
				const Color& color, float tipSize, float tipWidthMultiplier)
			{
				shapeRenderer.drawLine({startPoint, endPoint}, color);
				/*shapeRenderer.drawPyramid(endPoint,
					vec3(tipSize * tipWidthMultiplier, tipSize, tipSize * tipWidthMultiplier),
					Math::rotationOnto(vec3(0,1,0), endPoint - startPoint),
					color);
					*/
			};

			float tangentArrowThickness = 0.002f * gizmoCameraFactor;
			float tangentArrowTipSize = m_rotateGizmoTangentArrowTipSize * gizmoCameraFactor;
			drawArrow(shapeRenderer, tangentPosition, tangentArrowEndPointA, tangentArrowThickness,
				m_accumulatedAngles[i] >= 0.0f ? TangentArrowColor : tangentPassiveColor,
				tangentArrowTipSize, m_rotateGizmoTangentArrowTipWidthMultiplier);
			drawArrow(shapeRenderer, tangentPosition, tangentArrowEndPointB, tangentArrowThickness,
				m_accumulatedAngles[i] <= 0.0f ? TangentArrowColor : tangentPassiveColor,
				tangentArrowTipSize, m_rotateGizmoTangentArrowTipWidthMultiplier);

			// Rotated and accumulated angle visualization
			float angleVisualizationRadius = radius * 0.9f;
			const Color AccumulatedAnglesColor = Color(0.0f, 1.0f, 1.0f, 0.4f);
			const Color RotatedAnglesColor = Color(1.0f, 0.647f, 0.0f, 0.6f);

			// RAE_TODO: Draw these with filled geometry fans:
			shapeRenderer.drawArch(gizmoPosition,
				m_tangentAngle + m_rotatedAngles[i],
				m_tangentAngle + m_accumulatedAngles[i],
				angleVisualizationRadius, m_originalActiveDiskRotation, AccumulatedAnglesColor, 32, true);
			shapeRenderer.drawArch(gizmoPosition,
				m_tangentAngle,
				m_tangentAngle + m_rotatedAngles[i],
				angleVisualizationRadius, m_originalActiveDiskRotation, RotatedAnglesColor, 64, true);

			/* RAE_TODO text info about angles:
			String accumulatedString = printf("[%s: %.2f]", getAxisString(axis).c_str(),
				m_rotatedAngles[i] * Math::RadToDeg);

			drawGizmoText(accumulatedString, camera, gizmoPosition);
			*/
		}
	}
}

qua RotateGizmo::getRotateAxisDelta(
	const vec2& mouseDelta,
	const Camera& camera,
	const vec3& gizmoPosition, // RAE_TODO pass the position from outside for quicktransform modifiers.
	bool snapEnabled,
	float snapAngleStep,
	bool precisionModifier)
{
	float gizmoCameraFactor = camera.screenSizeFactor(m_position) * m_gizmoSizeMultiplier;

	vec2 inputDelta = mouseDelta * m_rotateGizmoSpeedMul
		* (precisionModifier ? m_rotateGizmoPrecisionMul : 1.0f)
		* gizmoCameraFactor;
	// Invert mouse Y
	inputDelta = vec2(inputDelta.x, -inputDelta.y);

	std::array<float, 3> angles = { 0.0f, 0.0f, 0.0f };
	qua rotateAxisDelta = qua();

	vec3 position = m_position;

	for (int i = 0; i < 3; ++i)
	{
		if (!m_axisActives[i])
		{
			continue;
		}

		Axis axis = Axis(i);

		vec3 axisDirection = m_rotation * axisVector(axis);
		qua diskRotation = m_rotation * axisRotation(axis);
		vec3 diskNormal = glm::normalize(diskRotation * axisVector(Axis::X));

		float radius = 2.0f;
		auto circle = Circle(position, axisDirection, radius);

		vec3 tangentPosition = getTangentPosition(position);

		vec3 mouseWorldPos = tangentPosition + (camera.right() * inputDelta.x) + (camera.up() * inputDelta.y);

		vec3 deltaPosOnEdge = circle.closestPointToEdge(mouseWorldPos);

		vec3 tangentAngleVector = glm::normalize(tangentPosition - position);
		vec3 currentAngleVector = glm::normalize(deltaPosOnEdge - position);

		rotateAxisDelta = Math::rotationOntoNormalized(tangentAngleVector, currentAngleVector);

		float angle = Math::signedAngle(tangentAngleVector, currentAngleVector, diskNormal);
		float finalAngle = angle;

		if (snapEnabled)
		{
			float currentTargetAngle = m_accumulatedAngles[i] + angle;
			float snappedTargetAngle = Math::snapValue(currentTargetAngle, snapAngleStep);

			// Sync our rotation to reach the snapped target.
			finalAngle = snappedTargetAngle - m_rotatedAngles[i];

			// Override rotateAxisDelta when snapping
			rotateAxisDelta = glm::angleAxis(finalAngle, diskNormal);

			m_rotatedAngles[i] += finalAngle;
			m_accumulatedAngles[i] += angle;
		}
		else
		{
			m_rotatedAngles[i] += finalAngle;
			// When not snapping, accumulatedAngles should be always the same as rotatedAngles.
			m_accumulatedAngles[i] = m_rotatedAngles[i];
		}
		
		angles[i] = finalAngle;
	}

	/*RAE_TODO
	if (actionAxis == ActionAxis::Screen)
	{
		if (m_activeAxes[0])
			return glm::angleAxis(angles[0], camera.right());
		else if (m_activeAxes[1])
			return glm::angleAxis(angles[1], camera.up());
		else if (m_activeAxes[2])
			return glm::angleAxis(angles[2], camera.front());
	}
	else if (actionAxis == ActionAxis::Workplane)
	{
		// Use the actual rotation of our gizmo
		return rotateAxisDelta;
	}
	*/

	// ActionAxis::World and ActionAxis::Local use just the axis angles.
	vec3 euler = vec3(angles[0], angles[1], angles[2]);
	return qua(euler);
}
