#include "../include/Handle.hpp"
#include <winternl.h>
#include <iostream>

// typedef enum _OBJECT_INFORMATION_CLASS {
//     ObjectNameInformation = 1,
// } OBJECT_INFORMATION_CLASS;
namespace WindowsInfo {

    using NtQueryObject_t = NTSTATUS(WINAPI*)(
        HANDLE, OBJECT_INFORMATION_CLASS, PVOID, ULONG, PULONG
    );

    typedef struct _UNICODE_STRING {
        USHORT Length;
        USHORT MaximumLength;
        PWSTR  Buffer;
    } UNICODE_STRING;

    typedef struct _OBJECT_NAME_INFORMATION {
        UNICODE_STRING Name;
    } OBJECT_NAME_INFORMATION;

    typedef struct _OBJECT_TYPE_INFORMATION {
        UNICODE_STRING TypeName;
        // Outros campos ignorados
    } OBJECT_TYPE_INFORMATION;



    Handle::Handle(HANDLE handleValue) : handleValue(handleValue) {
        HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
        if (!hNtdll) return;

        auto NtQueryObject = reinterpret_cast<NtQueryObject_t>(GetProcAddress(hNtdll, "NtQueryObject"));
        if (!NtQueryObject) return;

        ULONG returnLength = 0;

        // Obtém o tipo do objeto
        BYTE typeBuffer[0x1000] = {};
        if (NtQueryObject(handleValue, ObjectTypeInformation, typeBuffer, sizeof(typeBuffer), &returnLength) == 0) {
            auto objType = reinterpret_cast<OBJECT_TYPE_INFORMATION*>(typeBuffer);
            if (objType->TypeName.Buffer && objType->TypeName.Length > 0) {
                type.assign(objType->TypeName.Buffer, objType->TypeName.Length / sizeof(WCHAR));
            }
        }

        // Se for do tipo File, tenta obter o nome (caminho)
        if (type == L"File" || type == L"Directory") {
            BYTE nameBuffer[0x1000] = {};
            returnLength = 0;
            if (NtQueryObject(handleValue, ObjectNameInformation, nameBuffer, sizeof(nameBuffer), &returnLength) == 0) {
                auto objName = reinterpret_cast<OBJECT_NAME_INFORMATION*>(nameBuffer);
                if (objName->Name.Buffer && objName->Name.Length > 0) {
                    std::wstring fullName(objName->Name.Buffer, objName->Name.Length / sizeof(WCHAR));
                    name = fullName;

                    // Converter para caminho de drive (ex: C:\...) se possível
                    WCHAR logicalDrives[256];
                    GetLogicalDriveStringsW(256, logicalDrives);

                    WCHAR deviceName[MAX_PATH];
                    WCHAR* drive = logicalDrives;
                    while (*drive) {
                        if (QueryDosDeviceW(drive, deviceName, MAX_PATH)) {
                            size_t devNameLen = wcslen(deviceName);
                            if (fullName.compare(0, devNameLen, deviceName) == 0) {
                                std::wstring finalPath = drive;
                                finalPath.pop_back(); // Remove barra no final
                                finalPath += fullName.substr(devNameLen);
                                name = finalPath;
                                break;
                            }
                        }
                        drive += wcslen(drive) + 1;
                    }
                }
            }
        }
    }


    crow::json::wvalue Handle::to_json() const {
        crow::json::wvalue j;
        j["type"] = std::string(type.begin(), type.end());
        j["name"] = std::string(name.begin(), name.end());
        return j;
    }

}
