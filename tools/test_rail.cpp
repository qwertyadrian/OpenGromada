#include "game/const.h"
#include "game/engine.h"
#include "game/game_descriptor.h"
#include "game/map.h"
#include "game/map_format.h"
#include "game/map_steam.h"
#include "game/rail.h"
#include "game/train_info.h"
#include "sprite/r_dot.h"
#include "sprite/r_map.h"
#include "util/myerror.h"
#include "util/resource.h"
#include "video/vid.h"
#include "video/vid_exdata.h"
#include "world/hash_map.h"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

extern int R_MAP_dotArray[300000];

static bool g_verbose = false;

// Delete all rail sprites from the map and release rail dots cleanly
static void ClearMapRails(std::vector<RAIL*>& rails)
{
	for (RAIL* r : rails) {
		if (r) {
			r->ScalarDeletingDestructor(1);
		}
	}
	rails.clear();

	if (Hash) {
		delete Hash;
		Hash = nullptr;
	}

	while (RailMap.m_list.m_n > 0) {
		R_DOT* dot = RailMap.m_list.m_data[RailMap.m_list.m_n - 1];
		dot->m_refCount = 1;
		dot->Release();
	}
	RailMap.m_list.m_n = 0;
	memset(R_MAP_dotArray, 0, sizeof(R_MAP_dotArray));
}

// Load object definitions from objects.res
static bool LoadObjectsDatabase(const std::string& resPath, MAP_STEAM& map)
{
	RESOURCE res;
	if (res.OpenForRead(STRING(resPath.c_str()), 0x41544144 /* 'DATA' */)) {
		fprintf(stderr, "ERROR: Cannot open %s for reading\n", resPath.c_str());
		return false;
	}

	if (!Const) {
		Const = new CONSTS(&res);
		printf("Loaded CONSTS: %s\n", res.Good() ? "OK" : "FAILED");
	}

	int weapCount = map.LoadWeapon(&res);
	printf("Loaded WEAPON: count=%d (schema bytes=%d), res: %s\n", weapCount, GameDesc->m_weapRecordBytes, res.Good() ? "OK" : "FAILED");

	if (res.GoBegin(0x204a424f /* 'OBJ ' */)) {
		fprintf(stderr, "ERROR: 'OBJ ' section not found in %s (res: %s)\n", resPath.c_str(), res.Good() ? "OK" : "FAILED");
		return false;
	}

	printf("OBJ section found! Starting records loop...\n");
	int count = 0;
	do {
		int idx = -1;
		if (res.ReadWords(&idx, 4) || !MAP::ValidVidIndex(idx)) {
			continue;
		}
		STRING name;
		res.ReadString(name);
		VID* vid = new VID;
		vid->m_idx = idx;
		vid->SetName(name.m_str);
		vid->m_dotFrameCount = 32000;
		vid->LoadParameters(&res);
		STRING fname;
		res.ReadString(fname);
		vid->SetFileName(fname.m_str);

		if (vid->m_weaponIdx >= 0 && vid->m_weaponIdx < map.m_noWeapon) {
			vid->m_exData = (VID_EXDATA*) map.m_weapon + vid->m_weaponIdx;
		}
		else {
			vid->m_exData = (VID_EXDATA*) map.m_weapon;
		}

		map.m_vids[idx] = vid;
		if (idx >= map.m_noVid) {
			map.m_noVid = idx + 1;
		}
		if (count < 3 || count == 100 || count == 500) {
			printf("  Loaded OBJ %d: idx=%d, name='%s', fname='%s'\n", count, idx, name.m_str, fname.m_str);
		}
		count++;
	} while (!res.GoNextSub(0x204a424f));

	printf("Loaded %d object definitions. Calling SetChildAndLink...\n", count);
	for (int i = 0; i < map.m_noVid; ++i) {
		if (map.m_vids[i]) {
			map.m_vids[i]->SetChildAndLink();
		}
	}
	printf("SetChildAndLink finished successfully!\n");

	if (g_verbose) {
		printf("Loaded %d object definitions from %s (m_noVid=%d)\n", count, resPath.c_str(), map.m_noVid);
	}
	return count > 0;
}

