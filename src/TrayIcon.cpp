#include "TrayIcon.hpp"

#include <cstring>

bool TrayIcon::Install(HWND owner, HICON icon, const wchar_t* tooltip) {
    if (m_Installed) return true;

    m_Owner = owner;
    m_Data.cbSize           = sizeof(m_Data);
    m_Data.hWnd             = owner;
    m_Data.uID              = 1;
    m_Data.uFlags           = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    m_Data.uCallbackMessage = kCallbackMessage;
    m_Data.hIcon            = icon;
    if (tooltip) {
        lstrcpynW(m_Data.szTip, tooltip,
                  static_cast<int>(sizeof(m_Data.szTip) / sizeof(wchar_t)));
    }

    if (!Shell_NotifyIconW(NIM_ADD, &m_Data)) return false;
    m_Installed = true;
    return true;
}

void TrayIcon::Remove() {
    if (!m_Installed) return;
    Shell_NotifyIconW(NIM_DELETE, &m_Data);
    m_Installed = false;
}

void TrayIcon::ShowContextMenu() {
    if (!m_Owner) return;

    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, kCmdFeed,  L"&Feed");
    AppendMenuW(menu, MF_STRING, kCmdPet,   L"&Pet");
    AppendMenuW(menu, MF_STRING, kCmdSleep, L"Put to &Sleep");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, kCmdQuit,  L"&Quit");

    POINT pt;
    GetCursorPos(&pt);

    // Required workaround for the long-standing TrackPopupMenu bug where
    // the menu won't dismiss on click-outside unless the owner is foreground.
    SetForegroundWindow(m_Owner);
    TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_BOTTOMALIGN,
                   pt.x, pt.y, 0, m_Owner, nullptr);
    PostMessage(m_Owner, WM_NULL, 0, 0);

    DestroyMenu(menu);
}
