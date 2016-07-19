#include "RayTracer.hpp"

#include <iostream>	
#include <vector>
#include <string>

#include <glm/glm.hpp>
using glm::vec3;
using glm::dot;

#include "core/Utils.hpp"
#include "Random.hpp"

#include "Camera.hpp"
#include "Material.hpp"

using namespace Rae;

bool Sphere::hit(const Ray& ray, float t_min, float t_max, HitRecord& record) const
{
	vec3 oc = ray.origin() - center;
	float a = dot(ray.direction(), ray.direction());
	float b = dot(oc, ray.direction());
	float c = dot(oc, oc) - radius * radius;
	float discriminant = b * b - a * c;
	if (discriminant > 0)
	{
		float temp = (-b - sqrt(discriminant)) / a;
		if (temp < t_max && temp > t_min)
		{
			record.t = temp;
			record.point = ray.point_at_parameter(record.t);
			record.normal = (record.point - center) / radius;
			record.material = material;
			return true;
		}
	}
	return false;
}

Sphere::~Sphere()
{
	delete material; // Hmm. Currently no shared materials... TODO memory management.
}

bool HitableList::hit(const Ray& ray, float t_min, float t_max, HitRecord& record) const
{
	HitRecord tempRecord;
	bool hitAnything = false;
	float closestSoFar = t_max;
	for (size_t i = 0; i < list.size(); ++i)
	{
		if (list[i]->hit(ray, t_min, closestSoFar, tempRecord))
		{
			hitAnything = true;
			closestSoFar = tempRecord.t;
			record = tempRecord;
		}
	}
	return hitAnything;
}

ImageBuffer::ImageBuffer(int setWidth, int setHeight)
: width(setWidth),
height(setHeight),
imageId(-1) // init to invalid value.
{
	color_data.reserve(width * height);
	for (int i = 0; i < width * height; ++i)
	{
		color_data.push_back(vec3(0.5f, 0.5f, 0.5f));
	}	
	data.reserve(width * height * channels);
	for (int i = 0; i < width * height; ++i)
	{
		data.push_back(0);
		data.push_back(0);
		data.push_back(0);
		data.push_back(255);
	}
}

void ImageBuffer::createImage(NVGcontext* vg)
{
	if(imageId == -1 && vg != nullptr)
	{
		//imageId = nvgCreateImage(vg, "/Users/joonaz/Dropbox/taustakuvat/apple_galaxy.jpg", 0);
		imageId = nvgCreateImageRGBA(vg, width, height, /*imageFlags*/0, &data[0]);
	}
}

void ImageBuffer::update8BitImageBuffer(NVGcontext* vg)
{
	// update 8 bit image buffer
	{
		for (int j = 0; j < height; ++j)
		{
			for (int i = 0; i < width; ++i)
			{
				
				vec3 color = color_data[(j*width)+i];

				// Gamma correction done with sqrt only
				data[(j*width*channels) + (i*channels) + 0] = int8_t(255.99 * sqrt(color.r));
				data[(j*width*channels) + (i*channels) + 1] = int8_t(255.99 * sqrt(color.g));
				data[(j*width*channels) + (i*channels) + 2] = int8_t(255.99 * sqrt(color.b));
			}
		}

		nvgUpdateImage(vg, imageId, &data[0]);
	}
}

void ImageBuffer::clear()
{
	std::fill(color_data.begin(), color_data.end(), vec3(0,0,0));
	//std::fill(data.begin(), data.end(), 0);
}

void RayTracer::createSceneOne(HitableList& world, Camera& camera)
{
	camera.setPosition(vec3(0.698890f, 1.275992f, 6.693169f));
	camera.setYaw(Math::toRadians(188.0f));
	camera.setPitch(Math::toRadians(-7.744f));
	camera.setAperture(0.3f);
	camera.setFocusDistance(7.6f);

	world.add(
		new Sphere(vec3(0, 0, -1), 0.5f,
		new Lambertian(vec3(0.8f, 0.3f, 0.3f)))
		);
	world.add(
		new Sphere(vec3(0, -100.5f, -1), 100.0f,
		new Lambertian(vec3(0.0f, 0.7f, 0.8f)))
		);
	
	// Metal balls
	world.add(
		new Sphere(vec3(1, 0, -1), 0.5f,
		new Metal(vec3(0.8f, 0.6f, 0.2f), /*roughness*/0.0f))
		);
	world.add(
		new Sphere(vec3(-0.5f, 0.65f, -1), 0.4f,
		new Metal(vec3(0.8f, 0.4f, 0.8f), /*roughness*/0.3f))
		);
	// Dielectric
	world.add(
		new Sphere(vec3(-1, 0, -1), 0.5f,
		new Dielectric(vec3(0.8f, 0.5f, 0.3f), /*refractive_index*/1.5f))
		);
	world.add(
		new Sphere(vec3(-3.15, 0.1f, -5), 0.6f,
		new Lambertian(vec3(0.05f, 0.2f, 0.8f)))
		);
}

