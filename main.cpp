#include <windows.h>
#include <string>
#include <vector>
#include <atomic>
#include <gdiplus.h>
#include <algorithm>
#include <commctrl.h>
#include <tlhelp32.h>
#include <cstdlib>
#include <mutex>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "comctl32.lib")

#include "byte/build_bytes.h"
using namespace Gdiplus;

#define CLR_BACK_TOP Color(255, 15, 15, 15)
#define CLR_BACK_BOT Color(255, 5, 5, 5)
#define CLR_dMAIN Color(255, 139, 0, 255)
#define CLR_MAIN Color(255, 139, 0, 255)
#define CLR_MAIN_DIM Color(255, 100, 0, 200)
#define CLR_GRAY Color(255, 120, 120, 120)
#define CLR_WHITE Color(255, 255, 255, 255)

std::wstring current_status = L"Waiting...";
std::mutex statusMutex;

std::atomic<float> targetProgress(0.0f);
float visualProgress = 0.0f;
float flareAnim = 0.0f;
float windowAlpha = 0.0f;
float exitAnim = 0.0f;
float btnHoverAnim = 0.0f;

bool isExitHovered = false;
bool isBtnHovered = false;
bool isLoading = false;
bool trackingMouse = false;

RECT btnRect = { 15, 195, 225, 215 };
RECT exitRect = { 215, 12, 225, 22 };

struct Particle {
    float x, y, speed, opacity;
};

std::vector<Particle> particles;
ULONG_PTR gdiplusToken;

Font* fSmall = nullptr;
Font* fLogo = nullptr;
Font* fBtn = nullptr;

DWORD FindProcessId(const std::wstring& processName) {
    DWORD pid = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W entry;
        entry.dwSize = sizeof(entry);
        if (Process32FirstW(snapshot, &entry)) {
            do {
                if (_wcsicmp(entry.szExeFile, processName.c_str()) == 0) {
                    pid = entry.th32ProcessID;
                    break;
                }
            } while (Process32NextW(snapshot, &entry));
        }
        CloseHandle(snapshot);
    }
    return pid;
}

std::vector<unsigned char> DecryptPayload(const unsigned char* pSrcData, size_t dataSize) {
    std::vector<unsigned char> data(pSrcData, pSrcData + dataSize);
    /*
    for (size_t i = 0; i < dataSize; ++i) {
        data[i] ^= 0x5A;
    }
    */
    return data;
}

bool InjectViaTempFile(DWORD targetPid, const unsigned char* pSrcData, size_t dataSize) {
    wchar_t tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);

    std::wstring dllPath = std::wstring(tempPath) + L"sys_cache_" + std::to_wstring(GetTickCount64()) + L".dll";
    std::vector<unsigned char> decryptedBytes = DecryptPayload(pSrcData, dataSize);

    HANDLE hFile = CreateFileW(dllPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    DWORD written = 0;
    WriteFile(hFile, decryptedBytes.data(), (DWORD)decryptedBytes.size(), &written, NULL);
    CloseHandle(hFile);

    SetFileAttributesW(dllPath.c_str(), FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, targetPid);
    if (!hProcess) {
        DeleteFileW(dllPath.c_str());
        return false;
    }

    size_t pathSize = (dllPath.length() + 1) * sizeof(wchar_t);
    LPVOID pRemoteBuf = VirtualAllocEx(hProcess, NULL, pathSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pRemoteBuf) {
        CloseHandle(hProcess);
        DeleteFileW(dllPath.c_str());
        return false;
    }

    WriteProcessMemory(hProcess, pRemoteBuf, dllPath.c_str(), pathSize, NULL);

    LPTHREAD_START_ROUTINE pLoadLibrary = (LPTHREAD_START_ROUTINE)GetProcAddress(
        GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW"
    );

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, pLoadLibrary, pRemoteBuf, 0, NULL);
    if (hThread) {
        WaitForSingleObject(hThread, 5000);
        CloseHandle(hThread);
    }

    VirtualFreeEx(hProcess, pRemoteBuf, 0, MEM_RELEASE);
    CloseHandle(hProcess);
    DeleteFileW(dllPath.c_str());

    return true;
}

