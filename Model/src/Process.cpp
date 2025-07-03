#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#endif
#include <psapi.h>
#include <iostream>
#include <tchar.h>
#include <tlhelp32.h>
#include <winnt.h>
#include "../include/Process.hpp"

#ifndef STATUS_INFO_LENGTH_MISMATCH
#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004)
#endif

constexpr size_t PAGE_SIZE = 4096;

namespace WindowsInfo {
    Process::Process(std::string name, unsigned long id, unsigned long parentId, unsigned int threadCount, unsigned int priorityBase) :
    name(name),
    id(id),
    parentId(parentId),
    threadCount(threadCount),
    priorityBase(priorityBase),
    priorityClass(0),
    memoryCommitted(0),
    memoryReserved(0),
    memoryHeap(0),
    memoryStack(0),
    memoryCode(0),
    numberOfPages(0),
    threads(), memoryWorkingSet(0), memoryPrivateCommited(0)   {
        updateInfo();
    }
    void Process::updateInfo() {
        //Adiciona nome do processo
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, id);
        if( hProcess == NULL ) {
            //std::cerr << "OpenProcess failed: " << GetLastError() << std::endl;

            // printError( TEXT("OpenProcess") );
        }
        else {
            priorityClass = GetPriorityClass( hProcess );
            if( !priorityClass )
                std::cerr << "GetPriorityClass failed: " << GetLastError() << std::endl;
                // printError( TEXT("GetPriorityClass") );

            //https://stackoverflow.com/questions/2686096/c-get-username-from-process
            //https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-openprocesstoken
            // Abre token do processo
            HANDLE hToken = nullptr;
            if (!OpenProcessToken(hProcess, TOKEN_QUERY, &hToken)) {
                CloseHandle(hProcess);
                return;
            }
            // Determina o tamanho necessário do buffer
            DWORD tokenInfoLength = 0;
            GetTokenInformation(hToken, TokenUser, nullptr, 0, &tokenInfoLength);
            if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
                CloseHandle(hToken);
                CloseHandle(hProcess);
                return;
            }
            // Adquire o token
            BYTE* tokenInfo = new BYTE[tokenInfoLength];
            if (!GetTokenInformation(hToken, TokenUser, tokenInfo, tokenInfoLength, &tokenInfoLength)) {
                delete[] tokenInfo;
                CloseHandle(hToken);
                CloseHandle(hProcess);
                return;
            }

            TOKEN_USER* tokenUser = reinterpret_cast<TOKEN_USER*>(tokenInfo);
            WCHAR name[256], domain[256];
            DWORD nameLen = 256, domainLen = 256;
            SID_NAME_USE sidType;
            // https://learn.microsoft.com/pt-br/windows/win32/api/winbase/nf-winbase-lookupaccountsidw
            // Recupera nome através do SID (identificador do usuário)
            if (LookupAccountSidW(nullptr, tokenUser->User.Sid, name, &nameLen, domain, &domainLen, &sidType)) {
                std::wstring wuser = std::wstring(domain) + L"\\" + name;
                userName = std::string(wuser.begin(), wuser.end()); 

            } else {
                delete[] tokenInfo;
                CloseHandle(hToken);
                CloseHandle(hProcess);
                return;
            }

