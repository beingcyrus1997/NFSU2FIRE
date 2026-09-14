#include <windows.h>

// Global state to track if the exhaust flame toggle is ON or OFF
bool bExhaustFlameToggle = false;

// Helper function to safely write bytes into game memory
void PatchMemory(DWORD address, void* value, int size) {
    DWORD oldProtect;
    VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy((LPVOID)address, value, size);
    VirtualProtect((LPVOID)address, size, oldProtect, &oldProtect);
}

// Background thread loop running alongside the game
DWORD WINAPI MainThread(LPVOID lpParam) {
    // Wait until the main game executable is fully loaded into memory
    while (GetModuleHandleA("speed2.exe") == NULL) {
        Sleep(100);
    }

    // --- Exhaust & Nitrous Memory Addresses (NFSU2 v1.2) ---
    // In NFSU2, these pointer paths force the flame animation/particle state.
    // Address 0x00865FB4 often handles global NOS/Flame rendering animation triggers.
    DWORD baseAddress = 0x00865FB4; 

    while (true) {
        // Check if the 'K' key is pressed
        // (Using & 1 prevents multiple triggers from a single press)
        if (GetAsyncKeyState('K') & 1) {
            bExhaustFlameToggle = !bExhaustFlameToggle; // Switch true <-> false

            // Visual feedback indicator sound (Short high beep for ON, low beep for OFF)
            if (bExhaustFlameToggle) {
                Beep(800, 150); 
            } else {
                Beep(400, 150);
            }
        }

        // If the toggle is active, force the flame state values every frame
        if (bExhaustFlameToggle) {
            // Overwrite the rendering flag to constantly spawn exhaust flames
            // 0x01 forces standard engine overrun/NOS backfire flames
            unsigned char flameActiveByte = 0x01;
            
            // Apply patches to the global rendering offsets
            // Note: Different car mods might use unique offsets, but 0x00865FB4 is the global engine state hook
            PatchMemory(baseAddress, &flameActiveByte, sizeof(flameActiveByte));
        }

        Sleep(10); // Safeguard to keep CPU usage low
    }
    return 0;
}

// Entry point executed by the Ultimate ASI Loader
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        // Create an independent execution thread so it functions in menus and races alike
        CreateThread(NULL, 0, MainThread, NULL, 0, NULL);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