void RayTracer::createSceneFromBook(HitableList& list, Camera& camera)
{
	camera.setPosition(vec3(16.857f, 2.0f, 6.474f));
	camera.setYaw(Math::toRadians(247.8f));
	camera.setPitch(Math::toRadians(-4.762f));
	camera.setAperture(0.1f);
	camera.setFocusDistance(17.29f);

	list.add( new Sphere(vec3(0,-1000,0), 1000, new Lambertian(vec3(0.5, 0.5, 0.5))) );

	for (int a = -11; a < 11; a++)
	{
		for (int b = -11; b < 11; b++)
		{
			float choose_mat = getRandom();
			vec3 center(a + 0.9f * getRandom(), 0.2f, b + 0.9f * getRandom());
			if ((center-vec3(4,0.2,0)).length() > 0.9f)
			{ 
				if (choose_mat < 0.8f)
				{
					// diffuse
					list.add( new Sphere(center, 0.2f, new Lambertian(vec3( getRandom()*getRandom(), getRandom()*getRandom(), getRandom()*getRandom()))));
				}
				else if (choose_mat < 0.95f)
				{
					// metal
					list.add( new Sphere(center, 0.2f,
							new Metal(vec3(0.5f*(1.0f + getRandom()), 0.5f*(1.0f + getRandom()), 0.5f*(1.0f + getRandom())), /*roughness*/ 0.5f*getRandom())));
				}
				else
				{
					// glass
					list.add( new Sphere(center, 0.2f, new Dielectric(vec3(0.8f, 0.5f, 0.3f), /*refractive_index*/1.5f)) );
				}
			}
		}
	}

	list.add( new Sphere(vec3(0, 1, 0), 1.0, new Dielectric(vec3(0.8f, 0.5f, 0.3f), /*refractive_index*/1.5f)) );
	list.add( new Sphere(vec3(-4, 1, 0), 1.0, new Lambertian(vec3(0.0, 0.2, 0.9))) );
	list.add( new Sphere(vec3(4, 1, 0), 1.0, new Metal(vec3(0.7, 0.6, 0.5), 0.0)) );
}

void RayTracer::showScene(int number)
{
	if (number == 1)
	{
		clearScene();
		createSceneOne(m_world, m_camera);
	}

	if (number == 2)
	{
		clearScene();
		createSceneFromBook(m_world, m_camera);
	}
}

void RayTracer::clearScene()
{
	m_world.clear();
	m_camera.setNeedsUpdate();
	clear();
}

void RayTracer::clear()
{
	buffer->clear();
	currentSample = 0;
	totalRayTracingTime = -1.0;
	startTime = -1.0f;
}

RayTracer::RayTracer(Camera& setCamera)
: m_world(4),
m_camera(setCamera),
buffer(&smallBuffer)
{	
	createSceneOne(m_world, m_camera);
	//createSceneFromBook(m_world);
}

void RayTracer::setNanovgContext(NVGcontext* set_vg)
{
	vg = set_vg;

	smallBuffer.createImage(vg);
	bigBuffer.createImage(vg);
}

std::string toString(const HitRecord& record)
{
	return "t: " + std::to_string(record.t) + ", "
		+ "point: (" + std::to_string(record.point.x) + ", "
		+ std::to_string(record.point.y) + ", "
		+ std::to_string(record.point.z) + ")";
}

void RayTracer::autoFocus()
{
	// Get a ray to middle of the screen and focus there
	Ray ray = m_camera.getExactRay(0.5f, 0.5f);
	HitRecord record;
	if (m_world.hit(ray, 0.001f, FLT_MAX, record))
	{
		debugHitRecord = record;
		m_camera.animateFocusPosition(record.point, m_camera.focusSpeed());
	}
}

