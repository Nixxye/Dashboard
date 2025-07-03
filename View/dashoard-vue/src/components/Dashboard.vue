<script setup lang="ts">
import { ref, watch, onMounted, onUnmounted } from "vue";
import CpuMemoryCharts from "./CpuMemoryCharts.vue";
import ProcessModal from "./ProcessModal.vue";


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

      <ProcessModal
        v-if="selectedProcessData"
        :data="selectedProcessData"
        :selectedThreadIndex="selectedThreadIndex"
        @close="selectedProcessData = null"
        @select-thread="selectThread"
      />
    </div>
  </div>
</template>

<style scoped>
.container {
  display: flex;
  justify-content: center; /* centraliza horizontalmente */
  align-items: flex-start;
  gap: 2rem;
  flex-wrap: wrap; /* permite que quebre se faltar espaço */
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