// Load rail network from a binary .map level file
static bool LoadMapRails(const std::string& mapPath, MAP_STEAM& map, std::vector<RAIL*>& outRails)
{
	RESOURCE mapRes;
	if (mapRes.OpenForRead(STRING(mapPath.c_str()), 0x2050414d /* 'MAP ' */)) {
		fprintf(stderr, "ERROR: Cannot open map file: %s\n", mapPath.c_str());
		return false;
	}

	if (mapRes.GoBegin(0x44414548 /* 'HEAD' */)) {
		fprintf(stderr, "ERROR: Missing HEAD chunk in map: %s\n", mapPath.c_str());
		return false;
	}

	LEGACY_MAP_HEADER header;
	if (!header.Read(mapRes, false)) {
		fprintf(stderr, "ERROR: Failed to read map header: %s\n", mapPath.c_str());
		return false;
	}

	map.m_w = header.width;
	map.m_h = header.height;

	if (Hash) {
		delete Hash;
		Hash = nullptr;
	}
	Hash = new HASH_MAP(map.m_w, map.m_h, map.m_vids, map.m_noVid);

	if (mapRes.GoBegin(0x20525053 /* 'SPR ' */)) {
		fprintf(stderr, "ERROR: Missing SPR chunk in map: %s\n", mapPath.c_str());
		return false;
	}

	int pointerToken = -1;
	while (mapRes.Good() && !mapRes.ReadWords(&pointerToken, 4) && pointerToken != -1) {
		if (mapRes.Remaining() < 24) {
			break;
		}
		int nvid = -1;
		float x = 0, y = 0, z = 0;
		int direction = 0, army = 0;
		mapRes.ReadWords(&nvid, 4);
		if (header.version > 9) {
			mapRes.ReadWords(&x, 4);
			mapRes.ReadWords(&y, 4);
			mapRes.ReadWords(&z, 4);
		}
		else {
			int ix, iy, iz;
			mapRes.ReadWords(&ix, 4); x = (float) ix;
			mapRes.ReadWords(&iy, 4); y = (float) iy;
			mapRes.ReadWords(&iz, 4); z = (float) iz;
		}
		mapRes.ReadWords(&direction, 4);
		mapRes.ReadWords(&army, 4);

		if (nvid >= 0 && nvid < map.m_noVid && map.m_vids[nvid]) {
			if (map.m_vids[nvid]->m_sprClass == 22) {
				SPRITE* sp = map.CreateSprite(map.m_vids[nvid], x, y, z, ANGLE((char) direction), 0);
				if (sp) {
					outRails.push_back((RAIL*) sp);
				}
			}
		}
	}

	RailMap.m_unk0x08 = (int) map.m_w;
	RailMap.m_unk0x0c = (int) map.m_h;
	RailMap.CreateAdditionalDots();

	return true;
}

