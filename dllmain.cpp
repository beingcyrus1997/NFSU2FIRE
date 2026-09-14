#include <windows.h>

bool bExhaustFlameToggle = false;

// Function to safely inject overrides into game code memory pages
void WriteMemoryByte(DWORD address, unsigned char value) {
    DWORD oldProtect;
    VirtualProtect((LPVOID)address, 1, PAGE_EXECUTE_READWRITE, &oldProtect);
    *(unsigned char*)address = value;
    VirtualProtect((LPVOID)address, 1, oldProtect, &oldProtect);
}

DWORD WINAPI MainThread(LPVOID lpParam) {
    // Wait until the speed2.exe engine modules are fully running
    while (GetModuleHandleA("speed2.exe") == NULL) {
        Sleep(100);
    }

    // --- Hardcoded Memory Offset for NFSU2 v1.2 ---
    // This address targets the engine overrun flame logic check inside the v1.2 executable
    DWORD v12ExhaustFlamesAddress = 0x0059C3B2; 

    while (true) {
        // Intercept inputs safely anywhere in menus or gameplay loops
        if (GetAsyncKeyState('K') & 1) {
            bExhaustFlameToggle = !bExhaustFlameToggle;
            
            if (bExhaustFlameToggle) {
                Beep(1200, 80); // High short pitch = Activated
            } else {
                Beep(600, 80);  // Low short pitch = Deactivated
            }
        }

        if (bExhaustFlameToggle) {
            // Overwrite the conditional jump instruction
            // 0xEB = Assembly JMP (unconditional), forcing the particle manager to draw flames nonstop
            WriteMemoryByte(v12ExhaustFlamesAddress, 0xEB);
        } else {
            // Restore native game physics handling
            // 0x74 = Assembly JE (jump if equal) calculation check flag
            WriteMemoryByte(v12ExhaustFlamesAddress, 0x74);
        }

        Sleep(10); // Maintain low overhead footprint
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
