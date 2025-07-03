<template>
  <div class="modal-overlay" @click.self="close">
    <div class="modal-content">
      <button class="close-button" @click="close">×</button>
      <h1> {{internalData.name}}</h1>

      <div v-if="internalData">
        <p><strong>Usuário:</strong> {{ internalData.userName }}</p>
        <p><strong>Priority Base:</strong> {{ internalData.priorityBase }}</p>
        <p><strong>Priority Class:</strong> {{ internalData.priorityClass }}</p>
        <p><strong>Memória Working Set:</strong> {{ format(internalData.memoryWorkingSet) }}</p>
        <p><strong>Memória Committed:</strong> {{ format(internalData.memoryCommitted) }}</p>
        <p><strong>Memória Privada Committed:</strong> {{ format(internalData.privateMemoryCommitted) }}</p>
        <p><strong>Memória Reservada:</strong> {{ format(internalData.memoryReserved) }}</p>
        <p><strong>Memória Heap:</strong> {{ format(internalData.memoryHeap) }}</p>
        <p><strong>Memória Stack:</strong> {{ format(internalData.memoryStack) }}</p>
        <p><strong>Memória Code:</strong> {{ format(internalData.memoryCode) }}</p>
        <p><strong>Número de Páginas:</strong> {{ internalData.numberOfPages }}</p>

        <h4>Threads: {{ internalData.threadCount }}</h4>
        <ul class="thread-list">
          <li
            v-for="(thread, index) in internalData.threads"
            :key="'thread-' + thread.id"
            @click="selectThread(index)"
            :class="{ selected: selectedThreadIndex === index }"
          >
            Thread {{ thread.id }}
          </li>
        </ul>

        <!-- Semáforos -->
        <div v-if="internalData.semaphores && internalData.semaphores.length">
          <h4>Semáforos: {{ internalData.semaphores.length }}</h4>
        </div>

        <!-- Mutexes -->
        <div v-if="internalData.mutexes && internalData.mutexes.length">
          <h4>Mutexes: {{ internalData.mutexes.length }}</h4>
        </div>

        <!-- Arquivos em Disco -->
        <div v-if="internalData.diskFiles && internalData.diskFiles.length">
          <h4>Arquivos em Disco: {{ internalData.diskFiles.length }}</h4>
          <ul class="handle-list">
            <li v-for="(handle, index) in internalData.diskFiles" :key="'diskFile-' + index">
              <template v-if="handle.name">{{ handle.name }}</template>
            </li>
          </ul>
        </div>

        <!-- Dispositivos de Caracteres -->
        <div v-if="internalData.charFiles && internalData.charFiles.length">
          <h4>Dispositivos de Caracteres: {{ internalData.charFiles.length }}</h4>
          <ul class="handle-list">
            <li v-for="(handle, index) in internalData.charFiles" :key="'charFile-' + index">
              <template v-if="handle.name">{{ handle.name }}</template>
            </li>
          </ul>
        </div>

        <!-- Pipes -->
        <div v-if="internalData.pipeFiles && internalData.pipeFiles.length">
          <h4>Pipes: {{ internalData.pipeFiles.length }}</h4>
          <ul class="handle-list">
            <li v-for="(handle, index) in internalData.pipeFiles" :key="'pipeFile-' + index">
              <template v-if="handle.name">{{ handle.name }}</template>
            </li>
          </ul>
        </div>

        <!-- Arquivos Desconhecidos -->
        <div v-if="internalData.unknownFiles && internalData.unknownFiles.length">
          <h4>Arquivos Desconhecidos: {{ internalData.unknownFiles.length }}</h4>
          <ul class="handle-list">
            <li v-for="(handle, index) in internalData.unknownFiles" :key="'unknownFile-' + index">
              <template v-if="handle.name">{{ handle.name }}</template>
            </li>
          </ul>
        </div>

        <!-- Diretórios -->
        <div v-if="internalData.directories && internalData.directories.length">
          <h4>Diretórios: {{ internalData.directories.length }}</h4>
          <ul class="handle-list">
            <li v-for="(handle, index) in internalData.directories" :key="'directory-' + index">
              <template v-if="handle.name">{{ handle.name }}</template>
            </li>
          </ul>
        </div>

        <!-- Dispositivos -->
        <div v-if="internalData.devices && internalData.devices.length">
          <h4>Dispositivos: {{ internalData.devices.length }}</h4>
          <ul class="handle-list">
            <li v-for="(handle, index) in internalData.devices" :key="'device-' + index">
              <template v-if="handle.name">{{ handle.name }}</template>
            </li>
          </ul>
        </div>
      </div>

      <div v-else>
        <p>Carregando detalhes...</p>
      </div>
    </div>

    <!-- Popup Thread -->
    <div
      v-if="showThreadPopup"
      class="thread-popup-overlay"
      @click.self="closeThreadPopup"
    >
      <div class="thread-popup-content">
        <button class="close-button" @click="closeThreadPopup">×</button>
        <h4>Thread {{ selectedThreadData?.id }}</h4>
        <p><strong>Priority Base:</strong> {{ selectedThreadData?.priorityBase }}</p>
        <p><strong>Priority Delta:</strong> {{ selectedThreadData?.priorityDelta }}</p>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted, watch } from "vue";