void CleanLoaderTraces() {
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);

    std::wstring fullPath(exePath);
    size_t lastSlash = fullPath.find_last_of(L"\\");
    std::wstring exeName = (lastSlash != std::wstring::npos) ? fullPath.substr(lastSlash + 1) : fullPath;

    wchar_t windir[MAX_PATH];
    GetWindowsDirectoryW(windir, MAX_PATH);
    std::wstring prefetchPath = std::wstring(windir) + L"\\Prefetch\\";

    WIN32_FIND_DATAW findData;
    std::wstring searchPattern = prefetchPath + L"*" + exeName + L"*.pf";

    HANDLE hFind = FindFirstFileW(searchPattern.c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            std::wstring pfFile = prefetchPath + findData.cFileName;
            DeleteFileW(pfFile.c_str());
        } while (FindNextFileW(hFind, &findData));
        FindClose(hFind);
    }

    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\UserAssist", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        wchar_t subKeyName[256];
        DWORD index = 0;
        DWORD nameLen = 256;
        while (RegEnumKeyExW(hKey, index, subKeyName, &nameLen, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
            std::wstring countPath = std::wstring(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\UserAssist\\") + subKeyName + L"\\Count";

            HKEY hCountKey;
            if (RegOpenKeyExW(HKEY_CURRENT_USER, countPath.c_str(), 0, KEY_WRITE, &hCountKey) == ERROR_SUCCESS) {
                RegDeleteTreeW(hCountKey, NULL);
                RegCloseKey(hCountKey);
            }
            index++;

            nameLen = 256;
        }
        RegCloseKey(hKey);
    }

    system("fsutil usn deletejournal /d /c C: >nul 2>&1");
}

