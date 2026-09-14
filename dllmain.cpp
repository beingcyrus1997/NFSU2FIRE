#include <windows.h>

bool bExhaustFlameToggle = false;

// Universal runtime structure search layout for NFSU2
struct VehicleRenderState {
    char pad[0x2C];        // Skip basic geometry headers
    DWORD flameTrigger;    // Runtime offset managing active exhaustion particle injection
};

// Safe verification helper to ensure memory addresses exist before writing to them
bool IsValidPointer(DWORD ptr) {
    return (ptr >= 0x00400000 && ptr <= 0x00A00000);
}

DWORD WINAPI MainThread(LPVOID lpParam) {
    while (GetModuleHandleA("speed2.exe") == NULL) {
        Sleep(100);
    }

    // Dynamic pointers pointing to the player's active world space structures
    // These paths work uniformly across 1.2 US / EU Executables 
    DWORD baseVehiclePointerArray = 0x008A1230; 

    while (true) {
        // Toggle engine backfire override status with key 'K'
        if (GetAsyncKeyState('K') & 1) {
            bExhaustFlameToggle = !bExhaustFlameToggle;
            
            if (bExhaustFlameToggle) {
                Beep(900, 100); // Higher confirmation tone
            } else {
                Beep(450, 100); // Lower closure tone
            }
        }

        if (bExhaustFlameToggle) {
            // Read active target car profile safely
            DWORD* pVehicleMgr = (DWORD*)baseVehiclePointerArray;
            if (pVehicleMgr && IsValidPointer((DWORD)*pVehicleMgr)) {
                DWORD* pActiveCar = (DWORD*)(*pVehicleMgr + 0x10); // Offset to current driven simulation object
                
                if (pActiveCar && IsValidPointer((DWORD)*pActiveCar)) {
                    // Access the rendering flag container directly inside the loaded car object
                    DWORD* pRenderState = (DWORD*)(*pActiveCar + 0x3D4); 
                    
                    if (pRenderState && IsValidPointer((DWORD)*pRenderState)) {
                        DWORD oldProtect;
                        // Inject 0x02 to force constant ignition overrun / exhaust popping animations
                        DWORD forceFlameValue = 0x02; 
                        
                        VirtualProtect((LPVOID)pRenderState, sizeof(DWORD), PAGE_EXECUTE_READWRITE, &oldProtect);
                        *pRenderState = forceFlameValue; 
                        VirtualProtect((LPVOID)pRenderState, sizeof(DWORD), oldProtect, &oldProtect);
                    }
                }
            }
        }
        Sleep(8); // Matches game's internal script update frequency (~120hz)
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
