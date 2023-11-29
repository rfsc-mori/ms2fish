#ifndef AUTOFISHKB_H
#define AUTOFISHKB_H

#include <QtGlobal>
#include <libvirt/libvirt.h>

class AutoFishKB
{
	Q_DISABLE_COPY(AutoFishKB)

public:
	AutoFishKB();
	~AutoFishKB();

	bool ready() const noexcept;

	bool connectIfNotReady();
	bool pressSpace(int holdtime = 100);

private:
	virConnectPtr m_conn = nullptr;
	virDomainPtr m_dom = nullptr;
};

#endif // AUTOFISHKB_H
