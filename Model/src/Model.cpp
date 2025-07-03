#include "../include/Model.hpp"

void printError(TCHAR const *msg)
{
  // DWORD eNum;
  // TCHAR sysMsg[256];
  // TCHAR *p;

  // eNum = GetLastError();
  // FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
  //               NULL, eNum,
  //               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
  //               sysMsg, 256, NULL);

  // // Trim the end of the line and terminate it with a null
  // p = sysMsg;
  // while ((*p > 31) || (*p == 9))
  //   ++p;
  // do {
  //   *p-- = 0;
  // } while ((p >= sysMsg) && ((*p == '.') || (*p < 33)));

  // // Display the message
  // printf((char *)TEXT("\n  WARNING: %s failed with error %d (%s)"), msg, eNum,
  //        sysMsg);
}

std::string wchar_to_string(const wchar_t *wcharStr)
{
  if (wcharStr == nullptr)
  {
    return "";
  }

  // 1. Calcula o tamanho necessário para a string de destino (em UTF-8)
  int size_needed = WideCharToMultiByte(
      CP_UTF8,  // Converte para UTF-8
      0,        // Flags de conversão
      wcharStr, // String de origem (wide char)
      -1,       // -1 indica que a string de origem é terminada em nulo
      NULL, 0, NULL, NULL);

  if (size_needed == 0)
  {
    // Lidar com erro, se necessário
    return "";
  }

  // 2. Aloca a std::string com o tamanho calculado e faz a conversão
  std::string strTo(size_needed - 1, 0); // size_needed inclui o terminador nulo
  WideCharToMultiByte(
      CP_UTF8,
      0,
      wcharStr,
      -1,
      &strTo[0], // Ponteiro para o buffer da string de destino
      size_needed,
      NULL, NULL);

  return strTo;
}

namespace WindowsInfo
{

  Model::Model() : processes(), systemInfo() { updateProcesses(); }

  Model::~Model() { processes.clear(); }

  std::list<WindowsInfo::Process> Model::getProcesses()
  {
    updateProcesses();
    return processes;
  }

  WindowsInfo::System Model::getSystemInfo() { return systemInfo; }
  int Model::getThreadCount()
  {
    int threadCount = 0;
    for (WindowsInfo::Process process : processes)
    {
      threadCount += process.getThreadCount();
    }
    return threadCount;
  }
  // https://learn.microsoft.com/pt-br/windows/win32/toolhelp/taking-a-snapshot-and-viewing-processes
  void Model::updateProcesses()
  {
    processes.clear();

    HANDLE hProcessSnap;
    HANDLE hProcess;
    PROCESSENTRY32 pe32;
    DWORD dwPriorityClass;

    // Captura todos os processos em execução
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
      printError(TEXT("CreateToolhelp32Snapshot (of processes)"));
      return;
    }

    // Define o tamanho da estrutura
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Recupera informações sobre o primeiro processo
    if (!Process32First(hProcessSnap, &pe32))
    {
      printError(TEXT("Process32First")); // show cause of failure
      CloseHandle(hProcessSnap);          // clean the snapshot object
      return;
    }

    // Percorre todos os processos
    do
    {
      std::wstring ws(pe32.szExeFile);
      std::string name(ws.begin(), ws.end());
      processes.push_back(WindowsInfo::Process(
          name, (unsigned long)pe32.th32ProcessID,
          (unsigned long)pe32.th32ParentProcessID, (unsigned int)pe32.cntThreads,
          (unsigned int)pe32.pcPriClassBase));

      // Adquire a prioridade do processo
      // https://learn.microsoft.com/pt-br/windows/win32/api/processthreadsapi/nf-processthreadsapi-getpriorityclass
      dwPriorityClass = 0;
      hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
      if (hProcess == NULL)
        printError(TEXT("OpenProcess"));
      else
      {
        dwPriorityClass = GetPriorityClass(hProcess);
        if (!dwPriorityClass)
          printError(TEXT("GetPriorityClass"));
        CloseHandle(hProcess);
      }

    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
  }
  crow::json::wvalue Model::to_json()
  {
    crow::json::wvalue j;

    // Serializa a lista de processos
    crow::json::wvalue processesJson = crow::json::wvalue::list();
    int index = 0;
    for (auto &process : getProcesses())
    {
      processesJson[index++] = process.to_json();
    }
    j["processes"] = std::move(processesJson);

    // Serializa o objeto System
    j["systemInfo"] = systemInfo.to_json();

    // Opcional: adicionar contagem total de threads
    j["threadCount"] = getThreadCount();

    return j;
  }

