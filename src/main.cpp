#include <windows.h>
#include <gdiplus.h>
#pragma comment (lib,"Gdiplus.lib")
#include "../include/resource.h"
#include <random>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstdio>
using namespace std;
using namespace Gdiplus;


constexpr UINT delay = 10;
atomic<bool> running(true);
int imgIDs[] = { IDB_BIG_PAPI, IDB_GOLDEN_POSTURE, IDB_MEOW };

void drawAndRemove() {
    
}

void postureReminder() {
    int localDelay = delay;
    ULONGLONG runtime = 0;
    ULONGLONG iteration = 0;
    while (running) {
        if (localDelay > 0) {
            printf("[+] Delayed 1 second | Seconds left: %d | Total seconds elapsed: %llu\n", localDelay, runtime);

            drawAndRemove();

            this_thread::sleep_for(chrono::seconds(1));
            localDelay--;
        } else {
            printf("[+] Iteration %llu\n", ++iteration);

            localDelay = delay;
        }
        runtime++;
    }
}

void keybindListener() {
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

int main() {
    printf("[+] --------------------- Entrypoint triggered --------------------- \n");

    thread listener(keybindListener);
    thread reminder(postureReminder);

    reminder.join();
    listener.detach();

    printf("[-] --------------------- !!! Main thread intiatively terminated !!! ---------------------\n");
    return 0;
}