            delete[] tokenInfo;
            CloseHandle(hToken);
            CloseHandle( hProcess );
        }
        // loadHandles();
    }
    //https://learn.microsoft.com/en-us/windows/win32/toolhelp/traversing-the-thread-list
    std::list<Thread> Process::getThreads() {
        threads.clear();
        HANDLE hThreadSnap = INVALID_HANDLE_VALUE; 
        THREADENTRY32 te32; 
        // Captura todas as threads em execução 
        hThreadSnap = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 ); 
        if( hThreadSnap == INVALID_HANDLE_VALUE ) 
            return std::list<Thread>(); 
        
        // Preenche o tamanho da estrutura
        te32.dwSize = sizeof(THREADENTRY32); 
        
        // Recupera informações da primeira thread
        if( !Thread32First( hThreadSnap, &te32 ) ) 
        {
            std::cerr << "Thread32First failed: " << GetLastError() << std::endl;
            // printError( TEXT("Thread32First") ); // show cause of failure
            CloseHandle( hThreadSnap );          // clean the snapshot object
            return std::list<Thread>(); 
        }

        // Percorre todas as threads verificando se pertencem ao processo
        do 
        { 
            if( te32.th32OwnerProcessID == id )
            {
                Thread thread = Thread(te32.th32ThreadID, te32.tpBasePri, te32.tpDeltaPri);
                threads.push_back(thread);
            }
        } while( Thread32Next(hThreadSnap, &te32 ) ); 

        CloseHandle( hThreadSnap );
        return threads;
    }

    Process::~Process() {}
    unsigned int Process::getPriorityBase() { return priorityBase; }
    unsigned int Process::getPriorityClass() { return priorityClass; }
    unsigned int Process::getThreadCount() { return threadCount; }
    std::string Process::getUserName() { return userName; }
    void Process::loadMemoryInfo() {
      // N achei info, aqui foi chat na veia -> pesquisar na doc dps
      HANDLE hProcess =
          OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, id);
      if (!hProcess) {
        // std::cerr << "OpenProcess failed in loadMemoryInfo: " << GetLastError()
        // << std::endl;
        return;
      }

      MEMORY_BASIC_INFORMATION mbi;
      uintptr_t addr = 0;

      unsigned long long totalCommitted = 0, totalReserved = 0, totalFree = 0;
      unsigned long long heapSize = 0, stackSize = 0, codeSize = 0;
      unsigned long long pageReadable = 0, pageWritable = 0, memoryPrivateCommitedSize = 0, pageExecutable = 0;
      numberOfPages = 0;
      while (VirtualQueryEx(hProcess, reinterpret_cast<LPCVOID>(addr), &mbi,
                            sizeof(mbi)) == sizeof(mbi)) {
        if (mbi.State == MEM_COMMIT) {
          totalCommitted += static_cast<unsigned long long>(mbi.RegionSize);

          DWORD protect = mbi.Protect;

          // Heurísticas para heap, stack e código
          if (protect == PAGE_READWRITE && mbi.Type == MEM_PRIVATE)
            heapSize += static_cast<unsigned long long>(mbi.RegionSize);
          if ((protect & PAGE_EXECUTE) || (protect & PAGE_EXECUTE_READ) ||
              (protect & PAGE_EXECUTE_READWRITE))
            codeSize += static_cast<unsigned long long>(mbi.RegionSize);
          if (protect == PAGE_READWRITE && mbi.AllocationBase == mbi.BaseAddress)
            stackSize += static_cast<unsigned long long>(mbi.RegionSize);

          
          // https://learn.microsoft.com/pt-br/windows/win32/api/winnt/ns-winnt-memory_basic_information
          if (mbi.Type == MEM_PRIVATE && mbi.State == MEM_COMMIT)
            memoryPrivateCommitedSize += static_cast<unsigned long long>(mbi.RegionSize);
        

          numberOfPages += mbi.RegionSize / PAGE_SIZE;

        } else if (mbi.State == MEM_RESERVE) {
          totalReserved += static_cast<unsigned long long>(mbi.RegionSize);
        }
        addr += mbi.RegionSize;
      }

      // https://stackoverflow.com/questions/1984186/what-is-private-bytes-virtual-bytes-working-set 
      // https://learn.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-getprocessmemoryinfo 
      PROCESS_MEMORY_COUNTERS pMemCounters;
      if (GetProcessMemoryInfo(hProcess, &pMemCounters, sizeof(pMemCounters))) {
        this->memoryWorkingSet = pMemCounters.WorkingSetSize/1024;
      }

      CloseHandle(hProcess);

      this->memoryCommitted = totalCommitted / 1024;
      this->memoryPrivateCommited = memoryPrivateCommitedSize / 1024;
      this->memoryReserved = totalReserved / 1024;
      this->memoryHeap = heapSize / 1024;
      this->memoryStack = stackSize / 1024;
      this->memoryCode = codeSize / 1024;
    }
    struct SYSTEM_HANDLE_TABLE_ENTRY_INFO {
        USHORT UniqueProcessId;
        USHORT CreatorBackTraceIndex;
        BYTE ObjectTypeNumber;
        BYTE Flags;
        USHORT Handle;
        PVOID Object;
        ACCESS_MASK GrantedAccess;
    };

    struct SYSTEM_HANDLE_INFORMATION {
        ULONG NumberOfHandles;
        SYSTEM_HANDLE_TABLE_ENTRY_INFO Handles[1]; 
    };

    using NtQuerySystemInformation_t = NTSTATUS(WINAPI*)(
        ULONG SystemInformationClass,
        PVOID SystemInformation,
        ULONG SystemInformationLength,
        PULONG ReturnLength);

    void Process::loadHandles() {
        semaphores.clear();
        mutexes.clear();
        diskFiles.clear();
        charFiles.clear();
        pipeFiles.clear();
        unknownFiles.clear();
        directories.clear();
        devices.clear();

        // Abre handle do processo para duplicar handles
        HANDLE hProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, id);
        if (!hProcess) {
            std::cerr << "OpenProcess (PROCESS_DUP_HANDLE) failed for PID " << id << ": " << GetLastError() << std::endl;
            return;
        }

        // Pega ponteiro para NtQuerySystemInformation
        HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
        if (!hNtdll) {
            std::cerr << "GetModuleHandleW failed for ntdll.dll: " << GetLastError() << std::endl;
            CloseHandle(hProcess);
            return;
        }
        auto NtQuerySystemInformation = (NtQuerySystemInformation_t)GetProcAddress(hNtdll, "NtQuerySystemInformation");
        if (!NtQuerySystemInformation) {
            std::cerr << "GetProcAddress failed for NtQuerySystemInformation: " << GetLastError() << std::endl;
            CloseHandle(hProcess);
            return;
        }

        ULONG bufferSize = 0x10000;
        std::vector<BYTE> buffer(bufferSize);
        ULONG returnLength = 0;
        NTSTATUS status;
        const ULONG MAX_BUFFER_SIZE = 512 * 1024 * 1024; // 512 MB

        while ((status = NtQuerySystemInformation(16, buffer.data(), bufferSize, &returnLength)) == STATUS_INFO_LENGTH_MISMATCH) {
            if (bufferSize >= MAX_BUFFER_SIZE) {
                std::cerr << "Buffer size exceeded safe limit (" << MAX_BUFFER_SIZE / (1024 * 1024) << " MB), aborting.\n";
                CloseHandle(hProcess);
                return;
            }
            bufferSize *= 2;
            buffer.resize(bufferSize);
            std::cerr << "Resizing buffer to " << bufferSize / 1024 << " KB." << std::endl;
        }

        if (status != 0) {
            std::cerr << "NtQuerySystemInformation failed with status: 0x" << std::hex << status << std::dec << std::endl;
            CloseHandle(hProcess);
            return;
        }

        auto handleInfo = reinterpret_cast<SYSTEM_HANDLE_INFORMATION*>(buffer.data());
        std::cout << "Total system handles enumerated: " << handleInfo->NumberOfHandles << std::endl;

        const ULONG MAX_HANDLES_TO_PROCESS = 100000;
        ULONG processedHandles = 0;

        for (ULONG i = 0; i < handleInfo->NumberOfHandles && i < MAX_HANDLES_TO_PROCESS; ++i) {
            auto& h = handleInfo->Handles[i];
            // Verifica se o handle pertence ao processo atual
            if ((DWORD)h.UniqueProcessId != id) {
                continue; 
            }
            processedHandles++;

            HANDLE sourceProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, h.UniqueProcessId);
            if (!sourceProcess) {

                continue;
            }

            HANDLE dupHandle = nullptr;
            if (DuplicateHandle(sourceProcess, (HANDLE)(uintptr_t)h.Handle, GetCurrentProcess(), &dupHandle, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
                Handle handleObj(dupHandle);
                
                if (handleObj.type.empty()) {
                    std::cerr << "Handle object type is empty for duplicated handle 0x" << std::hex << dupHandle
                              << " from PID " << h.UniqueProcessId << ", original handle 0x" << h.Handle << std::dec << std::endl;
                    CloseHandle(dupHandle);
                    CloseHandle(sourceProcess);
                    continue;
                }

                if (handleObj.type == L"Mutant") {
                    mutexes.push_back(handleObj);
                } else if (handleObj.type == L"Semaphore") {
                    semaphores.push_back(handleObj);
                } else if (handleObj.type == L"File") {
                    DWORD fileType = GetFileType(handleObj.handleValue);
                    if (fileType == FILE_TYPE_UNKNOWN && GetLastError() != NO_ERROR) {
                        std::cerr << "GetFileType failed for File handle 0x" << std::hex << handleObj.handleValue
                                  << ": " << GetLastError() << std::dec << std::endl;
                    }
                    switch (fileType) {
                        case FILE_TYPE_CHAR:
                            charFiles.push_back(handleObj);
                            break;
                        case FILE_TYPE_DISK:
                            diskFiles.push_back(handleObj);
                            break;
                        case FILE_TYPE_PIPE:
                            pipeFiles.push_back(handleObj);
                            break;
                        case FILE_TYPE_UNKNOWN:
                            unknownFiles.push_back(handleObj);
                            break;
                        default:
                            break;
                    }
                } else if (handleObj.type == L"Directory") {
                    directories.push_back(handleObj);
                } else if (handleObj.type == L"Device") {
                    devices.push_back(handleObj);
                } else {
                    // std::cout << "Unhandled handle type: " << std::string(handleObj.type.begin(), handleObj.type.end()) << std::endl;
                }

                CloseHandle(dupHandle);
            } else {
                std::cerr << "DuplicateHandle failed for PID " << h.UniqueProcessId << ", original handle 0x" << std::hex << h.Handle
                          << std::dec << " (Error: " << GetLastError() << ")." << std::endl;
            }
            CloseHandle(sourceProcess);
        }
        std::cout << "Processed " << processedHandles << " handles for PID " << id << std::endl;
        std::cout << "Successfully categorized handles for PID " << id << ": "
                  << "Semaphores: " << semaphores.size()
                  << ", Mutexes: " << mutexes.size()
                  << ", Disk Files: " << diskFiles.size()
                  << ", Char Files: " << charFiles.size()
                  << ", Pipe Files: " << pipeFiles.size()
                  << ", Unknown Files: " << unknownFiles.size()
                  << ", Directories: " << directories.size()
                  << ", Devices: " << devices.size() << std::endl;

        CloseHandle(hProcess);
    }

    crow::json::wvalue Process::to_json_simple() {
        crow::json::wvalue j;
        j["id"] = id;
        j["parentId"] = parentId;
        j["name"] = name;
        return j;
    }
    crow::json::wvalue Process::to_json() {
        crow::json::wvalue j;
        j["id"] = id;
        j["parentId"] = parentId;
        j["name"] = name;
        j["threadCount"] = threadCount;
        j["priorityBase"] = priorityBase;
        j["priorityClass"] = priorityClass;
        j["userName"] = userName;
        std::cout << "Carregando informações de memória do processo " << id << std::endl;
        loadMemoryInfo();
        j["memoryWorkingSet"] = getMemoryWorkingSet();
        j["memoryCommitted"] = getMemoryCommitted();
        j["privateMemoryCommitted"] = getPrivateMemoryCommitted();
        j["memoryReserved"] = getMemoryReserved();
        j["memoryHeap"] = getMemoryHeap();
        j["memoryStack"] = getMemoryStack();
        j["memoryCode"] = getMemoryCode();
        j["numberOfPages"] = getNumberOfPages();

        // Serializa a lista de threads
        std::cout << "Carregando threads do processo " << id << std::endl;
        crow::json::wvalue threadsJson = crow::json::wvalue::list();
        int index = 0;
        std::list<Thread> threads = getThreads();
        for ( auto& thread : threads) {
            threadsJson[index++] = thread.to_json();
        }
        j["threads"] = std::move(threadsJson);
        // Serializa a lista de handles
        std::cout << "Carregando handles do processo " << id << std::endl;
        try {
            loadHandles();
        } catch (const std::exception& e) {
            std::cerr << "Erro carregando handles do processo " << id << ": " << e.what() << std::endl;
        }
        std::cout << "Handles carregados: " << semaphores.size() + mutexes.size() + diskFiles.size() + charFiles.size() + pipeFiles.size() + unknownFiles.size() + directories.size() + devices.size() << std::endl;
        crow::json::wvalue semaphoresJson = crow::json::wvalue::list();
        index = 0;
        for (auto& handle : semaphores) {
            semaphoresJson[index++] = handle.to_json();
        }
        j["semaphores"] = std::move(semaphoresJson);

        crow::json::wvalue mutexesJson = crow::json::wvalue::list();
        index = 0;
        for (auto& handle : mutexes) {
            mutexesJson[index++] = handle.to_json();
        }
        j["mutexes"] = std::move(mutexesJson);

        crow::json::wvalue diskFilesJson = crow::json::wvalue::list();
        index = 0;
        for (auto& handle : diskFiles) {
            diskFilesJson[index++] = handle.to_json();
        }
        j["diskFiles"] = std::move(diskFilesJson);

        crow::json::wvalue charFilesJson = crow::json::wvalue::list();
        index = 0;
        for (auto& handle : charFiles) {
            charFilesJson[index++] = handle.to_json();
        }
        j["charFiles"] = std::move(charFilesJson);

        crow::json::wvalue pipeFilesJson = crow::json::wvalue::list();
        index = 0;
        for (auto& handle : pipeFiles) {
            pipeFilesJson[index++] = handle.to_json();
        }
        j["pipeFiles"] = std::move(pipeFilesJson);

        crow::json::wvalue unknownFilesJson = crow::json::wvalue::list();
        index = 0;
        for (auto& handle : unknownFiles) {
            unknownFilesJson[index++] = handle.to_json();
        }
        j["unknownFiles"] = std::move(unknownFilesJson);

        crow::json::wvalue directoriesJson = crow::json::wvalue::list();
        index = 0;
        for (auto& handle : directories) {
            directoriesJson[index++] = handle.to_json();
        }
        j["directories"] = std::move(directoriesJson);

        crow::json::wvalue devicesJson = crow::json::wvalue::list();
        index = 0;
        for (auto& handle : devices) {
            devicesJson[index++] = handle.to_json();
        }
        j["devices"] = std::move(devicesJson);
        return j;
    }
} // namespace WindowsInfo