const props = defineProps<{
  data: any;
  selectedThreadIndex: number | null;
  processId: string;
}>();
const emit = defineEmits(["close", "select-thread"]);

const showThreadPopup = ref(false);
const selectedThreadData = ref<any>(null);
const internalData = ref<any>(props.data); // cópia local
let intervalId: number | null = null;

function fetchProcessData() {
  fetch(`http://localhost:8000/process/${props.processId}`)
    .then(res => res.json())
    .then(json => {
      internalData.value = json;
    })
    .catch(err => {
      console.error("Erro ao atualizar processo:", err);
    });
}

onMounted(() => {
//   intervalId = setInterval(fetchProcessData, 3000);
});

onUnmounted(() => {
  if (intervalId) clearInterval(intervalId);
});

function close() {
  emit("close");
}

function selectThread(index: number) {
  emit("select-thread", index);
  selectedThreadData.value = props.data.threads[index];
  showThreadPopup.value = true;
}

function closeThreadPopup() {
  showThreadPopup.value = false;
}

function format(memoryInKb: number) {
  return memoryInKb >= 1048576
    ? (memoryInKb / 1048576).toFixed(2) + " GB"
    : (memoryInKb / 1024).toFixed(2) + " MB";
}

const categories = [
  { key: "semaphores", label: "Semáforos" },
  { key: "mutexes", label: "Mutexes" },
  { key: "diskFiles", label: "Arquivos em Disco" },
  { key: "charFiles", label: "Dispositivos de Caracteres" },
  { key: "pipeFiles", label: "Pipes" },
  { key: "unknownFiles", label: "Arquivos Desconhecidos" },
  { key: "directories", label: "Diretórios" },
  { key: "devices", label: "Dispositivos" },
];
</script>

<style scoped>
.modal-overlay {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(0, 0, 0, 0.5);
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 1000;
}

.modal-content {
  background: rgb(26, 25, 25);
  padding: 2rem;
  border-radius: 8px;
  max-height: 90vh;
  overflow-y: auto;
  width: 60vw;
  position: relative;
}

.close-button {
  position: absolute;
  top: 0.5rem;
  right: 0.5rem;
  border: none;
  background: transparent;
  font-size: 1.2rem;
  cursor: pointer;
}

.thread-list li:hover {
  background-color: #5555aa;
  cursor: pointer;
}


ul.handle-list,
ul.thread-list {
  list-style-type: none;
  padding-left: 0;
  text-align: center;
  margin: 0 auto;
  width: fit-content;
}

/* Popup thread */

.thread-popup-overlay {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(0, 0, 0, 0.6);
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 1100;
}

.thread-popup-content {
  background: #2c2c2c;
  padding: 1.5rem;
  border-radius: 8px;
  width: 300px;
  color: white;
  position: relative;
  box-shadow: 0 0 20px rgba(0, 0, 0, 0.8);
}
</style>
