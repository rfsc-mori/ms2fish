#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "KVMFRException.h"
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QStatusBar>
#include "random.hpp"
#include <QProcess>

#include <QDebug>

using Random = effolkronium::random_static;

MainWindow::MainWindow(QWidget* parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	m_fishcv(this)
{
	ui->setupUi(this);
	this->setupUi();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::setupUi()
{
	 connect(ui->pbStart, &QPushButton::clicked, this, &MainWindow::startImageViewer);
	 connect(ui->pbStop, &QPushButton::clicked, this, &MainWindow::stopImageViewer);
	 connect(ui->pbControl, &QPushButton::clicked, this, &MainWindow::startLookingGlass);
	 connect(&m_imageUpdateTimer, &QTimer::timeout, this, &MainWindow::updateImageViewer);
	 connect(&m_fishcv, &AutoFishCV::autofishStateUpdated, this, &MainWindow::autofishUpdate);
	 connect(&m_fishcv, &AutoFishCV::autofishHookUpdated, this, &MainWindow::autofishCaptcha);
}

void MainWindow::startImageViewer()
{
	if (!m_autofish.connectIfNotReady())
	{		
		this->updateStatus("Failed to connect to libvirt.");
		return;
	}

	m_fishcv.resetState();

	m_imageUpdateTimer.start(ui->sbInterval->value());
	this->updateImageViewer();
}

void MainWindow::stopImageViewer()
{
	m_imageUpdateTimer.stop();
}

void MainWindow::updateImageViewer()
{
	if (m_kvmfr.ready())
	{
		try
		{
			m_kvmfr.fetchFrame();
		}
		catch (const KVMFRException& e)
		{
			this->updateStatus(QString("Failed to fetch current frame: %1").arg(e.what()));
			ui->imageViewer->clear();

			return;
		}

		if (ui->cbScale->isChecked())
		{
			ui->imageViewer->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
			ui->imageViewer->setPixmap(m_kvmfr.currentFrameAsPixmap().scaled(ui->imageViewer->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
		}
		else
		{
			ui->imageViewer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
			ui->imageViewer->setPixmap(m_kvmfr.currentFrameAsPixmap());
		}

		m_fishcv.setImage(m_kvmfr.currentFrameAsImage());
		m_fishcv.checkAllStates();
	}
}

void MainWindow::updateStatus(const QString& message)
{
	ui->statusBar->showMessage(message);
}

void MainWindow::autofishUpdate(AutoFishCV::State, AutoFishCV::State current)
{
	if (current == AutoFishCV::State::Idle)
	{
		this->updateStatus("Waiting.");

		if (m_imageUpdateTimer.interval() == fishingHookInterval)
		{
			m_imageUpdateTimer.setInterval(ui->sbInterval->value());
		}
	}
	else if (current == AutoFishCV::State::BeginFishing)
	{
		this->updateStatus("Begin fishing...");

		if (m_imageUpdateTimer.interval() == fishingHookInterval)
		{
			m_imageUpdateTimer.setInterval(ui->sbInterval->value());
		}

		bool cooldown = false;

		if (m_autofishCooldown.isValid())
		{
			if (!m_autofishCooldown.hasExpired(beginfishingCooldownReset))
			{
				m_fishcv.resetState();
				cooldown = true;
			}
		}

		if (!cooldown)
		{
			QTimer::singleShot(Random::get<int>(500, 5000), this, &MainWindow::beginFishing);
			m_autofishCooldown.start();
		}
	}
	else if (current == AutoFishCV::State::Hook)
	{
		this->updateStatus("Fishing hook detected!");

		m_autofishCooldown.invalidate();

		m_imageUpdateTimer.setInterval(fishingHookInterval);
		this->updateImageViewer();
	}
}

void MainWindow::beginFishing()
{
	m_autofish.pressSpace(Random::get<int>(100, 150));
}

void MainWindow::autofishCaptcha(const QPoint&, const QPoint&)
{
	qDebug() << "Captcha reached.";

	if (m_autofishCooldown.isValid())
	{
		if (!m_autofishCooldown.hasExpired(m_hookCooldownReset))
		{
			return;
		}
	}

	if (m_fishcv.isHookOnTop())
	{
		if (Random::get<bool>(0.05))
		{
			m_autofish.pressSpace(Random::get<int>(120, 200));
			m_hookCooldownReset = Random::get<int>(250, 300);
			m_autofishCooldown.start();

			qDebug() << "Top hit.";
		}
	}

	if (m_fishcv.isHookOnMiddle())
	{
		m_autofish.pressSpace(Random::get<int>(60, 150));
		m_hookCooldownReset = Random::get<int>(200, 250);
		m_autofishCooldown.start();

		qDebug() << "Middle hit.";
	}

	if (m_fishcv.isHookOnBottom())
	{
		m_autofish.pressSpace(50);
		m_hookCooldownReset = fishingHookInterval;
		m_autofishCooldown.start();

		qDebug() << "Bottom hit.";
	}
}

void MainWindow::startLookingGlass()
{
	QProcess::startDetached("vmlg");
}
