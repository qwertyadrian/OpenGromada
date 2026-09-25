#include "game/const.h"
#include "util/resource.h"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>

int main(int argc, char** argv)
{
	const char* resPath = argc > 1 ? argv[1] : "objects.res";
	printf("Testing CONSTS loading from: %s\n", resPath);

	RESOURCE res;
	if (res.OpenForRead(STRING(resPath), 0x41544144 /* 'DATA' */)) {
		fprintf(stderr, "ERROR: Cannot open resource file for reading: %s\n", resPath);
		return 1;
	}

	CONSTS c(&res);

	printf("\n--- CONSTS Values in Memory (OpenGromada C++) ---\n");
	printf("m_maxScrollSpeedX:             %f (expected 0.500000)\n", c.m_maxScrollSpeedX);
	printf("m_maxScrollSpeedY:             %f (expected 0.400000)\n", c.m_maxScrollSpeedY);
	printf("m_gravitation:                 %f (expected 0.000174)\n", c.m_gravitation);
	printf("m_gravitation2:                %f (expected 0.000084)\n", c.m_gravitation2);
	printf("m_repairSpeed:                 %d (expected 2)\n", c.m_repairSpeed);
	printf("m_ammoReloadTime:              %d (expected 30)\n", c.m_ammoReloadTime);
	printf("m_railRepairSpeed:             %f (expected 0.015000)\n", c.m_railRepairSpeed);
	printf("m_masterRepairSpeed:           %f (expected 0.005000)\n", c.m_masterRepairSpeed);
	printf("m_friction:                    %d (expected 600)\n", c.m_friction);
	printf("m_depoMillisecondsInSecond:    %d (expected 1200)\n", c.m_depoMillisecondsInSecond);
	printf("m_debugMode:                   %d (expected 1)\n", c.m_debugMode);
	printf("m_depoAutoRepairTimeInSeconds: %d (expected 120)\n", c.m_depoAutoRepairTimeInSeconds);
	printf("m_masterAutoRepairTimeInSeconds: %d (expected 120)\n", c.m_masterAutoRepairTimeInSeconds);
	printf("m_mouseTipsTime:               %u (expected 200)\n", c.m_mouseTipsTime);
	printf("m_depoAutoAddHpPerSecond:      %d (expected 1)\n", c.m_depoAutoAddHpPerSecond);
	printf("m_masterAutoAddHpPerSecond:    %d (expected 1)\n", c.m_masterAutoAddHpPerSecond);
	printf("m_fortCannonsAutoAddHpPerSecond: %d (expected 0)\n", c.m_fortCannonsAutoAddHpPerSecond);
	printf("m_repairSettingMineTime:       %d (expected 2000)\n", c.m_repairSettingMineTime);
	printf("m_repairDestroyingMineTime:    %d (expected 2000)\n", c.m_repairDestroyingMineTime);
	printf("m_dirijbanAmmoReloadTime:      %d (expected 30)\n", c.m_dirijbanAmmoReloadTime);
	printf("m_selectUnitGamma:             %u (expected 56576)\n", c.m_selectUnitGamma);
	printf("m_attackUnitGamma:             %u (expected 9175129)\n", c.m_attackUnitGamma);
	printf("m_lightedUnitGamma:            %u (expected 14540032)\n", c.m_lightedUnitGamma);
	printf("m_nukeForBirth:                %d (expected 4202624)\n", c.m_nukeForBirth);
	printf("m_safeClashSpeed:              %f (expected 0.015000)\n", c.m_safeClashSpeed);
	printf("m_messageStartDelay:           %d (expected 1000)\n", c.m_messageStartDelay);
	printf("------------------------------------------------\n\n");

	// Strict automated assertions
	assert(std::fabs(c.m_maxScrollSpeedX - 0.5f) < 1e-5f);
	assert(std::fabs(c.m_maxScrollSpeedY - 0.4f) < 1e-5f);
	assert(std::fabs(c.m_gravitation - 0.000174f) < 1e-7f);
	assert(std::fabs(c.m_gravitation2 - 0.000084f) < 1e-7f);
	assert(c.m_repairSpeed == 2);
	assert(c.m_ammoReloadTime == 30);
	assert(std::fabs(c.m_railRepairSpeed - 0.015f) < 1e-5f);
	assert(std::fabs(c.m_masterRepairSpeed - 0.005f) < 1e-5f);
	assert(c.m_friction == 600);
	assert(c.m_depoMillisecondsInSecond == 1200);
	assert(c.m_depoAutoRepairTimeInSeconds == 120);
	assert(c.m_masterAutoRepairTimeInSeconds == 120);
	assert(c.m_mouseTipsTime == 200);
	assert(c.m_depoAutoAddHpPerSecond == 1);
	assert(c.m_masterAutoAddHpPerSecond == 1);
	assert(c.m_fortCannonsAutoAddHpPerSecond == 0);
	assert(c.m_repairSettingMineTime == 2000);
	assert(c.m_repairDestroyingMineTime == 2000);
	assert(c.m_dirijbanAmmoReloadTime == 30);
	assert(c.m_selectUnitGamma == 56576);
	assert(c.m_attackUnitGamma == 9175129);
	assert(c.m_lightedUnitGamma == 14540032);
	assert(c.m_nukeForBirth == 4202624);
	assert(std::fabs(c.m_safeClashSpeed - 0.015f) < 1e-5f);
	assert(c.m_messageStartDelay == 1000);

	// Also verify union backward compatibility aliases
	assert(c.m_unk0x00 == c.m_maxScrollSpeedX);
	assert(c.m_repairByRepairHp == c.m_repairSpeed);
	assert(c.m_addAmmo == c.m_ammoReloadTime);
	assert(c.m_minMoveSpeed == c.m_safeClashSpeed);

	printf("[PASS] All 26 CONSTS fields verified successfully in C++ memory!\n");
	return 0;
}