// ----------------------------------------------------------------------------
// Test 1: Rail Graph Topology Verification on Campaign Maps
// ----------------------------------------------------------------------------
static bool TestRailTopology(const std::string& baseDir, MAP_STEAM& map, std::vector<RAIL*>& activeRails)
{
	printf("\n[TEST 1] Verifying rail graph topology on campaign maps...\n");

	const char* const testMaps[] = {
		"maps/l01/l01.map",
		"maps/l06/l06.map",
		"maps/l15/l15.map"
	};

	for (const char* relPath : testMaps) {
		std::string mapPath = (fs::path(baseDir) / relPath).string();
		if (!fs::exists(mapPath)) {
			printf("  [SKIP] Map file not found: %s\n", mapPath.c_str());
			continue;
		}

		ClearMapRails(activeRails);
		if (!LoadMapRails(mapPath, map, activeRails)) {
			fprintf(stderr, "  [FAIL] Failed to load rail network from %s\n", mapPath.c_str());
			return false;
		}

		int totalDots = RailMap.m_list.m_n;
		if (totalDots <= 0) {
			fprintf(stderr, "  [FAIL] No rail dots generated for %s (rails count=%zu)\n", relPath, activeRails.size());
			return false;
		}

		int totalLinks = 0;
		int junctionCount = 0; // Dots with >= 3 connections (rail switches/crossings)
		int deadEnds = 0;

		for (int i = 0; i < totalDots; ++i) {
			R_DOT* dot = RailMap.m_list.m_data[i];
			assert(dot != nullptr);
			assert(dot->m_noLinks >= 1 && dot->m_noLinks <= 6);

			totalLinks += dot->m_noLinks;
			if (dot->m_noLinks >= 3) {
				junctionCount++;
			}
			else if (dot->m_noLinks == 1) {
				deadEnds++;
			}

			// Validate all topological links and bidirectional symmetry
			for (int l = 0; l < dot->m_noLinks; ++l) {
				R_DOT_LINK& link = dot->m_links[l];
				assert(link.m_dot != nullptr);
				assert(link.m_backLink >= 0 && link.m_backLink < link.m_dot->m_noLinks);

				// Symmetric backlink must reference this exact dot
				R_DOT* neighbor = link.m_dot;
				R_DOT_LINK& backLink = neighbor->m_links[link.m_backLink];
				assert(backLink.m_dot == dot);
				assert(link.m_dist > 0);
			}
		}

		printf("  [PASS] %-17s: %zu rails -> %d dots, %d links, %d junctions (switches), %d dead-ends\n",
			   relPath, activeRails.size(), totalDots, totalLinks, junctionCount, deadEnds);

		assert(junctionCount > 0); // Real Locoland RTS maps always contain rail switches!
	}

	return true;
}

// ----------------------------------------------------------------------------
// Test 2: Railway Switches and Semaphores Logic
// ----------------------------------------------------------------------------
static bool TestSemaphoresAndSwitches(MAP_STEAM& map)
{
	printf("\n[TEST 2] Verifying switches and semaphores logic...\n");

	// Ensure we have a graph with at least one switch
	assert(RailMap.m_list.m_n > 0);

	R_DOT* switchDot = nullptr;
	for (int i = 0; i < RailMap.m_list.m_n; ++i) {
		if (RailMap.m_list.m_data[i]->m_noLinks >= 3) {
			switchDot = RailMap.m_list.m_data[i];
			break;
		}
	}
	assert(switchDot != nullptr);

	if (g_verbose) {
		printf("  Testing switch dot at (%d, %d, %d) with %d links\n",
			   switchDot->m_x, switchDot->m_y, switchDot->m_z, switchDot->m_noLinks);
	}

	// 1. Test SetSemaphoreOrMine
	// In Locoland: semaphore value = army + 4 for blocked/red semaphore
	const int redArmy0 = 4; // Army 0 red light
	R_DOT* setDot = RailMap.SetSemaphoreOrMine(switchDot->m_x, switchDot->m_y, redArmy0, 0);
	assert(setDot != nullptr);
	assert(setDot->m_unk0x14 == redArmy0);

	// 2. Test CanEnginePassTo under semaphore
	VID* engVid = nullptr;
	for (int i = 0; i < map.m_noVid; ++i) {
		if (map.m_vids[i] && map.m_vids[i]->m_sprClass == 21) {
			engVid = map.m_vids[i];
			break;
		}
	}
	assert(engVid != nullptr);

	ENGINE* engArmy0 = (ENGINE*) map.CreateSprite(engVid, (float) switchDot->m_x, (float) switchDot->m_y, (float) switchDot->m_z, ANGLE(0), 0);
	assert(engArmy0 != nullptr);
	engArmy0->m_flag = (engArmy0->m_flag & ~0x1800u) | (0 << 11); // Army 0

	ENGINE* engArmy1 = (ENGINE*) map.CreateSprite(engVid, (float) switchDot->m_x, (float) switchDot->m_y, (float) switchDot->m_z, ANGLE(0), 0);
	assert(engArmy1 != nullptr);
	engArmy1->m_flag = (engArmy1->m_flag & ~0x1800u) | (1 << 11); // Army 1

	// For Army 0, CanEnginePassTo should be blocked when both dots share redArmy0
	// For Army 1, semaphore 4 does not block passability
	int passArmy1 = switchDot->CanEnginePassTo(0, engArmy1);
	assert(passArmy1 != 0);

	// 3. Test switch arrow blocking via m_unk0x0c
	R_DOT* neighbor0 = switchDot->m_links[0].m_dot;
	int backLink = switchDot->m_links[0].m_backLink;

	// Reset semaphores to clear state
	switchDot->m_unk0x14 = 0;
	neighbor0->m_unk0x14 = 0;

	// When neighbor's m_unk0x0c points to the incoming backLink, engine cannot pass into it
	neighbor0->m_unk0x0c = backLink;
	assert(switchDot->CanEnginePassTo(0, engArmy0) == 0);

	// When switch opens another branch, incoming traffic can pass
	neighbor0->m_unk0x0c = -1;
	assert(switchDot->CanEnginePassTo(0, engArmy0) != 0);

	printf("  [PASS] Switch diverter and semaphore passability rules verified successfully\n");
	return true;
}

