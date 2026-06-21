#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#include <algorithm>
#include <cwchar>
#include <filesystem>

#include "Config.hpp"
#include "Pet.hpp"
#include "PetWindow.hpp"
#include "SpriteSheet.hpp"
#include "Stats.hpp"
#include "TrayIcon.hpp"

namespace {
    constexpr UINT IDI_APP_ICON = 101;

    PetWindow*   g_Window = nullptr;
    TrayIcon*    g_Tray   = nullptr;
    Pet*         g_Pet    = nullptr;

    LARGE_INTEGER g_QpcFreq{};
    float g_TimeSinceSave = 0.0f;

    void SaveNow() {
        if (g_Pet) StatsStore::Save(g_Pet->SnapshotStats());
    }

    void UpdateTrayTooltip() {
        if (!g_Tray || !g_Pet) return;
        const PetStats& s = g_Pet->Stats();
        wchar_t buf[128];
        swprintf_s(buf, L"Hunger: %d  Happiness: %d  Energy: %d",
                   static_cast<int>(s.hunger),
                   static_cast<int>(s.happiness),
                   static_cast<int>(s.energy));
        g_Tray->UpdateTooltip(buf);
    }

    std::filesystem::path ExeDir() {
        wchar_t buf[MAX_PATH];
        const DWORD n = GetModuleFileNameW(nullptr, buf, MAX_PATH);
        if (n == 0 || n >= MAX_PATH) return {};
        return std::filesystem::path(buf).parent_path();
    }

