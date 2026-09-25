#ifndef VID_EXDATA_H
#define VID_EXDATA_H

#include "util/decomp.h"

#include <cstddef>

class VID_EXDATA {
public:
	union { int m_targetMask; int m_unk0x00; };             // 0x00
	union { int m_propertyFlags; int m_unk0x04; };          // 0x04
	union { float m_length; float m_unk0x08; };             // 0x08
	union { float m_weight; float m_unk0x0c; };             // 0x0c
	union { float m_power; float m_unk0x10; };              // 0x10
	union { float m_battleRange; float m_unk0x14; };        // 0x14
	union { float m_aimRadius; float m_unk0x18; };          // 0x18
	union { float m_bulletSpeed; float m_unk0x1c; };        // 0x1c
	union { int m_reloadTime; int m_unk0x20; };             // 0x20
	union { int m_buildCost; int m_buildTime; };            // 0x24
	union { int m_ammoCapacity; int m_maxAmmo; };           // 0x28
	int m_army;                                             // 0x2c
	int m_defaultBehavior;                                  // 0x30
	union { int m_iconFrame; undefined m_unk0x34[0x4]; };   // 0x34
	union { int m_enemyRating; int m_unk0x38; };            // 0x38
	union { float m_minRange; float m_unk0x3c; };           // 0x3c
	int m_unk0x40;            // 0x40
	float m_unk0x44[7];       // 0x44
	undefined m_unk0x60[0x4]; // 0x60
	int m_unk0x64[8];         // 0x64
	int m_unk0x84[8];         // 0x84
	int m_unk0xa4[8];         // 0xa4
	int m_unk0xc4[8];         // 0xc4
	float m_unk0xe4[8];       // 0xe4
	float m_unk0x104[8];      // 0x104
	float m_unk0x124[8];      // 0x124
	float m_unk0x144[8];      // 0x144
	float m_unk0x164[8];      // 0x164
	float m_unk0x184[8];      // 0x184
	int m_unk0x1a4[8];        // 0x1a4
	int m_unk0x1c4[8];        // 0x1c4
	int m_unk0x1e4[8];        // 0x1e4

	int m_frameSpeed[8];
	float m_speed[8];
	float m_zSpeed[8];

	int m_detectPeriod;
	int m_fireInVolley;
	int m_reloadTimeInVolley;
	int m_pad;


	int m_legacyLifeTime;
};

static_assert(offsetof(VID_EXDATA, m_army) == 0x2c);
static_assert(offsetof(VID_EXDATA, m_frameSpeed) == 0x204);
static_assert(offsetof(VID_EXDATA, m_speed) == 0x224);
static_assert(offsetof(VID_EXDATA, m_zSpeed) == 0x244);
static_assert(offsetof(VID_EXDATA, m_detectPeriod) == 0x264);

#endif
