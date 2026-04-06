#include <windows.h>
#include <mmsystem.h>
#include "../include/resource.h"
#include <random>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstdio>
using namespace std;

constexpr UINT  REMINDER_DELAY_S    = 10;
constexpr UINT  OVERLAY_DURATION_MS = 750;
constexpr UINT  BMP_IDS[]           = { IDB_GOLDEN_POSTURE, IDB_BIG_PAPI, IDB_MEOW };
constexpr UINT  BMP_COUNT           = 3;
constexpr WCHAR OVERLAY_CLASS[]     = L"PostureOverlay";
constexpr UINT  TIMER_DISMISS       = 1;

atomic<bool> running(true);

/* pick random reminder image */
static UINT randomBmp() {
    static mt19937 rng(random_device{}());
    uniform_int_distribution<UINT> dist(0, BMP_COUNT - 1);
    return BMP_IDS[dist(rng)];
}

/* load packaged BMP file from embedded resource */
static HBITMAP loadBitmap(UINT id) {
    HINSTANCE hinst = GetModuleHandle(nullptr);
    HRSRC     hres  = FindResource(hinst, MAKEINTRESOURCE(id), RT_RCDATA);
    if (!hres) return nullptr;

    HGLOBAL hglob = LoadResource(hinst, hres);
    void*   pdata = LockResource(hglob);

    auto* fhdr = static_cast<BITMAPFILEHEADER*>(pdata);
    auto* ihdr = reinterpret_cast<BITMAPINFO*>(static_cast<BYTE*>(pdata) + sizeof(BITMAPFILEHEADER));

    HDC     hdc = GetDC(nullptr);
    HBITMAP hbm = CreateDIBitmap(
        hdc,
        &ihdr->bmiHeader,
        CBM_INIT,
        static_cast<BYTE*>(pdata) + fhdr->bfOffBits,
        ihdr,
        DIB_RGB_COLORS
    );
    ReleaseDC(nullptr, hdc);
    return hbm;
}

/* window proc */
static LRESULT CALLBACK OverlayProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC  hdc  = BeginPaint(hwnd, &ps);
            auto hbmp = reinterpret_cast<HBITMAP>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

            if (hbmp) {
                HDC     mdc = CreateCompatibleDC(hdc);
                HBITMAP old = static_cast<HBITMAP>(SelectObject(mdc, hbmp));
                BITMAP  bm  = {};
                GetObject(hbmp, sizeof(bm), &bm);

                RECT r = {};
                GetClientRect(hwnd, &r);
                StretchBlt(
                    hdc, 0, 0, r.right, r.bottom,
                    mdc, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY
                );

                SelectObject(mdc, old);
                DeleteDC(mdc);
            } else {
                RECT r = {};
                GetClientRect(hwnd, &r);
                FillRect(hdc, &r, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
                SetTextColor(hdc, RGB(255, 80, 80));
                SetBkMode(hdc, TRANSPARENT);
                DrawText(hdc, L"FIX YO DAMN POSTURE GNG", -1, &r,
                         DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
            // else {
            //     RECT r = {};
            //     GetClientRect(hwnd, &r);
            //     FillRect(hdc, &r, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
            //     SetTextColor(hdc, RGB(255, 80, 80));
            //     SetBkMode(hdc, TRANSPARENT);
            //     DrawText(hdc, L"FIX YO DAMN POSTURE GNG", -1, &r,
            //              DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            // }

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_TIMER: case WM_LBUTTONDOWN:
            KillTimer(hwnd, TIMER_DISMISS);
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, msg, wp, lp);
}

/* register overlay class once per proc */
static void ensureClassRegistered() {
    static bool done = false;
    if (done) return;

    WNDCLASSEX wc    = { sizeof(wc) };
    wc.lpfnWndProc   = OverlayProc;
    wc.hInstance     = GetModuleHandle(nullptr);
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = OVERLAY_CLASS;
    RegisterClassEx(&wc);
    done = true;
}

/* Callback to draw and remove reminder image */
static void drawAndRemove() {
    ensureClassRegistered();

    const int sw = GetSystemMetrics(SM_CXSCREEN);
    const int sh = GetSystemMetrics(SM_CYSCREEN);
    const int ow = sw;
    const int oh = sh / 8;
    const int oy = (sh - oh) / 2;

    HBITMAP hbmp = loadBitmap(randomBmp());

    HWND hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        OVERLAY_CLASS, nullptr,
        WS_POPUP | WS_VISIBLE,
        0, oy, ow, oh,
        nullptr, nullptr, GetModuleHandle(nullptr), nullptr
    );

    if (!hwnd) {
        if (hbmp) DeleteObject(hbmp);
        return;
    }

    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(hbmp));
    SetWindowPos(hwnd, HWND_TOPMOST, 0, oy, ow, oh, SWP_SHOWWINDOW);
    SetTimer(hwnd, TIMER_DISMISS, OVERLAY_DURATION_MS, nullptr);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (hbmp) DeleteObject(hbmp);
}

/* Callback to play reminder audio */
static void playAudio() {
    HINSTANCE hinst = GetModuleHandle(nullptr);
    HRSRC     hres  = FindResource(hinst, MAKEINTRESOURCE(IDR_LOCK_IN), RT_RCDATA);
    if (!hres) return;

    HGLOBAL hglob = LoadResource(hinst, hres);
    void*   pdata = LockResource(hglob);

    PlaySound(static_cast<LPCWSTR>(pdata), nullptr, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
}

/* Reminder thread entrypoint */
static void postureReminder() {
    int       localDelay = REMINDER_DELAY_S;
    ULONGLONG runtime    = 0;
    ULONGLONG iteration  = 0;

    while (running) {
        if (localDelay > 0) {
            printf("[+] Delayed 1 second | Seconds left: %d | Total seconds elapsed: %llu\n", localDelay, runtime);
            this_thread::sleep_for(chrono::seconds(1));
            localDelay--;
        } else {
            printf("[+] Iteration %llu\n", ++iteration);
            playAudio();
            drawAndRemove();
            localDelay = REMINDER_DELAY_S;
        }
        runtime++;
    }
}

/* Keybind listener thread entrypoint */
static void keybindListener() {
    while (running) {
        bool ctrl  = GetAsyncKeyState(VK_CONTROL) & 0x8000;
        bool shift = GetAsyncKeyState(VK_SHIFT)   & 0x8000;
        bool l     = GetAsyncKeyState('L')        & 0x8000;

        if (ctrl && shift && l) {
            printf("[+] ------------------------ Keybind pressed, program terminating ------------------------\n");
            running = false;
            exit(0);
        }

        this_thread::sleep_for(chrono::milliseconds(10));
    }
}

/* Main program entrypoint */
int main() {
    printf("[+] --------------------- Entrypoint triggered --------------------- \n");

    thread listener(keybindListener);
    thread reminder(postureReminder);

    reminder.join();
    listener.detach();

    printf("[-] --------------------- !!! Main thread intiatively terminated !!! ---------------------\n");
    return 0;
}