    bool OnMessage(UINT msg, WPARAM wp, LPARAM lp) {
        switch (msg) {
            case WM_NCLBUTTONDOWN:
                if (wp == HTCAPTION && g_Pet) {
                    g_Pet->PetIt();
                    UpdateTrayTooltip();
                }
                return false; // let DefWindowProc handle the drag

            case TrayIcon::kCallbackMessage:
                if (LOWORD(lp) == WM_RBUTTONUP || LOWORD(lp) == WM_CONTEXTMENU) {
                    if (g_Tray && g_Pet) {
                        const PetStats& s = g_Pet->Stats();
                        wchar_t statsLine[128];
                        swprintf_s(statsLine, L"Hunger: %d  Happiness: %d  Energy: %d",
                                   static_cast<int>(s.hunger),
                                   static_cast<int>(s.happiness),
                                   static_cast<int>(s.energy));
                        g_Tray->ShowContextMenu(statsLine);
                    }
                }
                return true;

            case WM_COMMAND: {
                const UINT id = LOWORD(wp);
                if (!g_Pet) return false;
                switch (id) {
                    case TrayIcon::kCmdFeed:
                        g_Pet->Feed();
                        UpdateTrayTooltip();
                        return true;
                    case TrayIcon::kCmdPet:
                        g_Pet->PetIt();
                        UpdateTrayTooltip();
                        return true;
                    case TrayIcon::kCmdSleep:
                        g_Pet->PutToSleep();
                        UpdateTrayTooltip();
                        return true;
                    case TrayIcon::kCmdNextMonitor:
                        if (g_Window) g_Window->MoveToNextMonitor(40, 60);
                        return true;
                    case TrayIcon::kCmdQuit:
                        SaveNow();
                        if (g_Tray) g_Tray->Remove();
                        PostQuitMessage(0);
                        return true;
                }
                return false;
            }

            case WM_DESTROY:
                SaveNow();
                if (g_Tray) g_Tray->Remove();
                PostQuitMessage(0);
                return true;
        }
        return false;
    }
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int) {
    QueryPerformanceFrequency(&g_QpcFreq);

    SpriteSheet sheet;
    const auto spritePath = (ExeDir() / Config::kSpriteSheetPath).string();
    if (!sheet.Load(spritePath, Config::kSpriteWidth, Config::kSpriteHeight)) {
        sheet.GenerateProcedural(Config::kSpriteWidth, Config::kSpriteHeight, 1);
    }

    SpriteSheet sleepSheet;
    const auto sleepSpritePath = (ExeDir() / Config::kSleepSpriteSheetPath).string();
    const bool hasSleepSheet =
        sleepSheet.Load(sleepSpritePath, Config::kSpriteWidth, Config::kSpriteHeight);

    SpriteSheet eatSheet;
    const auto eatSpritePath = (ExeDir() / Config::kEatSpriteSheetPath).string();
    const bool hasEatSheet =
        eatSheet.Load(eatSpritePath, Config::kSpriteWidth, Config::kSpriteHeight);

    SpriteSheet foodSheet;
    const auto foodSpritePath = (ExeDir() / Config::kFoodSpriteSheetPath).string();
    const bool hasFoodSheet =
        foodSheet.Load(foodSpritePath, Config::kSpriteWidth, Config::kSpriteHeight);

    Pet pet;
    PetStats loaded;
    double offlineSeconds = 0.0;
    if (StatsStore::Load(loaded, offlineSeconds)) {
        pet.RestoreFromSave(loaded, offlineSeconds);
    }
    g_Pet = &pet;

    PetWindow window;
    if (!window.Create(hInst, Config::kWindowWidth, Config::kWindowHeight)) {
        return 1;
    }
    window.SetMessageHandler(OnMessage);
    window.PositionBottomRight(40, 60);
    g_Window = &window;

    HICON appIcon = LoadIconW(hInst, MAKEINTRESOURCEW(IDI_APP_ICON));
    if (!appIcon) appIcon = LoadIcon(nullptr, IDI_APPLICATION);

    TrayIcon tray;
    tray.Install(window.Hwnd(), appIcon, L"Desktop Pet");
    g_Tray = &tray;

    LARGE_INTEGER lastTime;
    QueryPerformanceCounter(&lastTime);
    const double targetFrameSec = 1.0 / Config::kTargetFps;

    bool running = true;
    while (running) {
        MSG msg;
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) { running = false; break; }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        if (!running) break;

        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        const double elapsed = static_cast<double>(now.QuadPart - lastTime.QuadPart)
                             / static_cast<double>(g_QpcFreq.QuadPart);

        if (elapsed >= targetFrameSec) {
            const float dt = static_cast<float>(elapsed);
            lastTime = now;

            pet.Update(dt);

            const PetPose pose = pet.CurrentPose();
            const int destX = pose.xOffset;
            const int destY = Config::kBobMargin + pose.yOffset;

            const bool sleeping = pet.State() == PetState::Sleeping;
            const bool eating   = pet.State() == PetState::Eating;

            // Eating bite: which bite we're on and whether the eat-pose is active.
            int  bite        = 0;
            bool eatPoseOpen = false;
            if (eating) {
                const float phaseTime    = pet.PhaseTime();
                const float biteDuration = Config::kEatingDurationSec / static_cast<float>(Config::kEatingBiteCount);
                bite        = std::min(static_cast<int>(phaseTime / biteDuration),
                                       Config::kEatingBiteCount - 1);
                eatPoseOpen = (phaseTime - static_cast<float>(bite) * biteDuration) >= (biteDuration - Config::kBiteEatPhaseSec);
            }

            const SpriteSheet* petSheet = &sheet;
            if (sleeping && hasSleepSheet)       petSheet = &sleepSheet;
            else if (eating && eatPoseOpen && hasEatSheet) petSheet = &eatSheet;

            window.BeginFrame();
            window.BlitSprite(petSheet->FramePixels(0), petSheet->Stride(), 0, 0,
                              petSheet->FrameWidth(), petSheet->FrameHeight(),
                              destX, destY);

            // Food overlay: stays still, disappears top-to-bottom one third per bite.
            if (eating && hasFoodSheet) {
                const int rowsGone  = bite * (Config::kSpriteHeight / Config::kEatingBiteCount);
                const int foodH     = Config::kSpriteHeight - rowsGone;
                window.BlitSprite(foodSheet.FramePixels(0), foodSheet.Stride(),
                                  0, rowsGone, Config::kSpriteWidth, foodH,
                                  0, Config::kBobMargin + rowsGone);
            }

            window.EndFrame();

            g_TimeSinceSave += dt;
            if (g_TimeSinceSave >= Config::kAutoSaveIntervalSec) {
                g_TimeSinceSave = 0.0f;
                SaveNow();
                UpdateTrayTooltip();
            }
        } else {
            Sleep(1);
        }
    }

    SaveNow();
    tray.Remove();
    window.Destroy();
    return 0;
}
