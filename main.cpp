#include <windows.h>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstdio>
using namespace std;


/* Adjustable variable for delay between posture reminders */
constexpr int delay = 10;

/* Shared variable for terminating entire program */
atomic<bool> running(true);
/* Listener for keybind */
void keybindListener() {
    while (running) {
        bool ctrl  = GetAsyncKeyState(VK_CONTROL) & 0x8000;
        bool shift = GetAsyncKeyState(VK_SHIFT)   & 0x8000;
        bool l     = GetAsyncKeyState('L')        & 0x8000;
        
        // terminate upon keybind pressed
        if (ctrl && shift && l) {
            running = false;
            exit(0);
            printf("hopefully terminated :prayge:\n");
        }

        // sleep for 10ms between checks for stability (100 checks/s)
        this_thread::sleep_for(chrono::milliseconds(10));
    }
}

/* main program */
void postureReminder() {
    int localDelay = delay;
    while (running) {
        if (!delay) {
            printf("orz bruce\n");
            
            // reset delay
            localDelay = delay;
        } else {
            printf("delayed 1 sec\n");

            // delay 1 second
            this_thread::sleep_for(chrono::seconds(1));
            localDelay--;
        }
    }
}

/* program entrypoint */
int main() {
    printf("program started i think :blobpensivepray:\n");

    thread listener(keybindListener);
    thread reminder(postureReminder);

    reminder.join();
    listener.detach();

    printf("main thread terminated :lul:\n");
    return 0;
}
