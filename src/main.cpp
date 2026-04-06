#include <windows.h>
#include "../include/resource.h"
#include <random>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstdio>
using namespace std;

constexpr UINT  REMINDER_DELAY      = 10;
constexpr UINT  OVERLAY_DURATION    = 5;
constexpr WCHAR OVERLAY_CLASS[]     = L"PostureOverlay";
constexpr UINT  TIMER_DISMISS       = 1;

atomic<bool> running(true);

/* Callback to draw and remove reminder image */
static void drawAndRemove() {
    
}

/* Callback to play reminder audio */
static void playAudio() {

}

/* Reminder thread entrypoint */
static void postureReminder() {
    int localDelay = REMINDER_DELAY;
    ULONGLONG runtime = 0;
    ULONGLONG iteration = 0;
    while (running) {
        if (localDelay > 0) {
            printf("[+] Delayed 1 second | Seconds left: %d | Total seconds elapsed: %llu\n", localDelay, runtime);

            this_thread::sleep_for(chrono::seconds(1));
            localDelay--;
        } else {
            printf("[+] Iteration %llu\n", ++iteration);

            playAudio();
            drawAndRemove();

            localDelay = REMINDER_DELAY;
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
