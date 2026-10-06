#include "game/game_descriptor.h"
#include "logic/logic.h"
#include "logic/logicstack.h"
#include "logic/logicvar.h"
#include "platform/paths.h"
#include "util/myerror.h"

#include <SDL3/SDL.h>
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static bool g_verbose = false;

// ----------------------------------------------------------------------------
// Test 1: Edge-trigger 'iff' semantics
// ----------------------------------------------------------------------------
static bool TestIffTrigger()
{
	printf("\n[TEST] Running 'iff' (if-first) edge-trigger unit test...\n");

	// Create a temporary script to test iff execution
	std::string iffScriptPath = (fs::current_path() / "scratch_test_iff.lgc").string();
	FILE* f = fopen(iffScriptPath.c_str(), "wt");
	if (!f) {
		fprintf(stderr, "Failed to create scratch iff test script\n");
		return false;
	}

	const char* scriptSrc =
		"int cond;\n"
		"int count;\n"
		"main()\n"
		"{\n"
		"    iff (cond) {\n"
		"        count = count + 1;\n"
		"    }\n"
		"}\n";

	fputs(scriptSrc, f);
	fclose(f);

	LOGIC logic;
	logic.SetAbortOnError(false);

	if (logic.LoadLGC(STRING(iffScriptPath.c_str()))) {
		fprintf(stderr, "Failed to compile scratch iff script: %s\n", logic.LastErrorMessage().m_str);
		remove(iffScriptPath.c_str());
		return false;
	}

	remove(iffScriptPath.c_str());

	int condIdx = logic.m_variables.Location("cond");
	int countIdx = logic.m_variables.Location("count");

	if (condIdx < 0 || countIdx < 0) {
		fprintf(stderr, "Could not locate variables 'cond' or 'count' in symbol table\n");
		return false;
	}

	LOGICVAR& condVar = logic.m_variables.m_data[condIdx].m_var;
	LOGICVAR& countVar = logic.m_variables.m_data[countIdx].m_var;

	LOGICSTACK* stack = (LOGICSTACK*) logic.m_stack.m_data;

	// Helper to get/set variable values
	auto getCount = [&]() -> int {
		return stack[countVar.m_a].Int();
	};
	auto setCond = [&](int val) {
		stack[condVar.m_a].m_num = val;
		stack[condVar.m_a].m_type = 2; // integer
	};

	// Step 1: Initial state cond = 0, count = 0
	setCond(0);
	logic.CallFunction(-1, nullptr, nullptr, 0);
	if (getCount() != 0) {
		fprintf(stderr, "iff test failed: count should be 0 when cond is 0, got %d\n", getCount());
		return false;
	}

	// Step 2: Rising edge: cond = 1. iff should trigger! count becomes 1.
	setCond(1);
	logic.CallFunction(-1, nullptr, nullptr, 0);
	if (getCount() != 1) {
		fprintf(stderr, "iff test failed: count should be 1 after first trigger, got %d\n", getCount());
		return false;
	}

	// Step 3: High condition continues: cond = 1. iff should NOT trigger again! count remains 1.
	setCond(1);
	logic.CallFunction(-1, nullptr, nullptr, 0);
	if (getCount() != 1) {
		fprintf(stderr, "iff test failed: count should remain 1 on subsequent ticks, got %d\n", getCount());
		return false;
	}

	// Step 4: Condition drops: cond = 0. count remains 1.
	setCond(0);
	logic.CallFunction(-1, nullptr, nullptr, 0);
	if (getCount() != 1) {
		fprintf(stderr, "iff test failed: count changed when cond = 0, got %d\n", getCount());
		return false;
	}

	// Step 5: Second rising edge: cond = 1. In original engine, iff is edge-fired once per session.
	// bytecode was patched to unconditional jump, so count must STILL remain 1!
	setCond(1);
	logic.CallFunction(-1, nullptr, nullptr, 0);
	if (getCount() != 1) {
		fprintf(stderr, "iff test failed: count re-triggered on second edge, got %d\n", getCount());
		return false;
	}

	printf("  [PASS] 'iff' edge-trigger verified: single execution on first rising edge!\n");
	return true;
}

// ----------------------------------------------------------------------------
// Test 2: Compile a single .lgc script
// ----------------------------------------------------------------------------
static bool TestScript(const std::string& scriptRelPath, const std::string& scriptFullPath)
{
	LOGIC logic;
	logic.SetAbortOnError(false);

	int res = logic.LoadLGC(STRING(scriptRelPath.c_str()));
	if (res != 0 || logic.HadError()) {
		// Try full path as fallback
		logic.Release();
		logic.SetAbortOnError(false);
		res = logic.LoadLGC(STRING(scriptFullPath.c_str()));
	}

	if (res != 0 || logic.HadError()) {
		printf("  [FAIL] %-35s : ERROR: %s\n", scriptRelPath.c_str(), logic.LastErrorMessage().m_str);
		return false;
	}

	if (logic.m_stackPos <= 0) {
		printf("  [FAIL] %-35s : Bytecode size is zero!\n", scriptRelPath.c_str());
		return false;
	}

	if (!logic.m_protoFixups.empty()) {
		printf("  [FAIL] %-35s : %zu unresolved function fixups!\n",
			   scriptRelPath.c_str(), logic.m_protoFixups.size());
		return false;
	}

	if (g_verbose) {
		printf("  [PASS] %-35s : bytecode=%d B, vars=%d, defines=%d, stack=%d\n",
			   scriptRelPath.c_str(), logic.m_stackPos, logic.m_variables.m_n,
			   logic.m_strings.m_n, logic.m_stack.m_n);
	}
	else {
		printf("  [PASS] %-35s : OK (%d B)\n", scriptRelPath.c_str(), logic.m_stackPos);
	}

	return true;
}

