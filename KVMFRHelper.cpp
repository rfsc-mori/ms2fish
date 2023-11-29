#include "KVMFRHelper.h"
#include "KVMFRException.h"
#include <QFile>
#include <QThread>
#include <QImage>
#include <QPixmap>
#include <cstring>
#include "KVMFR.h"

#include <QDebug>

KVMFRHelper::KVMFRHelper() :
	m_shmFile(shmFile)
{
	if (m_shmFile.open(QFile::ReadWrite))
	{
		static_assert(shmSize > sizeof(KVMFRHeader));
		Q_ASSERT(m_shmFile.size() >= shmSize);

		m_shm = m_shmFile.map(0, m_shmFile.size(), QFile::NoOptions);
		m_header = reinterpret_cast<KVMFRHeader*>(m_shm);
	}
}

bool KVMFRHelper::loaded() const noexcept
{
	return (m_header != nullptr);
}

bool KVMFRHelper::validateMagic() const noexcept
{
	if (m_header == nullptr) return false;

	static_assert(sizeof(m_header->magic) >= sizeof(KVMFR_HEADER_MAGIC));

	if (std::memcmp(m_header->magic, KVMFR_HEADER_MAGIC, sizeof(KVMFR_HEADER_MAGIC)) != 0)
	{
		return false;
	}

	return true;
}

bool KVMFRHelper::validateVersion() const noexcept
{
	if (m_header == nullptr) return false;

	return (m_header->version == KVMFR_HEADER_VERSION);
}

bool KVMFRHelper::ready() const
{
	return (this->loaded() && this->validateMagic() && this->validateVersion());
}

KVMFRFrame KVMFRHelper::copyHeader() const
{
	KVMFRFrame header;
	std::memset(&header, 0, sizeof(KVMFRFrame));

	static_assert(sizeof(header) >= sizeof(KVMFRFrame));
	std::memcpy(&header, &m_header->frame, sizeof(KVMFRFrame));

	//__sync_or_and_fetch(&m_header->frame.flags, ~KVMFR_FRAME_FLAG_UPDATE);

	if (
		  header.type    >= FRAME_TYPE_MAX ||
		  header.width   == 0 ||
		  header.height  == 0 ||
		  header.pitch   == 0 ||
		  header.dataPos == 0 ||
		  header.dataPos > m_shmFile.size() ||
		  header.pitch   < header.width
		)
	{
		throw KVMFRInvalidHeaderException("Header sanity check failed.");
	}

	return header;
}

const QByteArray &KVMFRHelper::fetchFrame()
{
	//__sync_or_and_fetch(&m_header->flags, KVMFR_HEADER_FLAG_RESTART);

	KVMFRFrame header;

	try { header = this->copyHeader(); }
	catch (const KVMFRInvalidHeaderException&) { throw; }

	size_t dataSize = 0;
	size_t bpp = 0; // bits per pixel

	if (header.type == FRAME_TYPE_ARGB)
	{
		dataSize = header.height * header.pitch;
		bpp = 32;
	}
	else
	{
		throw KVMFRUnsupportedFrameTypeException("Unsupported frame type.");
	}

	if (header.dataPos + dataSize > m_shmFile.size())
	{
		throw KVMFRInvalidHeaderException("Header has an invalid dataPos.");
	}

	m_currentFrame = QByteArray(reinterpret_cast<const char*>(m_shm) + header.dataPos, dataSize);

	// m_currentFrame data must not be changed and must stay alive.
	m_currentImage = QImage(reinterpret_cast<const uchar*>(m_currentFrame.data()), shmWidth, shmHeight, shmWidth * (bpp / 8), QImage::Format_ARGB32);

	return m_currentFrame;
}

const QByteArray& KVMFRHelper::currentFrame() const
{
	return m_currentFrame;
}

const uchar* KVMFRHelper::currentFrameData() const
{
	return reinterpret_cast<const uchar*>(m_currentFrame.data());
}

const QImage& KVMFRHelper::currentFrameAsImage()
{
	return m_currentImage;
}

QPixmap KVMFRHelper::currentFrameAsPixmap() const
{
	return QPixmap::fromImage(m_currentImage);
}
