#ifdef USE_RAE_AV
#include <cmath>
#include <chrono>
#include <array>

#include "rae/core/Utils.hpp"
#include "rae/core/Types.hpp"

#include "rae_av/fx/OpticalFlow.hpp"

using namespace cv;
using namespace cv::cuda;
using namespace cv::optflow;
using namespace rae;
using namespace rae::av;

OpticalFlow::OpticalFlow(OpticalFlowMethod method) :
	m_opticalFlowMethod(method)
{
	//"/Users/joonaz/Documents/jonas/opencv-3.2.0/samples/data/basketball2.png"

	//String filename1 = "/Users/joonaz/Documents/jonas/hdr_testi_matskut2017/basketball1.png";
	//String filename2 = "/Users/joonaz/Documents/jonas/hdr_testi_matskut2017/basketball2.png";

	/*String filename1 = "/Users/joonaz/Documents/jonas/hdr_testi_matskut2017/0001.png";
	String filename2 = "/Users/joonaz/Documents/jonas/hdr_testi_matskut2017/0003.png";

	//m_frame0 = imread(filename1, IMREAD_GRAYSCALE);
	//m_frame1 = cv::imread(filename1, cv::IMREAD_GRAYSCALE);

	m_frame0 = cv::imread(filename1, cv::IMREAD_COLOR);
	m_frame1 = cv::imread(filename2, cv::IMREAD_COLOR);

	if (m_frame0.empty())
	{
		std::cerr << "Can't open image ["  << filename1 << "]\n";
		m_error = true;
		return;
	}

	if (m_frame1.empty())
	{
		std::cerr << "Can't open image ["  << filename2 << "]\n";
		m_error = true;
		return;
	}

	if (m_frame1.size() != m_frame0.size())
	{
		std::cerr << "OpticalFlow: Images should be of equal sizes\n";
		m_error = true;
		return;
	}

	LOG_F(INFO, "Loaded OpticalFlow example files. width: %i height: %i",
		m_frame0.cols, m_frame0.rows);
	*/
}

std::string humanTimeString(unsigned long long int timestamp)
{
	long milliseconds   = (long) (timestamp / 1000) % 1000;
	long seconds    = (((long) (timestamp / 1000) - milliseconds)/1000)%60;
	long minutes    = (((((long) (timestamp / 1000) - milliseconds)/1000) - seconds)/60) %60;
	long hours      = ((((((long) (timestamp / 1000) - milliseconds)/1000) - seconds)/60) - minutes)/60;

	return std::to_string(hours) + " h "
		+ std::to_string(minutes) + " min "
		+ std::to_string(seconds) + " s "
		+ std::to_string(milliseconds) + " ms";
} 

void OpticalFlow::process()
{
	if (m_state != EffectNodeState::Processing || m_error)
		return;

	LOG_F(INFO, "OpticalFlow processing.");

	Mat greyFrame0;
	cvtColor(m_frame0, greyFrame0, COLOR_BGR2GRAY);
	Mat greyFrame1;
	cvtColor(m_frame1, greyFrame1, COLOR_BGR2GRAY);

	std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();

	if (m_opticalFlowMethod == OpticalFlowMethod::DualTVL1)
	{
		LOG_F(INFO, ".");
		Ptr<DualTVL1OpticalFlow> optFlow = DualTVL1OpticalFlow::create();
		LOG_F(INFO, "..");
		optFlow->calc(greyFrame0, greyFrame1, m_flowForward);
		LOG_F(INFO, "...");
		optFlow->calc(greyFrame1, greyFrame0, m_flowBackward); // backward flow
		LOG_F(INFO, "...");
	}
	else if (m_opticalFlowMethod == OpticalFlowMethod::DeepFlow)
	{
		LOG_F(INFO, ".");
		Ptr<cv::DenseOpticalFlow> optFlow = optflow::createOptFlow_DeepFlow();
		LOG_F(INFO, "..");
		optFlow->calc(greyFrame0, greyFrame1, m_flowForward);
		LOG_F(INFO, "...");
		optFlow->calc(greyFrame1, greyFrame0, m_flowBackward); // backward flow
		LOG_F(INFO, "...");
	}
	else if (m_opticalFlowMethod == OpticalFlowMethod::Farneback)
	{
		calcOpticalFlowFarneback(greyFrame0, greyFrame1, m_flowForward, 0.5, 3, 15, 3, 5, 1.2, 0);
		calcOpticalFlowFarneback(greyFrame1, greyFrame0, m_flowBackward, 0.5, 3, 15, 3, 5, 1.2, 0);
	}
	#if 0
	else if (m_opticalFlowMethod == OpticalFlowMethod::CudaBrox)
	{
		GpuMat d_frame0(greyFrame0);
		GpuMat d_frame1(greyFrame1);
		GpuMat d_flow(greyFrame0.size(), CV_32FC2);

		Ptr<cuda::BroxOpticalFlow> brox = cuda::BroxOpticalFlow::create(0.197f, 50.0f, 0.8f, 10, 77, 10);

		GpuMat d_frame0f;
		GpuMat d_frame1f;

		d_frame0.convertTo(d_frame0f, CV_32F, 1.0 / 255.0);
		d_frame1.convertTo(d_frame1f, CV_32F, 1.0 / 255.0);

		brox->calc(d_frame0f, d_frame1f, d_flow);

		d_flow.upload(m_flowForward);
		d_flow.upload(m_flowBackward);
	}
	#endif

	std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();

	String timeString = humanTimeString(
		std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count());
	LOG_F(INFO, "OpticalFlow took: %s", timeString.c_str());
}

