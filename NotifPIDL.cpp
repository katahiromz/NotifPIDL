// NotifPIDL.cpp
// License: MIT
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shlobj.h>
#include <stdio.h>
#include <strsafe.h>
#include <string>
#include "memdump.h"

#define CLASSNAME TEXT("NotifPIDL")

HINSTANCE g_hInst = NULL;
HWND g_hMainWnd = NULL;
ULONG g_nChangeReg = 0;

#define MYWM_CHANGENOTIFY (WM_USER + 100)

const char *get_name_of_event(LONG event)
{
    event &= ~SHCNE_INTERRUPT;
    switch (event)
    {
    case SHCNE_RENAMEITEM: return "SHCNE_RENAMEITEM";
    case SHCNE_CREATE: return "SHCNE_CREATE";
    case SHCNE_DELETE: return "SHCNE_DELETE";
    case SHCNE_MKDIR: return "SHCNE_MKDIR";
    case SHCNE_RMDIR: return "SHCNE_RMDIR";
    case SHCNE_MEDIAINSERTED: return "SHCNE_MEDIAINSERTED";
    case SHCNE_MEDIAREMOVED: return "SHCNE_MEDIAREMOVED";
    case SHCNE_DRIVEREMOVED: return "SHCNE_DRIVEREMOVED";
    case SHCNE_DRIVEADD: return "SHCNE_DRIVEADD";
    case SHCNE_NETSHARE: return "SHCNE_DRIVEADD";
    case SHCNE_NETUNSHARE: return "SHCNE_NETUNSHARE";
    case SHCNE_ATTRIBUTES: return "SHCNE_ATTRIBUTES";
    case SHCNE_UPDATEDIR: return "SHCNE_UPDATEDIR";
    case SHCNE_UPDATEITEM: return "SHCNE_UPDATEITEM";
    case SHCNE_SERVERDISCONNECT  : return "SHCNE_SERVERDISCONNECT  ";
    case SHCNE_UPDATEIMAGE: return "SHCNE_UPDATEIMAGE";
    case SHCNE_DRIVEADDGUI: return "SHCNE_DRIVEADDGUI";
    case SHCNE_RENAMEFOLDER: return "SHCNE_RENAMEFOLDER";
    case SHCNE_FREESPACE: return "SHCNE_FREESPACE";
    case SHCNE_EXTENDED_EVENT: return "SHCNE_EXTENDED_EVENT";
    case SHCNE_ASSOCCHANGED: return "SHCNE_ASSOCCHANGED";
    default:
        return "(Unknown)";
    }
}

BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    g_hMainWnd = g_hMainWnd;

    SHChangeNotifyEntry entry;
    entry.fRecursive = TRUE;
    entry.pidl = NULL;
    DWORD fEvents = SHCNE_ALLEVENTS;
    g_nChangeReg = SHChangeNotifyRegister(hwnd,
                                          SHCNRF_InterruptLevel | SHCNRF_ShellLevel |
                                          SHCNRF_NewDelivery,
                                          fEvents, MYWM_CHANGENOTIFY,
                                          1, &entry);
    if (!g_nChangeReg)
    {
        MessageBox(hwnd, TEXT("SHChangeNotifyRegister failed"), NULL, MB_ICONERROR);
        return FALSE;
    }

    return TRUE;
}

void OnDestroy(HWND hwnd)
{
    SHChangeNotifyDeregister(g_nChangeReg);
    PostQuitMessage(0);
}

const char *get_binary_text(const void *data, size_t size)
{
    static char s_text[1024];
    TCHAR buf[16];
    const BYTE *pb = (const BYTE *)data;

    if (!size)
        return "(empty)";

    s_text[0] = 0;
    for (size_t ib = 0; ib < size; ++ib)
    {
        if (ib)
            StringCchCat(s_text, _countof(s_text), TEXT(" "));
        StringCchPrintf(buf, _countof(buf), TEXT("%02X"), pb[ib]);
        StringCchCat(s_text, _countof(s_text), buf);
    }

    return s_text;
}

void dump_pidl(const char *name, LPCITEMIDLIST pidl)
{
    UINT size = ILGetSize(pidl);

    CHAR szPath[MAX_PATH];
    if (!SHGetPathFromIDListA(pidl, szPath))
        szPath[0] = 0;

    printf("%s:\n", name);
    printf("  size: %u (0x%X) bytes\n", size, size);
    if (szPath[0])
        printf("  path: %s\n", szPath);

    if (size)
    {
        std::string str;
        memdump(str, pidl, size);
        printf("\n");
        printf("%s\n", str.c_str());
    }
}

void OnChangeNotify(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    // We use SHCNRF_NewDelivery method
    HANDLE hChange = (HANDLE)wParam;
    DWORD dwProcID = (DWORD)lParam;

    PIDLIST_ABSOLUTE *ppidl = NULL;
    LONG event;
    HANDLE hLock = SHChangeNotification_Lock(hChange, dwProcID, &ppidl, &event);
    if (!hLock)
    {
        printf("hLock == NULL\n");
        return;
    }

    printf("event: %s (0x%08lX)\n", get_name_of_event(event), event);

    if (ppidl[0])
        dump_pidl("ppidl[0]", ppidl[0]);
    else
        printf("ppidl[0] is NULL\n");

    if (ppidl[1])
        dump_pidl("ppidl[1]", ppidl[1]);
    else
        printf("ppidl[1] is NULL\n");

    SHChangeNotification_Unlock(hLock);
}

LRESULT CALLBACK
WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
        HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
    default:
        if (uMsg == MYWM_CHANGENOTIFY)
        {
            OnChangeNotify(hwnd, wParam, lParam);
            break;
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

INT WINAPI
WinMain(HINSTANCE   hInstance,
        HINSTANCE   hPrevInstance,
        LPSTR       lpCmdLine,
        INT         nCmdShow)
{
    g_hInst = hInstance;
    InitCommonControls();

    WNDCLASS wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    wc.lpszClassName = CLASSNAME;
    if (!RegisterClass(&wc))
    {
        MessageBox(NULL, TEXT("RegisterClass failed"), NULL, MB_ICONERROR);
        return 1;
    }

    DWORD style = WS_OVERLAPPEDWINDOW;
    HWND hwnd = CreateWindow(CLASSNAME, CLASSNAME, style,
                             CW_USEDEFAULT, CW_USEDEFAULT, 400, 300,
                             NULL, NULL, hInstance, NULL);
    if (!hwnd)
    {
        MessageBox(NULL, TEXT("CreateWindow failed"), NULL, MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (INT)msg.wParam;
}

int main(void)
{
    STARTUPINFO info = { sizeof(info) };
    GetStartupInfo(&info);
    return WinMain(GetModuleHandle(NULL), NULL, GetCommandLine(), info.wShowWindow);
}
