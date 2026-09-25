#ifndef CONST_H
#define CONST_H

#include "util/decomp.h"

#include <cstddef>

class RESOURCE;

class CONSTS {
public:
	CONSTS(RESOURCE* p_res);

	union { float m_maxScrollSpeedX; float m_unk0x00; };             // 0x00
	union { float m_maxScrollSpeedY; float m_unk0x04; };             // 0x04
	union { float m_gravitation; float m_unk0x08; };                 // 0x08
	union { float m_gravitation2; float m_unk0x0c; };                // 0x0c
	union { int m_repairSpeed; int m_repairByRepairHp; };            // 0x10
	union { int m_ammoReloadTime; int m_addAmmo; };                  // 0x14
	union { float m_railRepairSpeed; float m_unk0x18; };             // 0x18
	union { float m_masterRepairSpeed; float m_unk0x1c; };           // 0x1c
	union { int m_friction; int m_unk0x20; };                        // 0x20
	union { int m_depoMillisecondsInSecond; int m_unk0x24; };        // 0x24
	int m_debugMode;                                                 // 0x28
	union { int m_depoAutoRepairTimeInSeconds; int m_unk0x2c; };     // 0x2c
	union { int m_masterAutoRepairTimeInSeconds; int m_unk0x30; };   // 0x30
	union { unsigned int m_mouseTipsTime; unsigned int m_unk0x34; }; // 0x34
	union { int m_depoAutoAddHpPerSecond; int m_depoAddHp; };        // 0x38
	union { int m_masterAutoAddHpPerSecond; int m_buildingAddHp; };  // 0x3c
	union { int m_fortCannonsAutoAddHpPerSecond; int m_unk0x40; };   // 0x40
	union { int m_repairSettingMineTime; int m_repairDockTime; };    // 0x44
	union { int m_repairDestroyingMineTime; int m_unk0x48; };        // 0x48
	union { int m_dirijbanAmmoReloadTime; int m_balloonAddAmmo; };   // 0x4c
	union { unsigned int m_selectUnitGamma; unsigned int m_unk0x50; }; // 0x50
	union { unsigned int m_attackUnitGamma; unsigned int m_unk0x54; }; // 0x54
	union { unsigned int m_lightedUnitGamma; unsigned int m_unk0x58; };// 0x58
	union { int m_nukeForBirth; int m_unk0x5c; };                    // 0x5c
	union { float m_safeClashSpeed; float m_minMoveSpeed; };         // 0x60
	union { int m_messageStartDelay; int m_unk0x64; };               // 0x64
};

static_assert(sizeof(CONSTS) == 0x68, "CONSTS schema must be exactly 104 bytes");
static_assert(offsetof(CONSTS, m_debugMode) == 0x28, "m_debugMode offset mismatch");
static_assert(offsetof(CONSTS, m_safeClashSpeed) == 0x60, "m_safeClashSpeed offset mismatch");

extern CONSTS* Const;

#endif