vec3 RayTracer::rayTrace(const Ray& ray, Hitable& world, int depth)
{
	HitRecord record;
	if (world.hit(ray, 0.001f, rayMaxLength(), record))
	{
		// Visualize focus distance with a line
		if (m_isVisualizeFocusDistance)
		{
			float hitDistance = glm::length(record.point - m_camera.position());
			if (Utils::isEqual(m_camera.focusDistance(), hitDistance, 0.01f) == true)
			{
				return vec3(0,1,1); // cyan line
			}
		}

		// Normal raytracing
		if (isFastMode() == false)
		{
			Ray scattered;
			vec3 attenuation;

			if (depth < 50 && record.material->scatter(ray, record, attenuation, scattered))
			{
				return attenuation * rayTrace(scattered, world, depth + 1);
			}
			else
			{
				return vec3();
			}
		}
		else // FastMode returns just the material color
		{
			return record.material->albedo;
		}
	}
	return sky(ray);
}

vec3 RayTracer::sky(const Ray& ray)
{
	vec3 unitDirection = glm::normalize( ray.direction() );
	float t = 0.5f * (unitDirection.y + 1.0f);
	return (1.0f - t) * vec3(0.3f, 0.4f, 1.0f) + t * vec3(0.7f, 0.8f, 1.0f);
}

//#define RENDER_ALL_AT_ONCE

void RayTracer::update(double time, double delta_time)
{
	/*
	Old time based switch buffers system:

	if (switchTime > 0.0f)
	{
		if (buffer != &bigBuffer
			&&
			(total_ray_tracing_time >= switchTime
			||
			current_sample >= samples_limit))
		{
			buffer = &bigBuffer;
			buffer->clear();
			current_sample = 0;
		}
	}
	*/

	if (startTime == -1.0f)
		startTime = time;

	if (totalRayTracingTime == -1.0f)
		totalRayTracingTime = time;

	#ifdef RENDER_ALL_AT_ONCE
		renderAllAtOnce(time);
	#else
		renderSamples(time, delta_time);
		if (currentSample <= samplesLimit) // do once more than render
		{
			updateImageBuffer();
		}
	#endif
	
}

void RayTracer::toggleBufferQuality()
{
	if (buffer == &smallBuffer)
	{
		buffer = &bigBuffer;
	}
	else buffer = &smallBuffer;
	clear();
}

float RayTracer::rayMaxLength()
{
	if (isFastMode() == false)
		return FLT_MAX;
	else return 5.0f;
}

void RayTracer::renderAllAtOnce(double time)
{
	// timings for 100 samples at 500x250:
	// 14.791584 s
	// 14.715808 s
	// 14.710577 s

	if (currentSample < samplesLimit)
	{
		startTime = time;

		for (int j = 0; j < buffer->height; ++j)
		{
			for (int i = 0; i < buffer->width; ++i)
			{
				vec3 color;

				for (int sample = 0; sample < samplesLimit; sample++)
				{
					float u = float(i + drand48()) / float(buffer->width);
					float v = float(j + drand48()) / float(buffer->height);
					
					Ray ray = m_camera.getRay(u, v);
					color += rayTrace(ray, m_world, 0);
				}

				color /= float(samplesLimit);

				buffer->color_data[(j*buffer->width)+i] = color;
			}
		}
		
		currentSample = samplesLimit;
	}
	else if (currentSample == samplesLimit)
	{
		updateImageBuffer();

		// do only once:
		totalRayTracingTime = time - startTime;

		currentSample++;
	}
}

void RayTracer::renderSamples(double time, double delta_time)
{
	// timings for 100 samples at 500x250:
	// 15.426324 s
	// 15.402015 s
	// 15.347182 s

	if (currentSample < samplesLimit)
	{
		totalRayTracingTime = time - startTime;

		for (int j = 0; j < buffer->height; ++j)
		{
			for (int i = 0; i < buffer->width; ++i)
			{
				float u = float(i + drand48()) / float(buffer->width);
				float v = float(j + drand48()) / float(buffer->height);

				Ray ray = m_camera.getRay(u, v);
				vec3 color = rayTrace(ray, m_world, 0);

				//http://stackoverflow.com/questions/22999487/update-the-average-of-a-continuous-sequence-of-numbers-in-constant-time
				// add to average
				buffer->color_data[(j*buffer->width)+i]
					= (float(currentSample) * buffer->color_data[(j*buffer->width)+i] + color) / float(currentSample + 1);
			}
		}
		
		currentSample++;
	}
}