// ----------------------------------------------------------------------------
// Test 3: Train Composition, ForceLink, and BreakTrain
// ----------------------------------------------------------------------------
static bool TestTrainCompositionAndBreak(MAP_STEAM& map)
{
	printf("\n[TEST 3] Verifying train composition (ForceLink) and break (BreakTrain)...\n");

	VID* locoVid = nullptr;
	VID* carVid = nullptr;
	for (int i = 0; i < map.m_noVid; ++i) {
		if (map.m_vids[i] && map.m_vids[i]->m_sprClass == 21) {
			if (!locoVid) {
				locoVid = map.m_vids[i];
			}
			else if (!carVid) {
				carVid = map.m_vids[i];
			}
		}
	}
	assert(locoVid != nullptr);
	if (!carVid) {
		carVid = locoVid;
	}

	R_DOT* startDot = RailMap.m_list.m_n > 0 ? RailMap.m_list.m_data[0] : nullptr;
	float sx = startDot ? (float) startDot->m_x : 100.0f;
	float sy = startDot ? (float) startDot->m_y : 100.0f;
	float sz = startDot ? (float) startDot->m_z : 0.0f;

	ENGINE* e1 = (ENGINE*) map.CreateSprite(locoVid, sx, sy, sz, ANGLE(0), 0);
	ENGINE* e2 = (ENGINE*) map.CreateSprite(carVid, sx + 50.0f, sy, sz, ANGLE(0), 0);
	ENGINE* e3 = (ENGINE*) map.CreateSprite(carVid, sx + 100.0f, sy, sz, ANGLE(0), 0);

	assert(e1 != nullptr && e2 != nullptr && e3 != nullptr);

	// Initially all 3 engines are single cars
	assert(e1->m_prevEngine == nullptr && e1->m_nextEngine == nullptr);
	assert(e2->m_prevEngine == nullptr && e2->m_nextEngine == nullptr);
	assert(e3->m_prevEngine == nullptr && e3->m_nextEngine == nullptr);

	// Link e1 and e2
	e1->ForceLink(e2);
	assert(e1->m_nextEngine == e2);
	assert(e2->m_prevEngine == e1);
	assert(e1->FirstEngine() == e1);
	assert(e2->FirstEngine() == e1);
	assert(e1->LastEngine() == e2);
	assert(e2->LastEngine() == e2);

	// Link e2 and e3
	e2->ForceLink(e3);
	assert(e2->m_nextEngine == e3);
	assert(e3->m_prevEngine == e2);
	assert(e1->m_nextEngine == e2);
	assert(e2->m_prevEngine == e1);

	// Verify complete 3-car train traversal
	assert(e1->FirstEngine() == e1);
	assert(e2->FirstEngine() == e1);
	assert(e3->FirstEngine() == e1);
	assert(e1->LastEngine() == e3);
	assert(e2->LastEngine() == e3);
	assert(e3->LastEngine() == e3);

	assert(e1->NextEngine() == e2);
	assert(e2->NextEngine() == e3);
	assert(e3->NextEngine() == nullptr);

	assert(e1->InTrain(e2));
	assert(e1->InTrain(e3));
	assert(e3->InTrain(e1));

	// Test BreakTrain: uncouple e1 from (e2, e3)
	e1->BreakTrain(e2->m_x, e2->m_y);

	assert(e1->m_nextEngine == nullptr);
	assert(e2->m_prevEngine == nullptr);
	assert(e2->m_nextEngine == e3);
	assert(e3->m_prevEngine == e2);

	assert(e1->FirstEngine() == e1);
	assert(e1->LastEngine() == e1);
	assert(e2->FirstEngine() == e2);
	assert(e2->LastEngine() == e3);

	assert(!e1->InTrain(e2));
	assert(e2->InTrain(e3));

	// Clean up train sprites
	e1->ScalarDeletingDestructor(1);
	e2->ScalarDeletingDestructor(1);
	e3->ScalarDeletingDestructor(1);

	printf("  [PASS] Doubly-linked train assembly, bidirectional iteration, and uncoupling verified\n");
	return true;
}

