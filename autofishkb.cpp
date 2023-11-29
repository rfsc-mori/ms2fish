#include "autofishkb.h"

AutoFishKB::AutoFishKB()
{
}

AutoFishKB::~AutoFishKB()
{
	if (m_conn)
	{
		m_dom = nullptr;

		virConnectClose(m_conn);
		m_conn = nullptr;
	}
}

bool AutoFishKB::ready() const noexcept
{
	return (m_conn != nullptr && m_dom != nullptr);
}

bool AutoFishKB::connectIfNotReady()
{
	if (ready()) return true;

	if (m_conn == nullptr)
	{
		m_conn = virConnectOpen("qemu:///system");
	}

	if (m_conn != nullptr)
	{
		m_dom = virDomainLookupByName(m_conn, "wintos");
	}

	return ready();
}

bool AutoFishKB::pressSpace(int holdtime)
{
	if (!connectIfNotReady()) return false;
	if (holdtime < 0) return false;

	constexpr uint keycount = 1;
	uint keycodes[keycount] {0x20}; // VK_SPACE

	return (virDomainSendKey(m_dom, VIR_KEYCODE_SET_WIN32, holdtime, keycodes, keycount, 0) == 0);
}
