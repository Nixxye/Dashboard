<script setup lang="ts">
import { ref, watch, onMounted, onUnmounted } from "vue";
import CpuMemoryCharts from "./CpuMemoryCharts.vue";

  const data = ref<any>(null)
  const loading = ref(true)
  const error = ref<string | null>(null)
  const selectedProcessId = ref<string | null>(null)
  const systemInfo = ref<any>(null) 
  const selectedProcessData = ref<any>(null) 
  let timeoutId: number | undefined = undefined

const firstLoad = ref(true);
const isFetching = ref(false);

async function fetchData() {
  if (isFetching.value) return;
  isFetching.value = true;

  if (firstLoad.value) loading.value = true;
  error.value = null;

  try {
    const res = await fetch(import.meta.env.VITE_API_BASE_URL + "/buffer");
    if (!res.ok) throw new Error(`Erro ${res.status}`);
    data.value = await res.json();
  } catch (err: any) {
    error.value = err.message || "Erro desconhecido";
  } finally {
    if (firstLoad.value) {
      loading.value = false;
      firstLoad.value = false;
    }
    isFetching.value = false;
    timeoutId = window.setTimeout(fetchData, 2000);
  }
}
  async function selectProcess(id: number | string) {
    const idStr = id.toString()

    if (selectedProcessId.value === idStr) {
      selectedProcessId.value = null
      selectedProcessData.value = null
      selectedThreadIndex.value = null
      return
    }

    selectedProcessId.value = idStr
    selectedThreadIndex.value = null
    selectedProcessData.value = null

    try {
      console.log(`Enviando request para /process/${id}...`)
      const res = await fetch(`http://localhost:8000/process/${id}`)
      if (!res.ok) throw new Error(`Erro ao buscar detalhes do processo ${id}`)
      selectedProcessData.value = await res.json()
    } catch (err: any) {
      console.error(err)
      selectedProcessData.value = { error: 'Erro ao carregar detalhes' }
    }
  }

watch(
  data,
  (newData) => {
    if (newData && newData.systemInfo) {
      systemInfo.value = { ...newData.systemInfo };
    }
  },
  { immediate: true }
);


  onMounted(() => {
    fetchData()
  })

  onUnmounted(() => {
    if (timeoutId) clearTimeout(timeoutId)
  })
  const selectedThreadIndex = ref<number | null>(null)

  function selectThread(index: number) {
    selectedThreadIndex.value = selectedThreadIndex.value === index ? null : index
  }

function formatMemory(memoryInKb: number) {
  if (memoryInKb >= 1048576) {
    // 1024 * 1024
    return (memoryInKb / 1048576).toFixed(2) + " GB";
  } else {
    return (memoryInKb / 1024).toFixed(2) + " MB";
}
}
</script>

