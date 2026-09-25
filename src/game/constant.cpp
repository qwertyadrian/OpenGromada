#include "game/constant.h"

#include "util/myerror.h"
#include "util/resource.h"

#include <string.h>

// FUNCTION: ALIEN 0x42df40
CONSTS::CONSTS(RESOURCE* p_res)
{
	memset(this, 0, sizeof(*this));
	RESOURCE* res = p_res;
	int ignored;
	if (res->GoBegin(0x54534e43)) {
		MYERROR::Log(
			::Error,
			// STRING: ALIEN 0x483ab8
			"!!!ERROR!!! CNST Load Constant section not found"
		);
		return;
	}
	res->Read(&m_maxScrollSpeedX, 4);
	res->Read(&m_maxScrollSpeedY, 4);
	res->Read(&m_gravitation, 4);
	res->Read(&m_gravitation2, 4);
	res->Read(&m_repairSpeed, 4);
	res->Read(&m_ammoReloadTime, 4);
	res->Read(&m_railRepairSpeed, 4);
	res->Read(&m_masterRepairSpeed, 4);
	res->Read(&m_friction, 4);
	res->Read(&m_depoMillisecondsInSecond, 4);
	res->Read(&ignored, 4);
	res->Read(&m_depoAutoRepairTimeInSeconds, 4);
	res->Read(&m_masterAutoRepairTimeInSeconds, 4);
	res->Read(&m_mouseTipsTime, 4);
	res->Read(&m_depoAutoAddHpPerSecond, 4);
	res->Read(&m_masterAutoAddHpPerSecond, 4);
	res->Read(&m_fortCannonsAutoAddHpPerSecond, 4);
	res->Read(&m_repairSettingMineTime, 4);
	res->Read(&m_repairDestroyingMineTime, 4);
	res->Read(&m_dirijbanAmmoReloadTime, 4);
	res->Read(&m_selectUnitGamma, 4);
	res->Read(&m_attackUnitGamma, 4);
	res->Read(&m_lightedUnitGamma, 4);
	res->Read(&m_nukeForBirth, 4);
	res->Read(&m_safeClashSpeed, 4);
	res->Read(&m_messageStartDelay, 4);

	m_maxScrollSpeedX *= 0.001f;
	m_maxScrollSpeedY *= 0.001f;
	m_gravitation *= 0.000001f;
	m_gravitation2 *= 0.000001f;
	m_masterRepairSpeed *= 0.001f;
	m_railRepairSpeed *= 0.001f;
	m_safeClashSpeed *= 0.001f;
}
