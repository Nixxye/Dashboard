#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <iostream>

#include "../include/Process.hpp"

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
    threads() {
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
    }
    Process::~Process() {
        
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
    unsigned int Process::getPriorityBase() {
        return priorityBase;
    }
    unsigned int Process::getPriorityClass() {
        return priorityClass;
    }
    unsigned int Process::getThreadCount() {
        return threadCount;
    }
    std::string Process::getUserName() {
        return userName;
    }
    void Process::loadMemoryInfo() {
        // N achei info, aqui foi chat na veia -> pesquisar na doc dps
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, id);
        if (!hProcess) {
            //std::cerr << "OpenProcess failed in loadMemoryInfo: " << GetLastError() << std::endl;
            return;
        }

        MEMORY_BASIC_INFORMATION mbi;
        uintptr_t addr = 0;

        unsigned long long totalCommitted = 0, totalReserved = 0, totalFree = 0;
        unsigned long long heapSize = 0, stackSize = 0, codeSize = 0;
        unsigned long long pageReadable = 0, pageWritable = 0, pageExecutable = 0;
        numberOfPages = 0;
        while (VirtualQueryEx(hProcess, reinterpret_cast<LPCVOID>(addr), &mbi, sizeof(mbi)) == sizeof(mbi)) {
            if (mbi.State == MEM_COMMIT) {
                totalCommitted += static_cast<unsigned long long>(mbi.RegionSize);

                DWORD protect = mbi.Protect;

                // Heurísticas para heap, stack e código
                if (protect == PAGE_READWRITE && mbi.Type == MEM_PRIVATE)
                    heapSize += static_cast<unsigned long long>(mbi.RegionSize);
                if ((protect & PAGE_EXECUTE) || (protect & PAGE_EXECUTE_READ) || (protect & PAGE_EXECUTE_READWRITE))
                    codeSize += static_cast<unsigned long long>(mbi.RegionSize);
                if (protect == PAGE_READWRITE && mbi.AllocationBase == mbi.BaseAddress)
                    stackSize += static_cast<unsigned long long>(mbi.RegionSize);

                numberOfPages += mbi.RegionSize / PAGE_SIZE;
                
            } else if (mbi.State == MEM_RESERVE) {
                totalReserved += static_cast<unsigned long long>(mbi.RegionSize);
            }
            addr += mbi.RegionSize;
        }

        CloseHandle(hProcess);

        this->memoryCommitted = totalCommitted / 1024;
        this->memoryReserved = totalReserved / 1024;
        this->memoryHeap = heapSize / 1024;
        this->memoryStack = stackSize / 1024;
        this->memoryCode = codeSize / 1024;
    }
}