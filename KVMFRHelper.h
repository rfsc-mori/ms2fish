#ifndef KVMFRHELPER_H
#define KVMFRHELPER_H

#include <QFile>
#include <QByteArray>
#include <QImage>

struct KVMFRHeader;
struct KVMFRFrame;

class KVMFRHelper
{
	Q_DISABLE_COPY(KVMFRHelper)

public:
	KVMFRHelper();

public:
	bool ready() const;

	const QByteArray& fetchFrame();

	const QByteArray& currentFrame() const;
	const uchar* currentFrameData() const;

	const QImage& currentFrameAsImage();
	QPixmap currentFrameAsPixmap() const;

private:
	bool loaded() const noexcept;
	bool validateMagic() const noexcept;
	bool validateVersion() const noexcept;

	KVMFRFrame copyHeader() const;

private:
	QFile m_shmFile;
	uchar* m_shm = nullptr;
	KVMFRHeader* m_header = nullptr;
	QByteArray m_currentFrame;
	QImage m_currentImage;

private:
	constexpr static const char* shmFile = "/dev/shm/looking-glass";
	constexpr static const qint64 shmWidth = 1920;
	constexpr static const qint64 shmHeight = 1080;
	constexpr static const qint64 shmSize = shmWidth * shmHeight * 4 * 2; // https://looking-glass.hostfission.com/node/9
};

#endif // KVMFRHELPER_H
