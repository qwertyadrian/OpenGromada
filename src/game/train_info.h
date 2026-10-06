#ifndef TRAIN_INFO_H
#define TRAIN_INFO_H

#include "util/decomp.h"

class ENGINE;

class TRAIN_INFO {
public:
	unsigned int m_flag; // 0x00
	union {
		int m_unk0x04;          // 0x04
		float m_maxWeaponRange; // 0x04
	};
	union {
		int m_unk0x08;          // 0x08
		float m_minWeaponRange; // 0x08
	};
	float m_speedInc;  // 0x0c
	float m_accelTime; // 0x10
	union {
		float m_unk0x14;   // 0x14
		float m_accelWeight;
	};
	union {
		int m_unk0x18;     // 0x18
		int m_maxSpeed;    // PROP_SPEED (1)
	};
	union {
		int m_unk0x1c;     // 0x1c
		int m_carCount;
	};
	union {
		int m_unk0x20;     // 0x20
		int m_currentHp;   // PROP_HP (4)
	};
	union {
		int m_unk0x24;     // 0x24
		int m_maxHp;       // for PROP_LIFE (3)
	};
	union {
		int m_unk0x28;     // 0x28
		int m_weaponPower; // PROP_WEAPON (2)
	};
	union {
		int m_unk0x2c;     // 0x2c
		int m_buildTime;   // PROP_BUILD_TIME (7)
	};
	union {
		int m_unk0x30;     // 0x30
		int m_ammoCarCount;
	};
	union {
		int m_unk0x34;     // 0x34
		int m_currentAmmo; // PROP_AMMO_NO (10)
	};
	union {
		int m_unk0x38;     // 0x38
		int m_maxAmmo;     // PROP_AMMO_MAX (11)
	};
	union {
		int m_unk0x3c;     // 0x3c
		int m_ammoPercent; // PROP_AMMO (5)
	};

	TRAIN_INFO(const ENGINE* p_engine);
	void AddEngine(const ENGINE* p_engine);
	int Acceleration() const;
};

static_assert(sizeof(TRAIN_INFO) == 0x40, "TRAIN_INFO size mismatch");

int* FirstTrain(int p_army);
int* NextTrain();

#endif