void RayTracer::updateImageBuffer()
{
	buffer->update8BitImageBuffer(vg);
}

void RayTracer::renderNanoVG(NVGcontext* vg,  float x, float y, float w, float h)
{	
	nvgSave(vg);

	//override the given parameters and reuse w and h...
	/*
	x = -g_rae->screenHalfWidthP();
	y = -g_rae->screenHalfHeightP();
	w = g_rae->screenWidthP();
	h = g_rae->screenHeightP();
	*/

	//imgPaint = nvgImagePattern(vg, 0, 0, winWidth, winHeight, 0.0f, backgroundImage, (sinf(t*0.25f)+1.0f)*0.5f);
	imgPaint = nvgImagePattern(vg, x, y, w, h, 0.0f, buffer->imageId, 1.0f);
	nvgBeginPath(vg);
	nvgRect(vg, x, y, w, h);
	nvgFillPaint(vg, imgPaint);
	nvgFill(vg);

	// Text
	if (m_isInfoText)
	{
		nvgFontFace(vg, "sans");

		nvgFontSize(vg, 18.0f);
		nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
		nvgFillColor(vg, nvgRGBA(128, 128, 128, 192));
	
		float vertPos = 200.0f;

		std::string samples_str = "Samples: " + std::to_string(currentSample);
		nvgText(vg, 10.0f, vertPos, samples_str.c_str(), nullptr); vertPos += 20.0f;

		std::string samples_limit_str = "/" + std::to_string(samplesLimit);
		nvgText(vg, 10.0f, vertPos, samples_limit_str.c_str(), nullptr); vertPos += 20.0f;

		std::string total_time_str = "Time: " + std::to_string(totalRayTracingTime) + " s";
		nvgText(vg, 10.0f, vertPos, total_time_str.c_str(), nullptr); vertPos += 20.0f;

		std::string positionStr = "Position: "
			+ std::to_string(m_camera.position().x) + ", "
			+ std::to_string(m_camera.position().y) + ", "
			+ std::to_string(m_camera.position().z);
		nvgText(vg, 10.0f, vertPos, positionStr.c_str(), nullptr); vertPos += 20.0f;

		std::string yawStr = "Yaw: "
			+ std::to_string(Math::toDegrees(m_camera.yaw())) + "°"
			+ " Pitch: "
			+ std::to_string(Math::toDegrees(m_camera.pitch())) + "°";
		nvgText(vg, 10.0f, vertPos, yawStr.c_str(), nullptr); vertPos += 20.0f;

		std::string fovStr = "Field of View: "
			+ std::to_string(Math::toDegrees(m_camera.fieldOfView())) + "°";
		nvgText(vg, 10.0f, vertPos, fovStr.c_str(), nullptr); vertPos += 20.0f;

		std::string focusDistanceStr = "Focus distance: "
			+ std::to_string(m_camera.focusDistance());
		nvgText(vg, 10.0f, vertPos, focusDistanceStr.c_str(), nullptr); vertPos += 20.0f;

		nvgText(vg, 10.0f, vertPos, m_camera.isContinuousAutoFocus() ? "Autofocus ON" : "Autofocus OFF", nullptr);
		vertPos += 20.0f;

		std::string apertureStr = "Aperture: "
			+ std::to_string(m_camera.aperture());
		nvgText(vg, 10.0f, vertPos, apertureStr.c_str(), nullptr); vertPos += 20.0f;

		std::string debugStr = "Debug hit pos: "
			+ std::to_string(debugHitRecord.point.x) + ", "
			+ std::to_string(debugHitRecord.point.y) + ", "
			+ std::to_string(debugHitRecord.point.z);
		nvgText(vg, 10.0f, vertPos, debugStr.c_str(), nullptr); vertPos += 20.0f;

		vec3 focusPos = m_camera.getFocusPosition();
		std::string debugStr2 = "Debug focus pos: "
			+ std::to_string(focusPos.x) + ", "
			+ std::to_string(focusPos.y) + ", "
			+ std::to_string(focusPos.z);
		nvgText(vg, 10.0f, vertPos, debugStr2.c_str(), nullptr); vertPos += 20.0f;
	}

	nvgRestore(vg);	
}
