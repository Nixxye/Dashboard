<script setup lang="ts">
  import { ref, watch, onMounted, onUnmounted } from 'vue'
  import CpuMemoryCharts from './CpuMemoryCharts.vue'

  const data = ref<any>(null)
  const loading = ref(true)
  const error = ref<string | null>(null)
  const selectedProcessId = ref<string | null>(null)
  const systemInfo = ref<any>(null)  
  let timeoutId: number | undefined = undefined

  const firstLoad = ref(true)
  const isFetching = ref(false) 

  async function fetchData() {
    if (isFetching.value) return
    isFetching.value = true

    if (firstLoad.value) loading.value = true
    error.value = null

    try {
      const res = await fetch('http://localhost:8000/buffer')
      if (!res.ok) throw new Error(`Erro ${res.status}`)
      data.value = await res.json()
    } catch (err: any) {
      error.value = err.message || 'Erro desconhecido'
    } finally {
      if (firstLoad.value) {
        loading.value = false
        firstLoad.value = false
      }
      isFetching.value = false
      timeoutId = window.setTimeout(fetchData, 2000)
    }
  }

  watch(data, (newData) => {
    if (newData && newData.systemInfo) {
      systemInfo.value = { ...newData.systemInfo }
    }
  }, { immediate: true })

  function selectProcess(id: number | string) {
    const idStr = id.toString()
    selectedProcessId.value = selectedProcessId.value === idStr ? null : idStr
  }

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
      <ul class="process-list">
        <li 
          v-for="proc in data.processes" 
          :key="proc.id" 
          @click="selectProcess(proc.id)"
          :class="{ selected: selectedProcessId === proc.id.toString() }"
        >
          {{ proc.name }} (PID: {{ proc.id }})
        </li>
      </ul>

    <div class="process-details" v-if="selectedProcessId !== null">
      <h3>Detalhes do Processo</h3>
      <div 
        v-for="proc in data.processes" 
        :key="'details-' + proc.id" 
        v-show="proc.id.toString() === selectedProcessId"
      >
      <p><strong>Priority Base:</strong> {{ proc.priorityBase }}</p>
      <p><strong>Priority Class:</strong> {{ proc.priorityClass }}</p>
      <p><strong>Usuário:</strong> {{ proc.userName }}</p>
      <p><strong>Memória Working Set:</strong> {{ (proc.memoryWorkingSet / 1024).toFixed(2) }} MB</p>
      <p><strong>Memória Committed:</strong> {{ (proc.memoryCommitted / 1024).toFixed(2) }} MB</p>
      <p><strong>Memória Privada Committed:</strong> {{ (proc.privateMemoryCommitted / 1024).toFixed(2) }} MB</p>
      <p><strong>Memória Reservada:</strong> {{ (proc.memoryReserved / 1024).toFixed(2) }} MB</p>
      <p><strong>Memória Heap:</strong> {{ (proc.memoryHeap / 1024).toFixed(2) }} MB</p>
      <p><strong>Memória Stack:</strong> {{ (proc.memoryStack / 1024).toFixed(2) }} MB</p>
      <p><strong>Memória Code:</strong> {{ (proc.memoryCode / 1024).toFixed(2) }} MB</p>
      <p><strong>Número de Páginas:</strong> {{ proc.numberOfPages }}</p>
        <h4 style="margin-top: 1rem;">Threads: {{ proc.threadCount }} </h4>
        <ul class="thread-list">
          <li
            v-for="(thread, index) in proc.threads"
            :key="'thread-' + thread.id"
            @click="selectThread(index)"
            :class="{ selected: selectedThreadIndex === index }"
          >
            Thread {{ thread.id }}
          </li>
        </ul>

        <div v-if="selectedThreadIndex !== null">
          <h4>Detalhes da Thread</h4>
          <div v-if="proc.threads[selectedThreadIndex]">
            <p><strong>ID:</strong> {{ proc.threads[selectedThreadIndex].id }}</p>
            <p><strong>Priority Base:</strong> {{ proc.threads[selectedThreadIndex].priorityBase }}</p>
            <p><strong>Priority Delta:</strong> {{ proc.threads[selectedThreadIndex].priorityDelta }}</p>
          </div>
        </div>
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
  background-color: #eef;
}

.process-list li.selected {
  font-weight: bold;
  background-color: #cce5ff;
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
  background-color: #eef;
}
</style>
