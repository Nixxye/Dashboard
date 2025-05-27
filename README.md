# Dashboard

Desenvolvido pelos alunos Jean Carlos do Nascimento Cunha e Murilo Motalvão Pereira para a disciplina de Sistemas Operacionais (CSO30), da UTPFR Curitiba, sob orientação do professor Marco Aurélio Wehrmeister.

- Software de dashboard semelhante a um gerenciador de tarefas.
- Desenvolvido em C++ utilizando a biblioteca **imgui** para a interface gráfica.
- Utiliza diretamente a API do Windows para obter informações do sistema.

---

## Descrição das Telas da Aplicação

### Informações Gerais do Sistema

![Informações gerais](Img/infoGerais.png)

- **Uso da CPU**
  - Utiliza `NtQuerySystemInformation` com o parâmetro `SystemProcessorPerformanceInformation`.
- **Uso de Memória**
  - Utiliza `GlobalMemoryStatusEx`.
- **Número de Processos**
  - Utiliza `CreateToolhelp32Snapshot` + `Process32First`.
- **Número de Threads**
  - Utiliza `CreateToolhelp32Snapshot` + `Process32First`.

---

![Gráficos das CPUs](Img/graficosCPUs.png)

- **Uso da CPU**
  - Utiliza `NtQuerySystemInformation` com o parâmetro `SystemProcessorPerformanceInformation`.

---

![Lista de Processos](Img/listaProcessos.png)

- **Nome do Processo**
  - Utiliza `CreateToolhelp32Snapshot` + `Process32First`.
- **PID**
  - Utiliza `CreateToolhelp32Snapshot` + `Process32First`.
- **ID do Processo Pai**
  - Utiliza `CreateToolhelp32Snapshot` + `Process32First`.
- **Usuário**
  - Utiliza `OpenProcessToken` + `GetTokenInformation` + `LookupAccountSidW`.
- **Memory Working Set**
  - Utiliza `VirtualQueryEX`.
- **Memory Committed**
  - Utiliza `VirtualQueryEX`.
- **Private Memory Committed**
  - Utiliza `VirtualQueryEX`.
- **Memory Reserved**
  - Utiliza `VirtualQueryEX`.
- **Número de Páginas**
  - Utiliza `VirtualQueryEX`.
- **Prioridade Base (Processo)**
  - Utiliza `CreateToolhelp32Snapshot` + `Process32First`.
- **Classe de Prioridade**
  - Utiliza `GetPriorityClass`.
- **Número de Threads**
  - Utiliza `CreateToolhelp32Snapshot` + `Process32First`.
- **Prioridade Base (Thread)**
  - Utiliza `CreateToolhelp32Snapshot` + `Thread32First`.
- **Classe de Prioridade (Thread)**
  - Utiliza `CreateToolhelp32Snapshot` + `Thread32First`.