  void GetDiskUsage(FileSystemInfo::VolumeInfo& volumeInfo)
  {
    ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;
    const wchar_t *widecstr = std::wstring(volumeInfo.Path.begin(), volumeInfo.Path.end()).c_str();

    if (GetDiskFreeSpaceExW(widecstr, &freeBytesAvailable, &totalBytes, &totalFreeBytes))
    {
      ULONGLONG total = totalBytes.QuadPart;
      ULONGLONG free = totalFreeBytes.QuadPart;
      ULONGLONG used = total - free;
      volumeInfo.UsedMemory = used;
      volumeInfo.FreeMemory = free;
      volumeInfo.TotalMemory = total;

      std::wcout << L"Drive: " << widecstr << std::endl;
      std::wcout << L"  Total Size: " << total / (1024 * 1024 * 1024) << L" GB" << std::endl;
      std::wcout << L"  Used Space: " << used / (1024 * 1024 * 1024) << L" GB" << std::endl;
      std::wcout << L"  Free Space: " << free / (1024 * 1024 * 1024) << L" GB" << std::endl;
      std::wcout << std::endl;
    }
    else
    {
      std::wcerr << L"Failed to get disk info for " << widecstr << std::endl;
    }
  }

  std::vector<FileSystemInfo::VolumeInfo> Model::GetVolumesInfo()
  {
    wchar_t volumeName[MAX_PATH];
    HANDLE findHandle = FindFirstVolumeW(volumeName, ARRAYSIZE(volumeName));

    if (findHandle == INVALID_HANDLE_VALUE)
    {
      DWORD error = GetLastError();
      std::wcerr << L"FindFirstVolumeW failed with error: " << error << std::endl;
      exit(1);
    }

    std::wcout << L"System Volumes:\n"
               << std::endl;

    this->volumesInfo.clear();
    do
    {
      FileSystemInfo::VolumeInfo temp;
      std::wcout << L"Volume GUID: " << volumeName << std::endl;

      DWORD pathNamesLength = 0;
      // First call to get the required buffer size.
      if (!GetVolumePathNamesForVolumeNameW(volumeName, nullptr, 0, &pathNamesLength) && GetLastError() != ERROR_MORE_DATA)
      {
        DWORD error = GetLastError();
        std::wcerr << L"GetVolumePathNamesForVolumeNameW (pre-check) failed with error: " << error << std::endl;
      }
      else
      {
        temp.Guid.clear();
        temp.Guid.append((char *)volumeName);
        if (pathNamesLength > 0)
        {
          std::vector<wchar_t> pathNames(pathNamesLength);
          if (GetVolumePathNamesForVolumeNameW(volumeName, pathNames.data(), pathNamesLength, &pathNamesLength))
          {
            std::wcout << L"  Paths: ";
            for (const wchar_t *path = pathNames.data(); *path; path += wcslen(path) + 1)
            {
              temp.Path.clear();
              temp.Path.append(wchar_to_string(path));
              std::wcout << temp.Path.c_str() << L" ";
              this->GetVolumeInfo(temp);
              GetDiskUsage(temp);
            }
            std::wcout << std::endl;
          }
          else
          {
            DWORD error = GetLastError();
            std::wcerr << L"GetVolumePathNamesForVolumeNameW failed with error: " << error << std::endl;
          }
        }
      }
      std::wcout << L"----------------------------------------" << std::endl;
      if (temp.Path.length() > 1)
        this->volumesInfo.push_back(temp);

    } while (FindNextVolumeW(findHandle, volumeName, ARRAYSIZE(volumeName)));

    FindVolumeClose(findHandle);
    return this->volumesInfo; // Exit successfully
  }

