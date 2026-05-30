#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

class TrayIcon {
public:
    static constexpr UINT kCallbackMessage = WM_APP + 1;

    // Menu command IDs; the owner WndProc receives these via WM_COMMAND.
    static constexpr UINT kCmdFeed  = 1001;
    static constexpr UINT kCmdPet   = 1002;
    static constexpr UINT kCmdSleep = 1003;
    static constexpr UINT kCmdQuit  = 1004;

    bool Install(HWND owner, HICON icon, const wchar_t* tooltip);
    void Remove();
    void ShowContextMenu();

private:
    HWND  m_Owner = nullptr;
    NOTIFYICONDATAW m_Data{};
    bool  m_Installed = false;
};