inline bool isFlowCorrect(Point2f u)
{
	return !cvIsNaN(u.x) && !cvIsNaN(u.y) && fabs(u.x) < 1e9 && fabs(u.y) < 1e9;
}

void OpticalFlow::update()
{
	if (m_error)
	{
		LOG_F(ERROR, "OpticalFlow got some error.");
		return;
	}

	static int doneCounter = 0;

	switch(getState())
	{
		case EffectNodeState::Nothing:
		case EffectNodeState::WaitingForData:
		return;
		case EffectNodeState::Processing:
		{
			process(); // how to make this into async task?
			setState(EffectNodeState::Done);
		}
		break;
		case EffectNodeState::Done:
		{
			//float lerpValue = fabs((fmod(time, m_duration) / (m_duration/2.0f)) - 1.0f);
			//LOG_F(INFO, "time: %f lerpValue: %f", time, lerpValue);

			int writeFrames = 4;
			float lerpValue = (float)doneCounter / (float)writeFrames;
			LOG_F(INFO, "lerpValue: %f", lerpValue);
			LOG_F(INFO, "doneCounter: %i frameCount: %i", doneCounter, frameCount);

			if (m_frame0.size() != m_frame1.size() ||
				m_frame0.size() != m_flowForward.size() ||
				m_frame0.size() != m_flowBackward.size())
				return;

			auto remapWithFLow = [](const Mat& input, Mat& output, const Mat_<Point2f>& flow,
				float lerpValue)
			{
				Mat interpMap(flow.size(), flow.type());
				for (int y = 0; y < interpMap.rows; ++y)
				{
					for (int x = 0; x < interpMap.cols; ++x)
					{
						Point2f f = flow.at<Point2f>(y, x);
						interpMap.at<Point2f>(y, x) = Point2f(x + (f.x * lerpValue), y + (f.y * lerpValue));
					}
				}

				remap(input, output, interpMap, Mat(), INTER_LINEAR);
			};

			Mat output0;
			remapWithFLow(m_frame0, output0, m_flowBackward, lerpValue);

			float inverseLerpValue = 1.0 - lerpValue;
			Mat output1;
			remapWithFLow(m_frame1, output1, m_flowForward, inverseLerpValue);

			cv::addWeighted(output0, inverseLerpValue, output1, lerpValue, 0.0f, m_output);
			// START RAE_TODO OpticalFlow version
			// RAE_TODO THINK MORE Screen output: copyMatToImage(m_output, image);

			// To debug flows individually:
			//copyMatToImage(output0, image);

			/*String outFolder = "/Users/joonaz/Documents/jonas/hdr_testi_matskut2017/glassrender/";
			int numberOfZeroes = 6;
			String tempString = std::to_string(frameCount);
			String outFile = String(numberOfZeroes - tempString.length(), '0') + tempString;
			writeMatToPng(outFolder + outFile + ".png", m_output);
			*/
			// END RAE_TODO OpticalFlow version

			frameCount++;

			doneCounter++;
			if (doneCounter >= writeFrames)
			{
				doneCounter = 0;
				setState(EffectNodeState::WaitingForData);
			}
		}
	}
}

