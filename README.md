# Dashboard
Desenvolvido pelos alunos Jean Carlos do Nascimento Cunha e Murilo Motalvão Pereira para a disciplina de Sistemas Operacionais (CSO30), UTPFR Curitiba, sob orientação do professor Marco Aurélio Wehrmeister
* Software de Dashboard similar ao gerenciador de tarefas.
* Feito em C++ com a biblioteca imgui para a interface gráfica.
* Faz uso direto da api do windows para trazer as informações.

A seguir, uma descrição das telas de nossa aplicação:

Informações gerais do sistema

<img src="Img/infoGerais.png" alt="Informações gerais" />

* Uso da CPU
    * NtQuerySystemInformation com parâmetro SystemProcessorPerformanceInformation
* Uso de memória
    * GlobalMemoryStatusEx
* Número de processos
    * CreateToolhelp32Snapshot + Process32First
* Número de threads
    * CreateToolhelp32Snapshot + Process32First

<img src="Img/graficosCPUs.png" alt="Gráficos das CPUs">

* Uso da CPU
    * NtQuerySystemInformation com parâmetro SystemProcessorPerformanceInformation


<img src=Img/listaProcessos.png alt="Lista de Processos">

* Nome do processo
    * CreateToolhelp32Snapshot + Process32First
* PID
    * CreateToolhelp32Snapshot + Process32First
* Id do pai
    * CreateToolhelp32Snapshot + Process32First
* Usuário
    * OpenProcessToken + GetTokenInformation + LookupAccoutSidW
* Memory Working Set
    * VirtualQueryEX
* Memory Commited
    * VirtualQueryEX
* Private Memory Commited
    * VirtualQueryEX
* Memory Reserved
    * VirtualQueryEX
* Número de Páginas
    * VirtualQueryEX
* Priority Base
    * CreateToolhelp32Snapshot + Process32First
* Priority Class
    * GetPriorityClass
* Número de Threads
    * CreateToolhelp32Snapshot + Process32First