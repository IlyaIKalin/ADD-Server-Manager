#!/usr/bin/env python3
"""Run the actual FPS method bodies via a small C++ compatibility adapter.

This checks arithmetic and callback flow, not DayZ/Enforce compilation.
"""
import hashlib
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from build_engine_fps import build
from extract_release import extract_pbo

SOURCE = ROOT / "variants/engine-fps/addon/Scripts/3_Game/ADD_Server_Manager_FPS/ADD_EngineFps.c"


class EngineFpsTests(unittest.TestCase):
    def test_server_counters_with_expansion_limiter(self):
        source = SOURCE.read_text()
        source = source.replace("protected static", "public: inline static")
        source = source.replace("out float ", "float& ")
        source = source.replace("modded class DayZGame", "class DayZGame : public ExpansionGame")
        source = source.replace("super.OnUpdate", "ExpansionGame::OnUpdate")
        source = source.replace("override void", "public: void")
        source = source.replace("ADD_EngineFps.", "ADD_EngineFps::")
        source = re.sub(r"^}$", "};", source, flags=re.MULTILINE)
        harness = r'''
#include <cassert>
#include <cmath>
#include <iostream>
float clockTime = 0;
float GetTickTime() { return clockTime; }
class ExpansionGame {
public:
    int missionFrames = 0;
    float interval = 0.01f, last = 0;
    void OnUpdate(bool, float) {
        if (clockTime-last >= interval) {
            ++missionFrames;
            last = clockTime;
        }
    }
};
'''
        tests = r'''
int main() {
    float lo, avg, hi;
    ADD_EngineFps::TakeFps(lo, avg, hi);
    assert(lo == 0 && avg == 0 && hi == 0);
    DayZGame game;
    // 12000 engine callbacks/s, mission limited to approximately 100/s.
    for (int i=1; i<=36000; ++i) {
        clockTime = i/12000.0f;
        game.OnUpdate(true, 0); // quantized timeslice does not affect FPS
    }
    ADD_EngineFps::TakeFps(lo, avg, hi);
    assert(std::fabs(avg-12000) < 2);
    assert(lo > 11900 && hi < 12100);
    assert(game.missionFrames > 250 && game.missionFrames <= 301);
    // Change only the script limiter. Raw FPS must remain unchanged.
    int oldMissionFrames = game.missionFrames;
    game.interval = 0.002f;
    for (int i=1; i<=36000; ++i) {
        clockTime = 3+i/12000.0f;
        game.OnUpdate(true, 0);
    }
    ADD_EngineFps::TakeFps(lo, avg, hi);
    assert(std::fabs(avg-12000) < 2);
    assert(game.missionFrames-oldMissionFrames > 1300);
    // Real time weighting: 100 FPS for 2 seconds, 200 FPS for 2 seconds.
    for (int i=1; i<=200; ++i) {
        clockTime=6+i/100.0f; game.OnUpdate(true, 0);
    }
    for (int i=1; i<=400; ++i) {
        clockTime=8+i/200.0f; game.OnUpdate(true, 0);
    }
    ADD_EngineFps::TakeFps(lo, avg, hi);
    assert(std::fabs(avg-150) < 0.1);
    assert(lo > 98 && lo < 102 && hi > 198 && hi < 202);
    // A short heartbeat uses its actual duration and falls back to avg for extrema.
    for (int i=1; i<=200; ++i) {
        clockTime=10+i/1000.0f; game.OnUpdate(true, 0);
    }
    ADD_EngineFps::TakeFps(lo, avg, hi);
    assert(std::fabs(avg-1000) < 1 && lo == avg && hi == avg);
    // The partial 1000-FPS window must not contaminate the next heartbeat.
    for (int i=1; i<=200; ++i) {
        clockTime=10.2f+i/100.0f; game.OnUpdate(true, 0);
    }
    ADD_EngineFps::TakeFps(lo, avg, hi);
    assert(std::fabs(avg-100) < 0.1 && hi < 102);
    ADD_EngineFps::TakeFps(lo, avg, hi);
    assert(avg == 0 && lo == 0 && hi == 0);
    // A long stall must lower the measured FPS.
    clockTime += 5;
    game.OnUpdate(true, 0);
    ADD_EngineFps::TakeFps(lo, avg, hi);
    assert(std::fabs(avg-0.2f) < 0.001 && lo == avg && hi == avg);
    std::cout << "FPS arithmetic and limited/raw callback separation: PASS\n";
}
'''
        with tempfile.TemporaryDirectory() as td:
            cpp = Path(td) / "fps.cpp"
            exe = Path(td) / "fps"
            cpp.write_text(harness + source + tests)
            subprocess.run(["g++", "-std=c++17", "-DSERVER", str(cpp), "-o", str(exe)], check=True)
            subprocess.run([str(exe)], check=True)

    def test_client_has_no_counter_or_hook(self):
        result = subprocess.run(["g++", "-E", "-P", "-x", "c++", str(SOURCE)],
                                check=True, capture_output=True, text=True)
        self.assertEqual(result.stdout.strip(), "")

    def test_packaging_and_reference_preservation(self):
        archive = build()
        self.assertTrue(archive.exists())
        overlay_path = "Scripts/4_World/ADD_Server_Manager/Server/ADD_Ticker.c"
        ticker = (ROOT / "variants/engine-fps/overlay" / overlay_path).read_text()
        self.assertNotIn("CountFrame();", ticker)
        self.assertIn("ADD_EngineFps.TakeFps", ticker)
        with tempfile.TemporaryDirectory() as td:
            out = Path(td)
            mod = archive.parent / "@ADD_Server_Manager_FPS/addons"
            for name, prefix in [("ADD_Server_Manager", "ADD_Server_Manager"),
                                 ("ADD_Server_Manager_FPS", "ADD_Server_Manager_FPS")]:
                data = (mod / f"{name}.pbo").read_bytes()
                self.assertEqual(data[-21], 0)
                self.assertEqual(data[-20:], hashlib.sha1(data[:-21]).digest())
                target = out / name
                target.mkdir()
                extract_pbo(data, target)
            restored = out / "ADD_Server_Manager/game-mod"
            base = ROOT / "reference/v0.9.91/game-mod"
            changed = [p.relative_to(base).as_posix() for p in base.rglob("*")
                       if p.is_file() and p.read_bytes() != (restored / p.relative_to(base)).read_bytes()]
            self.assertEqual(changed, [overlay_path])
            config = (out / "ADD_Server_Manager_FPS/game-mod/config.cpp").read_text()
            self.assertIn('"DayZExpansion_Core_Scripts"', config)
            self.assertIn('"ADD_Server_Manager"', config)
        before = archive.read_bytes()
        self.assertEqual(before, build().read_bytes())


if __name__ == "__main__":
    unittest.main()