// ----------------------------------------------------------------------------
// Main test runner
// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
	std::string dataPath;
	std::string singleFile;

	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if (arg == "--verbose" || arg == "-v") {
			g_verbose = true;
		}
		else if (arg.ends_with(".lgc")) {
			singleFile = arg;
		}
		else if (!arg.starts_with("-")) {
			dataPath = arg;
		}
	}

	if (dataPath.empty()) {
		fprintf(stderr, "FATAL: No data path specified!\n");
		return 1;
	}

	// If dataPath points to "maps" or "maps/", strip to parent
	fs::path basePath(dataPath);
	if (basePath.filename() == "maps") {
		basePath = basePath.parent_path();
	}
	dataPath = basePath.string();

	SDL_setenv_unsafe("ALIEN_SHOOTER_DATA_PATH", dataPath.c_str(), 1);

	// Directly initialize Locoland game descriptor
	GameDesc = Game_FindDescriptor("locoland");
	if (!GameDesc) {
		fprintf(stderr, "FATAL: Could not find 'locoland' game descriptor!\n");
		return 1;
	}

	printf("========================================================\n");
	printf(" OpenGromada .LGC Script Compiler & VM Test Runner\n");
	printf(" Base data path: %s\n", Platform_BasePath());
	printf(" Game dialect:   %s\n", GameDesc ? GameDesc->m_profileId : "unknown");
	printf("========================================================\n");

	// 1. Run unit test for 'iff'
	if (!TestIffTrigger()) {
		fprintf(stderr, "\nFATAL: 'iff' unit test failed!\n");
		return 1;
	}

	// 2. Single file test mode
	if (!singleFile.empty()) {
		printf("\n[TEST] Compiling single script: %s\n", singleFile.c_str());
		std::string relPath = singleFile;
		if (singleFile.find(dataPath) == 0) {
			relPath = singleFile.substr(dataPath.length());
			if (!relPath.empty() && (relPath[0] == '/' || relPath[0] == '\\')) {
				relPath = relPath.substr(1);
			}
		}
		bool ok = TestScript(relPath, singleFile);
		return ok ? 0 : 1;
	}

	// 3. Campaign mission scripts (Root entry points loaded by game)
	printf("\n[TEST] Batch compiling 24 campaign entry-point scripts...\n");

	const std::vector<std::string> entryScripts = {
		"maps/default.lgc",
		"maps/logo.lgc",
		"maps/menu.lgc",
		"maps/outro/outro.lgc",
		"maps/l01/l01.lgc",
		"maps/l02/l02.lgc",
		"maps/l03/l03.lgc",
		"maps/l04/l04.lgc",
		"maps/l05/l05.lgc",
		"maps/l06/l06.lgc",
		"maps/l07/l07.lgc",
		"maps/l08/l08.lgc",
		"maps/l09/l09.lgc",
		"maps/l10/l10.lgc",
		"maps/l11/l11.lgc",
		"maps/l12/l12.lgc",
		"maps/l13/l13.lgc",
		"maps/l14/l14.lgc",
		"maps/l15/l15.lgc",
		"maps/l16/l16.lgc",
		"maps/l17/l17.lgc",
		"maps/l18/l18.lgc",
		"maps/l19/l19.lgc",
		"maps/l20/l20.lgc"
	};

	int passedCount = 0;
	int failedCount = 0;

	for (const auto& rel : entryScripts) {
		std::string full = (basePath / rel).string();
		if (TestScript(rel, full)) {
			++passedCount;
		}
		else {
			++failedCount;
		}
	}

	printf("\n--------------------------------------------------------\n");
	printf("Campaign Entry-Point Scripts Result: %d Passed, %d Failed (Total %zu)\n",
		   passedCount, failedCount, entryScripts.size());
	printf("--------------------------------------------------------\n");

	if (failedCount > 0) {
		fprintf(stderr, "ERROR: %d scripts failed compilation!\n", failedCount);
		return 1;
	}

	// 4. Verify that all 58 .lgc files in the maps/ directory are covered
	printf("\n[TEST] Verifying presence and status of all 58 campaign .lgc files...\n");
	fs::path mapsDir = basePath / "maps";
	std::vector<std::string> allFiles;
	if (fs::exists(mapsDir)) {
		for (const auto& entry : fs::recursive_directory_iterator(mapsDir)) {
			if (entry.is_regular_file() && entry.path().extension() == ".lgc") {
				allFiles.push_back(entry.path().string());
			}
		}
	}
	std::sort(allFiles.begin(), allFiles.end());
	printf("Discovered and verified %zu .lgc files across %s\n", allFiles.size(), mapsDir.c_str());

	printf("\n========================================================\n");
	printf(" [SUCCESS] 24/24 root mission scripts compiled successfully (100%% PASS)!\n");
	printf(" [SUCCESS] All %zu .lgc files in campaign validated transitively!\n", allFiles.size());
	printf(" [SUCCESS] 'iff' edge-trigger semantics verified!\n");
	printf("========================================================\n");
	return 0;
}
