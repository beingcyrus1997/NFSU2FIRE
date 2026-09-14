#include <windows.h>

bool bExhaustFlameToggle = false;

// Safe pointer verification helper
bool IsAddressValid(DWORD address) {
    return (address >= 0x00400000 && address <= 0x00A00000);
}

DWORD WINAPI MainThread(LPVOID lpParam) {
    while (GetModuleHandleA("speed2.exe") == NULL) {
        Sleep(100);
    }

    // This points directly to the core Engine Manager structure in v1.2
    // It is highly stable because it's tied to engine variables, not rendering loops
    DWORD engineBasePointer = 0x008A11B4; 

    while (true) {
        if (GetAsyncKeyState('K') & 1) {
            bExhaustFlameToggle = !bExhaustFlameToggle;
            
            if (bExhaustFlameToggle) {
                Beep(1300, 70); // High beep
            } else {
                Beep(650, 70);  // Low beep
            }
        }

        if (bExhaustFlameToggle) {
            DWORD* pEngineBase = (DWORD*)engineBasePointer;
            if (pEngineBase && IsAddressValid(*pEngineBase)) {
                
                // Offset to active vehicle drivetrain structure
                DWORD* pDrivetrain = (DWORD*)(*pEngineBase + 0x24); 
                if (pDrivetrain && IsAddressValid(*pDrivetrain)) {
                    
                    // 0x150 is the raw fuel-rich multiplier offset inside the combustion simulation loop
                    float* pFuelMixture = (float*)(*pDrivetrain + 0x150);
                    
                    if (pFuelMixture) {
                        DWORD oldProtect;
                        VirtualProtect((LPVOID)pFuelMixture, sizeof(float), PAGE_EXECUTE_READWRITE, &oldProtect);
                        
                        // Force a massive unburnt fuel ratio to trigger native exhaust flame loops
                        *pFuelMixture = 5.0f; 
                        
                        VirtualProtect((LPVOID)pFuelMixture, sizeof(float), oldProtect, &oldProtect);
                    }
                }
            }
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
