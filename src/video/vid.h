#ifndef VID_H
#define VID_H

#include "gfx/gamma.h"
#include "util/angle.h"
#include "util/decomp.h"

class SPRITE;
class TEXTURE;
class VID_EXDATA;
class RESOURCE;
struct VID_TEXCOOR;

// VTABLE: ALIEN 0x47a574

class VID {
public:
	VID();

	static int MemoryInUse;

	static int ViewXMin();
	static int ViewXMax();
	static int ViewYMin();
	static int ViewYMax();

	virtual VID* CreateMirror();                                  // vtable+0x00
	virtual void* ScalarDeletingDestructor(unsigned int p_flags); // vtable+0x04
	~VID();
	virtual void DrawVidToVid(const SPRITE* p_sprite); // vtable+0x08
	virtual int Draw(SPRITE* p_sprite);                // vtable+0x0c
	virtual void DrawShadow(SPRITE* p_sprite) {}       // vtable+0x10
	virtual void DrawToVid(
		const SPRITE* p_sprite,
		const VID_TEXCOOR* p_texCoor,
		TEXTURE* p_texture,
		TEXTURE* p_zTexture
	);                                                              // vtable+0x14
	virtual void Load(RESOURCE* p_res) {}                           // vtable+0x18
	virtual int SetGamma(const GAMMA& p_gamma, unsigned int p_idx); // vtable+0x1c
	virtual int HaveShadow() { return 0; }                          // vtable+0x20
	virtual void SetLayer();                                        // vtable+0x24

	int m_idx;              // 0x04
	char* m_name;           // 0x08
	union { unsigned int m_sprType; unsigned int m_unk0x0c; }; // 0x0c
	undefined4 m_sprClass;                                    // 0x10
	union { unsigned int m_propertyFlags; unsigned int m_flag; }; // 0x14
	union { int m_moveMask; int m_unk0x18; };                 // 0x18
	union {
		undefined m_unk0x1c[0x8];
		struct {
			float m_footprintWidth;  // 0x1c
			float m_footprintHeight; // 0x20
		};
	};
	union { float m_sizeZ; float m_unk0x24; };                // 0x24

	int m_defaultMaxHp;                                       // 0x28
	union { float m_maxSpeed; float m_unk0x2c; };             // 0x2c
	union { float m_maxZSpeed; float m_unk0x30; };            // 0x30
	union { float m_acceleration; float m_unk0x34; };         // 0x34
	union { float m_deceleration; float m_unk0x38; };         // 0x38
	union { float m_rotationPeriod; float m_unk0x3c; };       // 0x3c
	union {
		VID* m_weapon; // 0x40
		VID* m_unk0x40;
		int m_weaponIdx; // 0x40
	};
	union { float m_deathDamage; float m_blastRadius; };      // 0x44
	union { int m_deathPush; int m_fireDamage; };             // 0x48
	union { float m_linkCoorX; float m_unk0x4c; };            // 0x4c
	union { float m_linkCoorY; float m_unk0x50; };            // 0x50
	union { float m_linkCoorZ; float m_unk0x54; };            // 0x54
	union {
		int m_linkVidIdx;
		int m_nLinkVid; // 0x58
		undefined m_unk0x58[4];
	};
	union {
		VID* m_linkVid; // 0x5c
		VID* m_unk0x5c;
	};
	union { float m_topZ; float m_unk0x60; };                 // 0x60
	union { float m_moveUpZ1; float m_unk0x64; };             // 0x64
	union { float m_moveUpZ2; float m_unk0x68; };             // 0x68
	union { unsigned int m_lifetime; undefined4 m_unk0x6c; }; // 0x6c
	unsigned int m_noDir;                                     // 0x70
	int m_noAnimCadr[17];                                     // 0x74
	int m_aniSfx[17];                                         // 0xb8
	unsigned int m_aniDuration[17];                           // 0xfc
	union { float m_aniChildX[17]; float m_unk0x140[17]; };   // 0x140
	union { float m_aniChildY[17]; float m_unk0x184[17]; };   // 0x184
	union { float m_aniChildZ[17]; float m_unk0x1c8[17]; };   // 0x1c8
	union { int m_aniChildNvid[17]; int m_unk0x20c[17]; };    // 0x20c
	union {                                                   // 0x250
		VID* m_aniChildVid[17];
		struct {
			VID* m_unk0x250[8];
			VID* m_weaponVid;
			VID* m_unk0x274[4];
			VID* m_unk0x284;
			VID* m_unk0x288[2];
			VID* m_unk0x290;
		};
	};