DWORD WINAPI StartLogic(LPVOID lpParam) {
    HWND hWnd = (HWND)lpParam;
    isLoading = true;

    {
        std::lock_guard<std::mutex> lock(statusMutex);
        current_status = L"Hiding process...";
    }
    targetProgress = 0.2f;
    Sleep(400);

    LONG_PTR exStyle = GetWindowLongPtrW(hWnd, GWL_EXSTYLE);
    SetWindowLongPtrW(hWnd, GWL_EXSTYLE, (exStyle & ~WS_EX_APPWINDOW) | WS_EX_TOOLWINDOW);

    {
        std::lock_guard<std::mutex> lock(statusMutex);
        current_status = L"Cleaning logs...";
    }
    targetProgress = 0.4f;
    Sleep(400);

    const wchar_t* targetLogs[] = {
        L"Application",
        L"System",
        L"Security",
        L"Microsoft-Windows-PowerShell/Operational",
        L"Windows PowerShell"
    };

    wchar_t sysDir[MAX_PATH];
    GetSystemDirectoryW(sysDir, MAX_PATH);
    std::wstring wevtutilPath = std::wstring(sysDir) + L"\\wevtutil.exe";

    for (const auto& logName : targetLogs) {
        std::wstring cmdLine = wevtutilPath + L" cl \"" + std::wstring(logName) + L"\"";
        STARTUPINFOW si = { sizeof(si) };
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        PROCESS_INFORMATION pi = { 0 };

        if (CreateProcessW(wevtutilPath.c_str(), const_cast<LPWSTR>(cmdLine.c_str()), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
            WaitForSingleObject(pi.hProcess, 1000);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
    }

    {
        std::lock_guard<std::mutex> lock(statusMutex);
        current_status = L"Clearing USN Journal...";
    }
    targetProgress = 0.6f;
    Sleep(400);
    system("fsutil usn deletejournal /d /c C: >nul 2>&1");

    {
        std::lock_guard<std::mutex> lock(statusMutex);
        current_status = L"Waiting for RustClient...";
    }
    targetProgress = 0.8f;

    DWORD targetPid = 0;
    while (targetPid == 0) {
        targetPid = FindProcessId(L"RustClient.exe");
        if (targetPid == 0) {
            Sleep(1000);
        }
    }

    {
        std::lock_guard<std::mutex> lock(statusMutex);
        current_status = L"Injecting payload...";
    }
    targetProgress = 0.95f;
    Sleep(500);

    InjectViaTempFile(targetPid, buildData::build_bytes, buildData::build_size);

    {
        std::lock_guard<std::mutex> lock(statusMutex);
        current_status = L"Cleaning loader traces...";
    }
    targetProgress = 0.98f;
    Sleep(300);

    CleanLoaderTraces();

    {
        std::lock_guard<std::mutex> lock(statusMutex);
        current_status = L"Complete";
    }
    targetProgress = 1.0f;
    Sleep(500);

    ExitProcess(0);
    return 0;
}

void FillRoundedRect(Graphics* g, Brush* b, RectF r, float rd) {
    GraphicsPath p;
    p.AddArc(r.X, r.Y, rd, rd, 180, 90);
    p.AddArc(r.X + r.Width - rd, r.Y, rd, rd, 270, 90);
    p.AddArc(r.X + r.Width - rd, r.Y + r.Height - rd, rd, rd, 0, 90);
    p.AddArc(r.X, r.Y + r.Height - rd, rd, rd, 90, 90);
    g->FillPath(b, &p);
}

void DrawRoundedRect(Graphics* g, Pen* p, RectF r, float rd) {
    GraphicsPath path;
    path.AddArc(r.X, r.Y, rd, rd, 180, 90);
    path.AddArc(r.X + r.Width - rd, r.Y, rd, rd, 270, 90);
    path.AddArc(r.X + r.Width - rd, r.Y + r.Height - rd, rd, rd, 0, 90);
    path.AddArc(r.X, r.Y + r.Height - rd, rd, rd, 90, 90);
    path.CloseFigure();
    g->DrawPath(p, &path);
}

Color LerpColorGdi(Color c1, Color c2, float t) {
    return Color(
        (BYTE)(c1.GetA() + (c2.GetA() - c1.GetA()) * t),
        (BYTE)(c1.GetR() + (c2.GetR() - c1.GetR()) * t),
        (BYTE)(c1.GetG() + (c2.GetG() - c1.GetG()) * t),
        (BYTE)(c1.GetB() + (c2.GetB() - c1.GetB()) * t)
    );
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
    {
        fSmall = new Font(L"Segoe UI", 10, FontStyleRegular, UnitPixel);
        fLogo = new Font(L"Segoe UI Semibold", 11, FontStyleRegular, UnitPixel);
        fBtn = new Font(L"Segoe UI", 10, FontStyleRegular, UnitPixel);

        for (int i = 0; i < 45; i++)
            particles.push_back({ (float)(rand() % 240), (float)(rand() % 230), (float)(2 + rand() % 10) / 100.0f, (float)(rand() % 80) / 100.0f });

        SetTimer(hWnd, 1, 16, NULL);
        return 0;
    }

    case WM_TIMER:
    {
        if (windowAlpha < 1.0f) {
            windowAlpha += 0.05f;
            SetLayeredWindowAttributes(hWnd, 0, (BYTE)(255 * (windowAlpha > 1.0f ? 1.0f : windowAlpha)), LWA_ALPHA);
        }

        if (isExitHovered) exitAnim = min(1.0f, exitAnim + 0.1f); else exitAnim = max(0.0f, exitAnim - 0.1f);
        if (isBtnHovered) btnHoverAnim = min(1.0f, btnHoverAnim + 0.1f); else btnHoverAnim = max(0.0f, btnHoverAnim - 0.1f);
        if (visualProgress < targetProgress) visualProgress += 0.01f;

        flareAnim += 0.015f;
        if (flareAnim > 1.5f) flareAnim = 0.0f;

        for (auto& p : particles) {
            p.y -= p.speed;
            if (p.y < -10) {
                p.y = 235;
                p.x = (float)(rand() % 240);
            }
        }

        InvalidateRect(hWnd, NULL, FALSE);
        return 0;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        int w = 240, h = 230;

        HDC mDC = CreateCompatibleDC(hdc);
        HBITMAP mBM = CreateCompatibleBitmap(hdc, w, h);
        SelectObject(mDC, mBM);

        Graphics g(mDC);
        g.SetSmoothingMode(SmoothingModeAntiAlias);
        g.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

        RectF rF(0, 0, (REAL)w, (REAL)h);
        LinearGradientBrush brB(rF, CLR_BACK_TOP, CLR_BACK_BOT, LinearGradientModeVertical);
        g.FillRectangle(&brB, rF);

        for (auto& p : particles) {
            SolidBrush pB(Color((BYTE)(p.opacity * 100), 139, 0, 255));
            g.FillEllipse(&pB, (REAL)p.x, (REAL)p.y, 1.2f, 1.2f);
        }

        SolidBrush brMain(CLR_MAIN), brGray(CLR_GRAY), brWhite(CLR_WHITE);
        g.DrawString(L"Ghost", -1, fLogo, PointF(15, 15), &brMain);
        g.DrawString(L"Client", -1, fLogo, PointF(50, 15), &brGray);

        Pen pE(LerpColorGdi(Color(180, 120, 120, 120), Color(255, 40, 200, 255), exitAnim), 1.2f);
        g.DrawLine(&pE, (REAL)exitRect.left, (REAL)exitRect.top, (REAL)exitRect.right, (REAL)exitRect.bottom);
        g.DrawLine(&pE, (REAL)exitRect.right, (REAL)exitRect.top, (REAL)exitRect.left, (REAL)exitRect.bottom);

        RectF rB((REAL)btnRect.left, (REAL)btnRect.top, (REAL)(btnRect.right - btnRect.left), (REAL)(btnRect.bottom - btnRect.top));

        if (isLoading) {
            std::wstring statusCopy;
            {
                std::lock_guard<std::mutex> lock(statusMutex);
                statusCopy = current_status;
            }
            std::wstring ws = statusCopy + L" (" + std::to_wstring((int)(visualProgress * 100)) + L"%)";

            StringFormat sf;
            sf.SetAlignment(StringAlignmentCenter);
            g.DrawString(ws.c_str(), -1, fSmall, RectF(0, rB.Y - 25, 240, 20), &sf, &brGray);

            RectF bgLine(20, rB.Y + 5, 200, 4.0f);
            SolidBrush brLineBg(Color(40, 255, 255, 255));
            FillRoundedRect(&g, &brLineBg, bgLine, 2.0f);

            float curW = bgLine.Width * visualProgress;

            if (curW > 1) {
                RectF actLine(bgLine.X, bgLine.Y, curW, 4.0f);

                Pen pGlow(Color(80, 139, 0, 255), 4.0f);
                g.DrawLine(&pGlow, actLine.X, actLine.Y + 2, actLine.X + actLine.Width, actLine.Y + 2);

                LinearGradientBrush brP(actLine, CLR_MAIN, CLR_MAIN_DIM, LinearGradientModeHorizontal);
                FillRoundedRect(&g, &brP, actLine, 2.0f);

                float fX = actLine.X + (actLine.Width * flareAnim) - 40;
                RectF rFlare(fX, actLine.Y, 40, actLine.Height);

                LinearGradientBrush brFlare(rFlare, Color(0, 255, 255, 255), Color(150, 255, 255, 255), LinearGradientModeHorizontal);

                g.SetClip(actLine);
                g.FillRectangle(&brFlare, rFlare);
                g.ResetClip();
            }
        }
        else {
            Pen pB(LerpColorGdi(Color(255, 40, 40, 45), CLR_MAIN, btnHoverAnim), 1.2f);
            DrawRoundedRect(&g, &pB, rB, 4.0f);

            StringFormat sf;
            sf.SetAlignment(StringAlignmentCenter);
            sf.SetLineAlignment(StringAlignmentCenter);

            SolidBrush brT(LerpColorGdi(CLR_GRAY, CLR_WHITE, btnHoverAnim));
            g.DrawString(L"LAUNCH", -1, fBtn, rB, &sf, &brT);
        }

        BitBlt(hdc, 0, 0, w, h, mDC, 0, 0, SRCCOPY);
        DeleteObject(mBM);
        DeleteDC(mDC);
        EndPaint(hWnd, &ps);

        return 0;
    }

    case WM_NCHITTEST:
    {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        ScreenToClient(hWnd, &pt);

        if (PtInRect(&exitRect, pt) || PtInRect(&btnRect, pt)) {
            return HTCLIENT;
        }

        return HTCAPTION;
    }

    case WM_MOUSEMOVE:
    {
        if (!trackingMouse) {
            TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, hWnd, 0 };
            TrackMouseEvent(&tme);
            trackingMouse = true;
        }

        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        isBtnHovered = PtInRect(&btnRect, pt);
        isExitHovered = PtInRect(&exitRect, pt);

        return 0;
    }

    case WM_MOUSELEAVE:
    {
        isBtnHovered = isExitHovered = trackingMouse = false;
        return 0;
    }

    case WM_LBUTTONDOWN:
    {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };

        if (PtInRect(&exitRect, pt)) {
            ExitProcess(0);
        }

        if (!isLoading && PtInRect(&btnRect, pt)) {
            CreateThread(NULL, 0, StartLogic, hWnd, 0, NULL);
        }

        return 0;
    }

    case WM_DESTROY:
    {
        delete fSmall;
        delete fLogo;
        delete fBtn;
        PostQuitMessage(0);
        return 0;
    }

    default:
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
}

int APIENTRY WinMain(HINSTANCE hi, HINSTANCE, LPSTR, int) {
    GdiplusStartupInput gi;
    GdiplusStartup(&gdiplusToken, &gi, NULL);

    WNDCLASSEXW wc = { sizeof(WNDCLASSEXW) };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hi;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"GhostClientLoader";

    RegisterClassExW(&wc);

    HWND hw = CreateWindowExW(WS_EX_LAYERED, wc.lpszClassName, L"Ghost Client", WS_POPUP | WS_VISIBLE,
        (GetSystemMetrics(0) - 240) / 2, (GetSystemMetrics(1) - 230) / 2, 240, 230, 0, 0, hi, 0);

    MSG m;
    while (GetMessageW(&m, 0, 0, 0)) {
        TranslateMessage(&m);
        DispatchMessageW(&m);
    }

    GdiplusShutdown(gdiplusToken);

    return 0;
}