  int Model::GetVolumeInfo(FileSystemInfo::VolumeInfo &volumeInfo)
  {
    // The path to the volume you want to inspect.
    // Note the L prefix to indicate a wide character string.

    // Buffers to store the output information.
    wchar_t volumeNameBuffer[MAX_PATH + 1];
    wchar_t fileSystemNameBuffer[MAX_PATH + 1];

    // Variables to hold other volume information.
    DWORD volumeSerialNumber;
    DWORD maximumComponentLength;
    DWORD fileSystemFlags;

    // Call the GetVolumeInformationW function.
    // This function retrieves information about the file system and volume
    // associated with the specified root directory.
    const wchar_t *widecstr = std::wstring(volumeInfo.Path.begin(), volumeInfo.Path.end()).c_str();
    BOOL success = GetVolumeInformationW(
        widecstr,                                      // lpRootPathName
        volumeNameBuffer,                              // lpVolumeNameBuffer
        sizeof(volumeNameBuffer) / sizeof(wchar_t),    // nVolumeNameSize
        &volumeSerialNumber,                           // lpVolumeSerialNumber
        &maximumComponentLength,                       // lpMaximumComponentLength
        &fileSystemFlags,                              // lpFileSystemFlags
        fileSystemNameBuffer,                          // lpFileSystemNameBuffer
        sizeof(fileSystemNameBuffer) / sizeof(wchar_t) // nFileSystemNameSize
    );

    // Check if the function call was successful.
    if (!success)
    {
      return 1; // Exit with an error code
    }

    // Print the retrieved information to the console.
    std::wcout << L"Information for volume: " << volumeInfo.Path.c_str() << std::endl;
    std::wcout << L"---------------------------------" << std::endl;

    volumeInfo.VolumeName = wchar_to_string(volumeNameBuffer);
    // Using iomanip for better formatting
    std::wcout << std::left << std::setw(25) << L"Volume name" << L": " << volumeNameBuffer << std::endl;
    std::wcout << std::left << std::setw(25) << L"Serial number" << L": " << volumeSerialNumber << std::endl;

    volumeInfo.SerialNumber = volumeSerialNumber;
    // Print serial number in hex format
    std::wcout << std::left << std::setw(25) << L"Serial number hex" << L": 0x" << std::hex << std::uppercase << volumeSerialNumber << std::dec << std::nouppercase << std::endl;

    std::wcout << std::left << std::setw(25) << L"Max component length" << L": " << maximumComponentLength << std::endl;
    std::wcout << std::left << std::setw(25) << L"File system flags" << L": " << fileSystemFlags << std::endl;
    std::wcout << std::left << std::setw(25) << L"File system name" << L": " << fileSystemNameBuffer << std::endl;
    return 0;
  }

  std::chrono::system_clock::time_point Model::FileTimeToSystemClock(const FILETIME &ft)
  {
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    uint64_t filetime_ticks = uli.QuadPart;

    const uint64_t UNIX_EPOCH_AS_FILETIME_TICKS = 116444736000000000ULL;

    uint64_t ticks_since_unix_epoch = filetime_ticks - UNIX_EPOCH_AS_FILETIME_TICKS;

    return std::chrono::system_clock::time_point{std::chrono::duration<long long, std::ratio<1, 10000000>>{static_cast<long long>(ticks_since_unix_epoch)}};
  }

  bool Model::GetDirectoryInfo(std::string path, std::vector<FileSystemInfo::FileInfo> &filesInfo)
  {
    WIN32_FIND_DATA ffd;
    LARGE_INTEGER filesize;
    TCHAR szDir[MAX_PATH];
    size_t length_of_arg;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    DWORD dwError = 0;
    const wchar_t *widecstr = std::wstring(path.begin(), path.end()).c_str();
    StringCchLength(widecstr, MAX_PATH, &length_of_arg);

    if (length_of_arg > (MAX_PATH - 3))
    {
      return true;
    }

    // Prepare string for use with FindFile functions.  First, copy the
    // string to a buffer, then append '\*' to the directory name.

    StringCchCopy(szDir, MAX_PATH, widecstr);
    StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

    // Find the first file in the directory.

    hFind = FindFirstFileW(szDir, &ffd);

    if (INVALID_HANDLE_VALUE == hFind)
    {
      std::cout << "deu merda" << std::endl;
      return true;
    }

    // List all the files in the directory with some info about them.

    this->filesInfo.clear();
    do
    {
      FileSystemInfo::FileInfo temp;
      if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        std::cout << wchar_to_string(ffd.cFileName) << "/\n";
        temp.FileName = wchar_to_string(ffd.cFileName) + "/";
      }
      else
      {
        filesize.LowPart = ffd.nFileSizeLow;
        filesize.HighPart = ffd.nFileSizeHigh;
        std::cout << wchar_to_string(ffd.cFileName) << " " << ((unsigned long long)filesize.QuadPart) << " bytes\n";
        temp.FileName = wchar_to_string(ffd.cFileName);
        temp.FileSize = ((unsigned long long)filesize.QuadPart);
      }
      temp.CreationTime = FileTimeToSystemClock(ffd.ftCreationTime);
      temp.LastAccessTime = FileTimeToSystemClock(ffd.ftLastAccessTime);
      temp.LastWriteTime = FileTimeToSystemClock(ffd.ftLastWriteTime);

      filesInfo.push_back(temp);
    } while (FindNextFileW(hFind, &ffd) != 0);

    dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES)
    {
      return false;
    }

    FindClose(hFind);
    return false;
  }

} // namespace WindowsInfo