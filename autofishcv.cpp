#include "autofishcv.h"
#include <QFile>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include "mat_and_qimage.hpp"

#include <opencv2/highgui.hpp>
#include <QDebug>

AutoFishCV::AutoFishCV(QObject* parent) :
	QObject(parent)
{
	this->loadResource(Resource::BeginFishingImage, ":/images/ms2fish-beginfishing-kr.png");
	this->loadResource(Resource::BeginFishingMask, ":/images/ms2fish-beginfishing-mask-kr.png");
	this->loadResource(Resource::HookImage, ":/images/ms2fish-hook.png");
	this->loadResource(Resource::HookMask, ":/images/ms2fish-hook-mask.png");
}

void AutoFishCV::setImage(const QImage& image)
{
	m_image = ocv::qt::qimage_to_mat_cpy(image);
	cv::cvtColor(m_image, m_image, CV_BGR2GRAY);
}

void AutoFishCV::resetState()
{
	m_state = State::Invalid;
}

bool AutoFishCV::loadResource(Resource state, const QString& rc)
{
	QFile file(rc);

	if (file.open(QFile::ReadOnly))
	{
		qint64 size = file.size();
		Q_ASSERT(size > 0);

		std::vector<uchar> buffer(size);
		file.read(reinterpret_cast<char*>(buffer.data()), size);

		m_resources.insert(state, cv::imdecode(buffer, cv::IMREAD_GRAYSCALE));

		return true;
	}

	return false;
}

// https://docs.opencv.org/3.0-beta/doc/tutorials/imgproc/histograms/template_matching/template_matching.html
bool AutoFishCV::isSimilarDbg(const cv::Mat& lg, const cv::Mat& pattern, const cv::Mat& mask, QPoint* center) const
{
	bool match = false;
	int match_method = cv::TM_CCORR_NORMED;

	cv::Mat img_display;
	lg.copyTo(img_display);

	int result_rows = lg.rows - pattern.rows + 1;
	int result_cols = lg.cols - pattern.cols + 1;
	cv::Mat result(result_rows, result_cols, CV_32FC1);

	cv::matchTemplate(lg, pattern, result, match_method, mask);
	cv::threshold(result, result, 0.95, 1, cv::THRESH_TOZERO);
	cv::normalize(result, result, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());

	double minVal = 0, maxVal = 0, threshold = 0.6;
	cv::Point minLoc {0}, maxLoc {0};
	cv::Point matchLoc;

	cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat());

	if (match_method == cv::TM_SQDIFF || match_method == cv::TM_SQDIFF_NORMED)
	{
		matchLoc = minLoc;
	}
	else
	{
		matchLoc = maxLoc;
	}

	if (center != nullptr)
	{
		*center = QPoint(matchLoc.x + (pattern.cols / 2), matchLoc.y + (pattern.rows / 2));
	}

	if (maxVal >= threshold)
	{
		cv::rectangle(img_display, matchLoc, cv::Point(matchLoc.x + pattern.cols, matchLoc.y + pattern.rows), cv::Scalar::all(68), 2, 8, 0);
		cv::rectangle(result, matchLoc, cv::Point(matchLoc.x + pattern.cols, matchLoc.y + pattern.rows), cv::Scalar::all(68), 2, 8, 0);

		match = true;
	}

	cv::imshow("a", lg);
	cv::imshow("b", pattern);
	cv::imshow("1", img_display);
	cv::imshow("2", result);

	return match;
}

bool AutoFishCV::isSimilar(const cv::Mat& lg, const cv::Mat& pattern, const cv::Mat& mask, QPoint* center) const
{
	int result_rows = lg.rows - pattern.rows + 1;
	int result_cols = lg.cols - pattern.cols + 1;
	cv::Mat result(result_rows, result_cols, CV_32FC1);

	cv::matchTemplate(lg, pattern, result, cv::TM_CCORR_NORMED, mask);
	cv::threshold(result, result, 0.95, 1, cv::THRESH_TOZERO);
	cv::normalize(result, result, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());

	double minVal = 0, maxVal = 0, threshold = 0.6;
	cv::Point minLoc {0}, maxLoc {0};
	cv::Point matchLoc;

	cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat());
	matchLoc = maxLoc;

	if (center != nullptr)
	{
		*center = QPoint(matchLoc.x + (pattern.cols / 2), matchLoc.y + (pattern.rows / 2));
	}

	if (maxVal >= threshold)
	{
		return true;
	}

	return false;
}

void AutoFishCV::checkAllStates()
{
	State state = State::Idle;

	if (this->checkBeginFishingState())
	{
		state = State::BeginFishing;
	}

	if (this->checkHookState())
	{
		state = State::Hook;
	}

	if (m_state != state)
	{
		State old = m_state;
		m_state = state;

		emit autofishStateUpdated(old, m_state);
	}
}

bool AutoFishCV::checkBeginFishingState()
{
	Q_ASSERT(m_resources.contains(Resource::BeginFishingImage));
	Q_ASSERT(m_resources.contains(Resource::BeginFishingMask));

	cv::Rect r {1008, 466, 251, 113};
	cv::Mat idle = m_image(r);

	cv::imwrite("kr.jpg", idle);

	if (isSimilar(idle, m_resources[Resource::BeginFishingImage], m_resources[Resource::BeginFishingMask]))
	{
		return true;
	}

	return false;
}

bool AutoFishCV::checkHookState()
{
	Q_ASSERT(m_resources.contains(Resource::HookImage));
	Q_ASSERT(m_resources.contains(Resource::HookMask));

	cv::Rect r {835, 527, 100, 243};
	cv::Mat idle = m_image(r);

	QPoint center;

	if (isSimilar(idle, m_resources[Resource::HookImage], m_resources[Resource::HookMask], &center))
	{
		QPoint old = m_hook;
		m_hook = center;

		emit autofishHookUpdated(old, m_hook);

		return true;
	}

	return false;
}

bool AutoFishCV::isHookOnTop() const noexcept
{
	return (m_hook.y() < topRedH);
}

bool AutoFishCV::isHookOnMiddle() const noexcept
{
	return (middleBlueL <= m_hook.y() && m_hook.y() < middleBlueH);
}

bool AutoFishCV::isHookOnBottom() const noexcept
{
	return (bottomRedL < m_hook.y());
}

int AutoFishCV::hookBottomOffset() const noexcept
{
	return (m_hook.y() - bottomRedL);
}
