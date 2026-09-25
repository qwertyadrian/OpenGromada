#include "game/constant.h"

#include "util/myerror.h"
#include "util/resource.h"

#include <string.h>

// FUNCTION: ALIEN 0x42df40
CONSTS::CONSTS(RESOURCE* p_res)
{
	*this = CONSTS();
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
	int rawMaxScrollSpeedX = 0, rawMaxScrollSpeedY = 0;
	int rawGravitation = 0, rawGravitation2 = 0;
	int rawRailRepairSpeed = 0, rawMasterRepairSpeed = 0;
	int rawSafeClashSpeed = 0;

	res->Read(&rawMaxScrollSpeedX, 4);
	res->Read(&rawMaxScrollSpeedY, 4);
	res->Read(&rawGravitation, 4);
	res->Read(&rawGravitation2, 4);
	res->Read(&m_repairSpeed, 4);
	res->Read(&m_ammoReloadTime, 4);
	res->Read(&rawRailRepairSpeed, 4);
	res->Read(&rawMasterRepairSpeed, 4);
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
	res->Read(&rawSafeClashSpeed, 4);
	res->Read(&m_messageStartDelay, 4);

	m_maxScrollSpeedX = static_cast<float>(rawMaxScrollSpeedX) * 0.001f;
	m_maxScrollSpeedY = static_cast<float>(rawMaxScrollSpeedY) * 0.001f;
	m_gravitation = static_cast<float>(rawGravitation) * 0.000001f;
	m_gravitation2 = static_cast<float>(rawGravitation2) * 0.000001f;
	m_railRepairSpeed = static_cast<float>(rawRailRepairSpeed) * 0.001f;
	m_masterRepairSpeed = static_cast<float>(rawMasterRepairSpeed) * 0.001f;
	m_safeClashSpeed = static_cast<float>(rawSafeClashSpeed) * 0.001f;
}
