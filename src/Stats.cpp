#include "Stats.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>

#include <cstdio>
#include <ctime>
#include <fstream>
#include <system_error>

namespace {
    constexpr uint32_t kMagic   = 0x50455450; // 'PETP'
    constexpr uint32_t kVersion = 1;

    struct FileLayout {
        uint32_t magic;
        uint32_t version;
        PetStats stats;
        int64_t  savedAtUnix;
    };
}

std::filesystem::path StatsStore::Path() {
    PWSTR roaming = nullptr;
    if (FAILED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &roaming))) {
        return {};
    }
    std::filesystem::path p(roaming);
    CoTaskMemFree(roaming);
    p /= L"DesktopPet";
    std::error_code ec;
    std::filesystem::create_directories(p, ec);
    p /= L"state.bin";
    return p;
}

bool StatsStore::Load(PetStats& outStats, double& outSecondsSinceSave) {
    outSecondsSinceSave = 0.0;
    auto path = Path();
    if (path.empty()) return false;

    std::ifstream f(path, std::ios::binary);
    if (!f) return false;

    FileLayout layout{};
    f.read(reinterpret_cast<char*>(&layout), sizeof(layout));
    if (!f || layout.magic != kMagic || layout.version != kVersion) return false;

    outStats = layout.stats;
    const auto now = static_cast<int64_t>(std::time(nullptr));
    if (now > layout.savedAtUnix) {
        outSecondsSinceSave = static_cast<double>(now - layout.savedAtUnix);
    }
    return true;
}

void StatsStore::Save(const PetStats& stats) {
    auto path = Path();
    if (path.empty()) return;

    FileLayout layout{};
    layout.magic       = kMagic;
    layout.version     = kVersion;
    layout.stats       = stats;
    layout.savedAtUnix = static_cast<int64_t>(std::time(nullptr));

    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    if (!f) return;
    f.write(reinterpret_cast<const char*>(&layout), sizeof(layout));
}