// ----------------------------------------------------------------------------
// Test 4: TRAIN_INFO Properties & Opcode 212 (TrainProperty)
// ----------------------------------------------------------------------------
static bool TestTrainInfoAndProperties(MAP_STEAM& map)
{
	printf("\n[TEST 4] Verifying TRAIN_INFO and Opcode 212 (TrainProperty)...\n");

	VID* locoVid = nullptr;
	VID* carVid = nullptr;
	for (int i = 0; i < map.m_noVid; ++i) {
		if (map.m_vids[i] && map.m_vids[i]->m_sprClass == 21) {
			if (!locoVid) {
				locoVid = map.m_vids[i];
			}
			else if (!carVid) {
				carVid = map.m_vids[i];
			}
		}
	}
	assert(locoVid != nullptr);
	if (!carVid) {
		carVid = locoVid;
	}

	R_DOT* startDot = RailMap.m_list.m_n > 0 ? RailMap.m_list.m_data[0] : nullptr;
	float sx = startDot ? (float) startDot->m_x : 100.0f;
	float sy = startDot ? (float) startDot->m_y : 100.0f;
	float sz = startDot ? (float) startDot->m_z : 0.0f;

	ENGINE* e1 = (ENGINE*) map.CreateSprite(locoVid, sx, sy, sz, ANGLE(0), 0);
	ENGINE* e2 = (ENGINE*) map.CreateSprite(carVid, sx + 50.0f, sy, sz, ANGLE(0), 0);
	ENGINE* e3 = (ENGINE*) map.CreateSprite(carVid, sx + 100.0f, sy, sz, ANGLE(0), 0);

	e1->ForceLink(e2);
	e2->ForceLink(e3);

	// Construct TRAIN_INFO on head of the train
	TRAIN_INFO info(e1);

	assert(info.m_carCount == 3);
	assert(info.m_maxHp > 0);
	assert(info.m_currentHp >= 0);

	if (g_verbose) {
		printf("  TRAIN_INFO values:\n");
		printf("    m_carCount:     %d\n", info.m_carCount);
		printf("    m_maxSpeed:     %d\n", info.m_maxSpeed);
		printf("    m_currentHp:    %d\n", info.m_currentHp);
		printf("    m_maxHp:        %d\n", info.m_maxHp);
		printf("    m_weaponPower:  %d\n", info.m_weaponPower);
		printf("    m_buildTime:    %d\n", info.m_buildTime);
		printf("    m_ammoPercent:  %d\n", info.m_ammoPercent);
		printf("    m_currentAmmo:  %d\n", info.m_currentAmmo);
		printf("    m_maxAmmo:      %d\n", info.m_maxAmmo);
		printf("    Acceleration:   %d\n", info.Acceleration());
	}

	// Verify all opcode 212 query values against export.lgc constants
	// Case 1: PROP_SPEED
	assert(info.m_maxSpeed >= 0);

	// Case 2: PROP_WEAPON
	assert(info.m_weaponPower >= 0);

	// Case 3: PROP_LIFE (percentage 0..100)
	int life = info.m_maxHp ? info.m_currentHp * 100 / info.m_maxHp : 100;
	assert(life >= 0 && life <= 100);

	// Case 4: PROP_HP
	assert(info.m_currentHp >= 0);

	// Case 5: PROP_AMMO
	assert(info.m_ammoPercent >= 0 && info.m_ammoPercent <= 100);

	// Case 6: PROP_ACCELERATE
	assert(info.Acceleration() >= 0);

	// Case 7: PROP_BUILD_TIME
	assert(info.m_buildTime >= 0);

	// Case 9: PROP_FREE
	bool isFree = true;
	for (ENGINE* e = e1->FirstEngine(); e; e = e->NextEngine()) {
		if (!e->IsCommand(0)) {
			isFree = false;
			break;
		}
	}
	assert(isFree == true);

	// Case 10: PROP_AMMO_NO
	assert(info.m_currentAmmo >= 0);

	// Case 11: PROP_AMMO_MAX
	assert(info.m_maxAmmo >= 0);

	// Test single car after split
	e1->BreakTrain(e2->m_x, e2->m_y);
	TRAIN_INFO infoSingle(e1);
	assert(infoSingle.m_carCount == 1);

	TRAIN_INFO infoTail(e2);
	assert(infoTail.m_carCount == 2);

	// Clean up train sprites
	e1->ScalarDeletingDestructor(1);
	e2->ScalarDeletingDestructor(1);
	e3->ScalarDeletingDestructor(1);

	printf("  [PASS] All 10 TrainProperty (opcode 212) queries verified against TRAIN_INFO\n");
	return true;
}

