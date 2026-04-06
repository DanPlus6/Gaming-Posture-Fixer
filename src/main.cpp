#include <windows.h>
#include <thread>
#include <atomic>
#include <chrono>
#include "../include/resource.h"
#include <cstdio>
using namespace std;


constexpr int delay = 10;
atomic<bool> running(true);
int imgIDs[] = { IDB_BIG_PAPI, IDB_GOLDEN_POSTURE, IDB_MEOW };

void drawAndRemove() {
    int selectedID = imgIDs[rand() % 3];

    
}

void postureReminder() {
    int localDelay = delay;
    unsigned long long runtime = 0;
    unsigned long long iteration = 0;
    while (running) {
        if (localDelay > 0) {
            printf("[+] Delayed 1 second | Seconds left: %d | Total seconds elapsed: %llu\n", localDelay, runtime);

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
