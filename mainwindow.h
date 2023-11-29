#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QElapsedTimer>
#include <QTimer>
#include "KVMFRHelper.h"
#include "autofishcv.h"
#include "autofishkb.h"

namespace Ui
{
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget* parent = nullptr);
	~MainWindow();

private:
	void setupUi();

private slots:
	void startImageViewer();
	void stopImageViewer();

	void updateImageViewer();

	void updateStatus(const QString& message);

	void autofishUpdate(AutoFishCV::State previous, AutoFishCV::State current);
	void autofishCaptcha(const QPoint& old, const QPoint& current);

	void beginFishing();

	void startLookingGlass();

private:
	Ui::MainWindow* ui = nullptr;
	QTimer m_imageUpdateTimer;

	KVMFRHelper m_kvmfr;
	AutoFishCV m_fishcv;
	AutoFishKB m_autofish;

	QElapsedTimer m_autofishCooldown;
	int m_hookCooldownReset = 300;
	constexpr static int beginfishingCooldownReset = 1000;

	constexpr static int fishingHookInterval = 50;
};

#endif // MAINWINDOW_H