const Mat& OpticalFlow::getOutputAtTime(float lerpTime)
{
	LOG_F(INFO, "getOutputAtTime: lerpTime: %f", lerpTime);

	if (lerpTime <= 0.0f)
		return m_frame0;
	else if (lerpTime >= 1.0f)
		return m_frame1;

	if (m_frame0.size() != m_frame1.size() ||
		m_frame0.size() != m_flowForward.size() ||
		m_frame0.size() != m_flowBackward.size())
			return m_output;

	auto remapWithFLow = [](const Mat& input, Mat& output, const Mat_<Point2f>& flow,
		float lerpTime)
	{
		Mat interpMap(flow.size(), flow.type());
		for (int y = 0; y < interpMap.rows; ++y)
		{
			for (int x = 0; x < interpMap.cols; ++x)
			{
				Point2f f = flow.at<Point2f>(y, x);
				interpMap.at<Point2f>(y, x) = Point2f(x + (f.x * lerpTime), y + (f.y * lerpTime));
			}
		}

		remap(input, output, interpMap, Mat(), INTER_LINEAR);
	};

	Mat output0;
	remapWithFLow(m_frame0, output0, m_flowBackward, lerpTime);

	float inverseLerpValue = 1.0 - lerpTime;
	Mat output1;
	remapWithFLow(m_frame1, output1, m_flowForward, inverseLerpValue);

	cv::addWeighted(output0, inverseLerpValue, output1, lerpTime, 0.0f, m_output);

	return m_output;
}

void OpticalFlow::writeMatToPng(String filepath, const cv::Mat& mat)
{
	std::vector<int> compression_params;
	compression_params.push_back(IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(9);

	imwrite(filepath, mat, compression_params);
}

void OpticalFlow::copyMatToImage(const Mat& mat, ImageBuffer<uint8_t>& image)
{
	if (image.width() != mat.cols or image.height() != mat.rows)
		image.init(mat.cols, mat.rows);

	for (int y = 0; y < mat.rows; ++y)
	{
		for (int x = 0; x < mat.cols; ++x)
		{
			const Vec3b& pix = mat.at<Vec3b>(y, x);
			image.setPixel(x, y, {uint8_t(pix[2]), uint8_t(pix[1]), uint8_t(pix[0]), 255} );

			// Grayscale version
			//uchar pix = mat.at<uchar>(y, x);
			//image.data[(y*image.width*image.channels) + (x*image.channels) + 0] = uint8_t(pix);
			//image.data[(y*image.width*image.channels) + (x*image.channels) + 1] = uint8_t(pix);
			//image.data[(y*image.width*image.channels) + (x*image.channels) + 2] = uint8_t(pix);
		}
	}

	image.requestUpdate();
}

void OpticalFlow::pushFrame(AVFrame* frameRGB)
{
	if (getState() == EffectNodeState::Nothing)
	{
		copyAVFrameToMat(frameRGB, m_frame0);
		setState(EffectNodeState::WaitingForData);
	}
	else
	{
		if (m_frame0.size() == m_frame1.size()) // To check if we ever wrote to m_frame1... so we don't do this the first time.
			m_frame0 = m_frame1;
		copyAVFrameToMat(frameRGB, m_frame1);
		setState(EffectNodeState::Processing);
	}
}

void OpticalFlow::replaceFrame0(AVFrame* frameRGB)
{
	copyAVFrameToMat(frameRGB, m_frame0);
}

void OpticalFlow::copyAVFrameToMat(AVFrame* frameRGB, cv::Mat& mat)
{
	LOG_F(INFO, "Copy AVFrame to Mat. AVFrame width: %i height: %i linesize: %i",
		frameRGB->width, frameRGB->height, frameRGB->linesize);

	mat = Mat(frameRGB->height, frameRGB->width, CV_8UC3, Scalar(0,0,255));
	for (int y = 0; y < frameRGB->height; ++y)
	{
		for (int x = 0; x < frameRGB->width; ++x)
		{
			int p = x * 3 + y * frameRGB->linesize[0];
			uint8_t r = frameRGB->data[0][p];
			uint8_t g = frameRGB->data[0][p+1];
			uint8_t b = frameRGB->data[0][p+2];
			mat.at<Vec3b>(y, x) = Vec3b(b, g, r);
		}
	}

	LOG_F(INFO, "Copied AVFrame to Mat. width: %i height: %i", mat.cols, mat.rows);
}

void OpticalFlow::writeFrameToImage(ImageBuffer<uint8_t>& image)
{
	copyMatToImage(m_output, image);
}

void OpticalFlow::writeFrameToDiskAndImage(String filepath, ImageBuffer<uint8_t>& image)
{
	copyMatToImage(m_output, image);

	String outFolder = filepath;
		int numberOfZeroes = 6;
		String tempString = std::to_string(m_outFrameCount);
		String outFile = String(numberOfZeroes - tempString.length(), '0') + tempString;
		writeMatToPng(outFolder + outFile + ".png", m_output);

	m_outFrameCount++;
}
#endif
