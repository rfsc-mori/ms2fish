#ifndef AUTOFISHCV_H
#define AUTOFISHCV_H

#include <QObject>
#include <QMap>
#include <QPoint>
#include <opencv2/core/mat.hpp>

class AutoFishCV : public QObject
{
	Q_OBJECT

public:
	enum class State
	{
		Invalid,
		Idle,
		BeginFishing,
		Hook
	};

private:
	enum class Resource
	{
		BeginFishingImage,
		BeginFishingMask,
		HookImage,
		HookMask
	};

public:
	explicit AutoFishCV(QObject* parent = nullptr);

signals:
	void autofishStateUpdated(State previous, State current);
	void autofishHookUpdated(const QPoint& previous, const QPoint& center);

public slots:
	void setImage(const QImage& image);
	void resetState();

	void checkAllStates();
	bool checkBeginFishingState();
	bool checkHookState();

public:
	bool isHookOnTop() const noexcept;
	bool isHookOnMiddle() const noexcept;
	bool isHookOnBottom() const noexcept;

	int hookBottomOffset() const noexcept;

private:
	bool loadResource(Resource state, const QString& rc);
	bool isSimilar(const cv::Mat& lg, const cv::Mat& pattern, const cv::Mat& mask = cv::Mat(), QPoint* center = nullptr) const;
	bool isSimilarDbg(const cv::Mat& lg, const cv::Mat& pattern, const cv::Mat& mask = cv::Mat(), QPoint* center = nullptr) const;

private:
	cv::Mat m_image;
	QMap<Resource, cv::Mat> m_resources;
	State m_state = State::Invalid;
	QPoint m_hook;

	constexpr static int hitUp = 5;

	constexpr static int topRedH = 90;
	constexpr static int middleBlueL = topRedH + 20;
	constexpr static int middleBlueH = 130;
	constexpr static int bottomRedL = 135;
};

#endif // AUTOFISHCV_H