<template>
  <h1>Dashboard do Sistema</h1>

  <div v-if="loading">Carregando...</div>
  <div v-else-if="error">Erro: {{ error }}</div>
  <div v-else>
    <CpuMemoryCharts v-if="systemInfo" :systemInfo="systemInfo" />

    <h2>Processos</h2>
    <div class="container">
      <!-- Lista à esquerda -->
      <ul class="process-list" v-if="data && data.processes">
        <li 
          v-for="proc in data.processes" 
          :key="proc.id" 
          @click="selectProcess(proc.id)"
          :class="{ selected: selectedProcessId === proc.id.toString() }"
        >
          {{ proc.name }} (PID: {{ proc.id }})
        </li>
      </ul>

      <!-- Detalhes à direita -->
      <div class="process-details" v-if="selectedProcessId !== null">
        <h3>Detalhes do Processo</h3>

        <div v-if="selectedProcessData && !selectedProcessData.error">
          <p><strong>Priority Base:</strong> {{ selectedProcessData.priorityBase }}</p>
          <p><strong>Priority Class:</strong> {{ selectedProcessData.priorityClass }}</p>
          <p><strong>Usuário:</strong> {{ selectedProcessData.userName }}</p>
          <p><strong>Memória Working Set:</strong> {{ formatMemory(selectedProcessData.memoryWorkingSet) }}</p>
          <p><strong>Memória Committed:</strong> {{ formatMemory(selectedProcessData.memoryCommitted) }}</p>
          <p><strong>Memória Privada Committed:</strong> {{ formatMemory(selectedProcessData.privateMemoryCommitted) }}</p>
          <p><strong>Memória Reservada:</strong> {{ formatMemory(selectedProcessData.memoryReserved) }}</p>
          <p><strong>Memória Heap:</strong> {{ formatMemory(selectedProcessData.memoryHeap) }}</p>
          <p><strong>Memória Stack:</strong> {{ formatMemory(selectedProcessData.memoryStack) }}</p>
          <p><strong>Memória Code:</strong> {{ formatMemory(selectedProcessData.memoryCode) }}</p>
          <p><strong>Número de Páginas:</strong> {{ selectedProcessData.numberOfPages }}</p>

          <h4 style="margin-top: 1rem;">Threads: {{ selectedProcessData.threadCount }}</h4>
          <ul class="thread-list">
            <li
              v-for="(thread, index) in selectedProcessData.threads"
              :key="'thread-' + thread.id"
              @click="selectThread(index)"
              :class="{ selected: selectedThreadIndex === index }"
            >
              Thread {{ thread.id }}
            </li>
          </ul>

          <div v-if="selectedThreadIndex !== null">
            <h4>Detalhes da Thread</h4>
            <div v-if="selectedProcessData.threads[selectedThreadIndex]">
              <p><strong>ID:</strong> {{ selectedProcessData.threads[selectedThreadIndex].id }}</p>
              <p><strong>Priority Base:</strong> {{ selectedProcessData.threads[selectedThreadIndex].priorityBase }}</p>
              <p><strong>Priority Delta:</strong> {{ selectedProcessData.threads[selectedThreadIndex].priorityDelta }}</p>
            </div>
          </div>

          <!-- Semáforos -->
          <div v-if="selectedProcessData.semaphores && selectedProcessData.semaphores.length">
            <h4>Semáforos: {{ selectedProcessData.semaphores.length }}</h4>
          </div>

          <!-- Mutexes -->
          <div v-if="selectedProcessData.mutexes && selectedProcessData.mutexes.length">
            <h4>Mutexes: {{ selectedProcessData.mutexes.length }}</h4>
          </div>

          <!-- Arquivos em Disco -->
          <div v-if="selectedProcessData.diskFiles && selectedProcessData.diskFiles.length">
            <h4>Arquivos em Disco: {{ selectedProcessData.diskFiles.length }}</h4>
            <ul class="handle-list">
              <li v-for="(handle, index) in selectedProcessData.diskFiles" :key="'diskFile-' + index">
                <template v-if="handle.name">{{ handle.name }}</template>
              </li>
            </ul>
          </div>

          <!-- Dispositivos de Caracteres -->
          <div v-if="selectedProcessData.charFiles && selectedProcessData.charFiles.length">
            <h4>Dispositivos de Caracteres: {{ selectedProcessData.charFiles.length }}</h4>
            <ul class="handle-list">
              <li v-for="(handle, index) in selectedProcessData.charFiles" :key="'charFile-' + index">
                <template v-if="handle.name">{{ handle.name }}</template>
              </li>
            </ul>
          </div>

          <!-- Pipes -->
          <div v-if="selectedProcessData.pipeFiles && selectedProcessData.pipeFiles.length">
            <h4>Pipes: {{ selectedProcessData.pipeFiles.length }}</h4>
            <ul class="handle-list">
              <li v-for="(handle, index) in selectedProcessData.pipeFiles" :key="'pipeFile-' + index">
                <template v-if="handle.name">{{ handle.name }}</template>
              </li>
            </ul>
          </div>

          <!-- Arquivos Desconhecidos -->
          <div v-if="selectedProcessData.unknownFiles && selectedProcessData.unknownFiles.length">
            <h4>Arquivos Desconhecidos: {{ selectedProcessData.unknownFiles.length }}</h4>
            <ul class="handle-list">
              <li v-for="(handle, index) in selectedProcessData.unknownFiles" :key="'unknownFile-' + index">
                <template v-if="handle.name">{{ handle.name }}</template>
              </li>
            </ul>
          </div>

          <!-- Diretórios -->
          <div v-if="selectedProcessData.directories && selectedProcessData.directories.length">
            <h4>Diretórios: {{ selectedProcessData.directories.length }}</h4>
            <ul class="handle-list">
              <li v-for="(handle, index) in selectedProcessData.directories" :key="'directory-' + index">
                <template v-if="handle.name">{{ handle.name }}</template>
              </li>
            </ul>
          </div>

          <!-- Dispositivos -->
          <div v-if="selectedProcessData.devices && selectedProcessData.devices.length">
            <h4>Dispositivos: {{ selectedProcessData.devices.length }}</h4>
            <ul class="handle-list">
              <li
                v-for="(handle, index) in selectedProcessData.devices"
                :key="'device-' + index"
              >
                <template v-if="handle.name">{{ handle.name }}</template>
              </li>
            </ul>
          </div>
        </div>

        <div v-else>
          <p>Carregando detalhes...</p>
        </div>
      </div>
    </div>
  </div>
</template>

<style scoped>
.container {
  display: flex;
  gap: 2rem;
  align-items: flex-start;
}

.process-list {
  width: 300px;
  max-height: 80vh;
  overflow-y: auto;
  list-style-type: none;
  padding: 0;
  border: 1px solid #ddd;
  border-radius: 4px;
}

.process-list li {
  padding: 0.5rem 1rem;
  cursor: pointer;
}

.process-list li:hover {
  background-color: #4f4fbb;
}

.process-list li.selected {
  font-weight: bold;
  background-color: #1d1d61;
}

.process-details {
  flex: 1;
  max-height: 80vh;
  overflow-y: auto;
  border: 1px solid #ddd;
  border-radius: 4px;
  padding: 1rem;
}
</style>

<style scoped>
ul {
  list-style-type: none;
  padding: 0;
}

li:hover {
  background-color: #4f4fbb;
  cursor: pointer;
}

li.selected {
  font-weight: bold;
  background-color: #080867;
}
</style>
