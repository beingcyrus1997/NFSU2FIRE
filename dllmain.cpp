#include <windows.h>

bool bExhaustFlameToggle = false;

// Function to safely inject assembly overrides into the game executable engine
void PatchGameByte(DWORD address, unsigned char value) {
    DWORD oldProtect;
    VirtualProtect((LPVOID)address, 1, PAGE_EXECUTE_READWRITE, &oldProtect);
    *(unsigned char*)address = value;
    VirtualProtect((LPVOID)address, 1, oldProtect, &oldProtect);
}

DWORD WINAPI MainThread(LPVOID lpParam) {
    // Confirm speed2.exe engine modules are ready
    while (GetModuleHandleA("speed2.exe") == NULL) {
        Sleep(100);
    }

    // Engine Routine Addresses managing exhaust heat and overrun flame injection thresholds
    // Overwriting these ensures calculations always evaluate to "spawn flame particles"
    DWORD engineOverrunAddress = 0x0059C6D5; 
    DWORD menuShowcaseFlameAddress = 0x004B2A1E;

    while (true) {
        // Listening for 'K' button input to cycle states
        if (GetAsyncKeyState('K') & 1) {
            bExhaustFlameToggle = !bExhaustFlameToggle;
            
            if (bExhaustFlameToggle) {
                Beep(1000, 120); // High beep = ON
            } else {
                Beep(500, 120);  // Low beep = OFF
            }
        }

        if (bExhaustFlameToggle) {
            // Force code comparison flags to pass verification loops
            // 0xEB represents an unconditional assembly JMP (Jump) instruction
            PatchGameByte(engineOverrunAddress, 0xEB);
            PatchGameByte(menuShowcaseFlameAddress, 0x01);
        } else {
            // Revert back to original game engine logic bytes (conditional jumps/checks)
            PatchGameByte(engineOverrunAddress, 0x74); 
            PatchGameByte(menuShowcaseFlameAddress, 0x00);
        }

        Sleep(10); 
    }
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, MainThread, NULL, 0, NULL);
    }
    return TRUE;
}
