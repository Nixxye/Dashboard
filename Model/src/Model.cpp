#include "../include/Model.hpp"

void printError(TCHAR const *msg) {
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

namespace WindowsInfo {

Model::Model() : processes(), systemInfo() { updateProcesses(); }

Model::~Model() { processes.clear(); }

std::list<WindowsInfo::Process> Model::getProcesses() {
  updateProcesses();
  return processes;
}

WindowsInfo::System Model::getSystemInfo() { return systemInfo; }
int Model::getThreadCount() {
  int threadCount = 0;
  for (WindowsInfo::Process process : processes) {
    threadCount += process.getThreadCount();
  }
  return threadCount;
}
// https://learn.microsoft.com/pt-br/windows/win32/toolhelp/taking-a-snapshot-and-viewing-processes
void Model::updateProcesses() {
  processes.clear();

  HANDLE hProcessSnap;
  HANDLE hProcess;
  PROCESSENTRY32 pe32;
  DWORD dwPriorityClass;

  // Captura todos os processos em execução
  hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hProcessSnap == INVALID_HANDLE_VALUE) {
    printError(TEXT("CreateToolhelp32Snapshot (of processes)"));
    return;
  }

  // Define o tamanho da estrutura
  pe32.dwSize = sizeof(PROCESSENTRY32);

  // Recupera informações sobre o primeiro processo
  if (!Process32First(hProcessSnap, &pe32)) {
    printError(TEXT("Process32First")); // show cause of failure
    CloseHandle(hProcessSnap);          // clean the snapshot object
    return;
  }

  // Percorre todos os processos
  do {
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
    else {
      dwPriorityClass = GetPriorityClass(hProcess);
      if (!dwPriorityClass)
        printError(TEXT("GetPriorityClass"));
      CloseHandle(hProcess);
    }

  } while (Process32Next(hProcessSnap, &pe32));

  CloseHandle(hProcessSnap);
}
} // namespace WindowsInfo