	int m_aniFireCount[17]; // 0x294
	int m_colorSub;         // 0x2d8
	int m_colorAdd;         // 0x2dc
	float m_gammaR;         // 0x2e0
	float m_gammaG;         // 0x2e4
	float m_gammaB;         // 0x2e8
	char* m_fname;          // 0x2ec

	union {
		unsigned short m_pixelFlag16; // 0x2f0
		struct {
			unsigned char m_pixelFlag; // 0x2f0
			unsigned char m_fontFlag;  // 0x2f1
		};
	};
	union {
		undefined m_unk0x2f2[0xa]; // 0x2f2
		struct {
			unsigned short m_defaultAniPeriod; // 0x2f2
			short m_dotFrameCount;             // 0x2f4
			short m_unk0x2f6;                  // 0x2f6
			short m_messageLineHeight;         // 0x2f8
			undefined m_unk0x2fa[0x2];         // 0x2fa
		};
	};
	int m_aniBegCadr[17];    // 0x2fc
	int m_aniDirCadrs[17];   // 0x340
	float m_unk0x384;        // 0x384
	float m_unk0x388;        // 0x388
	int m_layer;             // 0x38c
	unsigned int m_unk0x390; // 0x390
	int m_unk0x394[5];       // 0x394

	int m_entitiesNumber[4]; // 0x3a8
	int m_deaths[4];         // 0x3b8
	int m_recolors[4];       // 0x3c8
	int m_maxHp[4];          // 0x3d8

	GAMMA m_gamma[4];        // 0x3e8
	int m_unk0x408[20];      // 0x408
	unsigned int m_unk0x458; // 0x458
	VID_EXDATA* m_exData;    // 0x45c
	VID* m_weaponPtr;        // 0x460
	VID* m_mirror;           // 0x464
	union {
		int m_nLinkDots; // 0x468
		int m_unk0x468;
	};
	union {
		float* m_dotCoords; // 0x46c
		void* m_unk0x46c;
	};
	union {
		int* m_dotFrameStarts; // 0x470
		void* m_unk0x470;
	};
	int m_canMove;           // 0x474
	int m_unk0x478;          // 0x478
	unsigned int m_unk0x47c; // 0x47c
	unsigned int m_prop;     // 0x480

	float m_randomSpeed;
	float m_randomZSpeed;
	unsigned int m_ext1Property = 0;
	unsigned int m_ext2Property = 0;

	const char* GetFileName() const { return m_fname; }
	void SetName(const char* p_name);
	void SetFileName(const char* p_name);
	void SetGridZ(SPRITE* p_sprite);
	void ResetGridZ(SPRITE* p_sprite);

	unsigned int ResetSprites();
	int GetBuildTime();
	VID* SetPropHide(int p_hide);
	int Error(int p_type, const char* p_msg, int p_size);
	void SetChildAndLink();
	unsigned int PropHide();
	unsigned int RealDirection(ANGLE p_dir);
	ANGLE SteppedDirection(ANGLE p_dir) const;
	float CalculateZSpeed(float p_dz, float p_dist);
	int HaveWeapon();
	int GetFireDamage();
	int GetMaxAmmo();
	int DefaultLifetime() const;
	int GetMaxHp(int army);
	unsigned int GetEntitiesNumber(int a2);
	unsigned int GetEntitiesNumberTotal();
	int GetDeathsNumber(int army);
	unsigned int GetDeathsNumberTotal();
	int GetRecolors(int army);
	unsigned int GetRecolorsTotal();
	unsigned int GetNotCreateAsChild();
	unsigned int SetNotCreateAsChild(unsigned int p_value);
	virtual void SetReColorForArmy(unsigned int p_color) {}
	unsigned int IsLight();
	void LoadParameters(RESOURCE* p_res);
	VID* SetMaxHp(int p_army, int p_maxHp);
	void SetHpCoeff(int p_army, int p_coeff);
};

extern VID* EmptyVid;

// SYNTHETIC: ALIEN 0x4136e0
// `dynamic initializer for EmptyVid'

#endif
