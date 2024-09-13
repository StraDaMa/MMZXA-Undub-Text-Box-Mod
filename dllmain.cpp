// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#define EXTERN_DLL_EXPORT extern "C" __declspec(dllexport)

EXTERN_DLL_EXPORT void mod_open() {
    HMODULE exeBase = GetModuleHandleA("MZZXLC.exe");
    if (exeBase == NULL) {
        return;
    }
    uint8_t* exeBasePtr = (uint8_t*)exeBase;
    PIMAGE_DOS_HEADER exeDosHeader = (PIMAGE_DOS_HEADER)exeBasePtr;
    PIMAGE_NT_HEADERS exeNtHeader = (PIMAGE_NT_HEADERS)(exeBasePtr + exeDosHeader->e_lfanew);
    PIMAGE_SECTION_HEADER firstSection = (PIMAGE_SECTION_HEADER)(IMAGE_FIRST_SECTION(exeNtHeader));
    uint8_t* textSectionPtr = exeBasePtr + firstSection->VirtualAddress;
    size_t textSectionSize = firstSection->Misc.VirtualSize;
    // This pattern finds all 2 functions that reference the message box font / names
    const static std::array<uint8_t, 0x10> messageBoxNameFuncPattern = {
        0x40, 0x53,                  //PUSH RBX
        0x57,                        //PUSH RDI
        0x41, 0x57,                  //PUSH R15
        0x48, 0x83, 0xEC, 0x30,      //SUB RSP,0x30
        0x48, 0x89, 0x6C, 0x24, 0x58,//MOV QWORD PTR SS:[RSP+0x58],RBP
        0x4C, 0x8D                   //LEA R15,QWORD PTR DS:[0x140000000]
    };
    uint8_t* textSectionEndPtr = textSectionPtr + textSectionSize;
    uint8_t* targetFunctionPtr = textSectionPtr;
    // There's 2 functions for message box fonts / names
    for (size_t i = 0; i < 2; i++) {
        targetFunctionPtr = std::search(
            targetFunctionPtr + messageBoxNameFuncPattern.size(), textSectionEndPtr,
            messageBoxNameFuncPattern.data(), messageBoxNameFuncPattern.data() + messageBoxNameFuncPattern.size()
        );
        // Pattern not found anymore
        if (targetFunctionPtr == textSectionEndPtr) {
            break;
        }
        // Update referenced offsets from Japanese to English ones
        // Instead of copying the English assets over the Japanese ones
        int movInst1Offset = 0x3E;
        *(uint32_t*)(targetFunctionPtr + movInst1Offset + 0x5) += 0x14 * 0x60;
        int leaInstOffset = 0x53;
        *(uint32_t*)(targetFunctionPtr + leaInstOffset + 0x3) += 0x1F00;
        int movInst2Offset = 0x8A;
        *(uint32_t*)(targetFunctionPtr + movInst2Offset + 0x5) += 0x14 * 0x60;
    }
    return;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