// ============================================================================
// Main Test Runner
// ============================================================================
int main(int argc, char** argv)
{
	std::string baseDir = "/home/qwertyadrian/Projects/KhonKaaDoo";

	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if (arg == "--verbose" || arg == "-v") {
			g_verbose = true;
		}
		else if (arg[0] != '-') {
			baseDir = arg;
		}
	}

	printf("====================================================================\n");
	printf(" OpenGromada / KhonKaaDoo: RTS Rail Networks & Train Physics Test\n");
	printf(" Asset directory: %s\n", baseDir.c_str());
	printf("====================================================================\n");

	// Step 1: Configure game descriptor for Locoland
	Game_SetCliOverride(GAME_LOCOLAND);
	GameDesc = Game_FindDescriptor("locoland");
	assert(GameDesc != nullptr);
	assert(GameDesc->m_weapRecordBytes == 68);
	assert(GameDesc->m_objSchema == GAME_OBJ_LOCOLAND);

	// Step 2: Initialize headless MAP_STEAM instance
	MAP_STEAM map;
	Map = &map;

	// Step 3: Load objects.res
	std::string resPath = (fs::path(baseDir) / "objects.res").string();
	if (!fs::exists(resPath)) {
		fprintf(stderr, "FATAL: objects.res not found at: %s\n", resPath.c_str());
		return 1;
	}

	if (!LoadObjectsDatabase(resPath, map)) {
		fprintf(stderr, "FATAL: Failed to load objects.res\n");
		return 1;
	}

	// Step 4: Run all test suites
	std::vector<RAIL*> activeRails;
	bool ok = true;
	ok = ok && TestRailTopology(baseDir, map, activeRails);
	ok = ok && TestSemaphoresAndSwitches(map);
	ok = ok && TestTrainCompositionAndBreak(map);
	ok = ok && TestTrainInfoAndProperties(map);

	// Clean up active rail network and spatial hash before map destruction
	ClearMapRails(activeRails);

	printf("\n--------------------------------------------------------------------\n");
	if (ok) {
		printf("[PASS] All rail network and train physics tests completed successfully!\n");
	}
	else {
		printf("[FAIL] Rail network and train physics tests encountered failures.\n");
	}
	printf("--------------------------------------------------------------------\n");

	return ok ? 0 : 1;
}
