<template>
  <div class="modal-overlay" @click.self="close">
    <div class="modal-content">
      <button class="close-button" @click="close">×</button>
      <h3>Detalhes do Processo</h3>

      <div v-if="data">
        <p><strong>Usuário:</strong> {{ data.userName }}</p>
        <p><strong>Priority Base:</strong> {{ data.priorityBase }}</p>
        <p><strong>Priority Class:</strong> {{ data.priorityClass }}</p>
        <p><strong>Memória Working Set:</strong> {{ format(data.memoryWorkingSet) }}</p>
        <p><strong>Memória Committed:</strong> {{ format(data.memoryCommitted) }}</p>
        <p><strong>Memória Privada Committed:</strong> {{ format(data.privateMemoryCommitted) }}</p>
        <p><strong>Memória Reservada:</strong> {{ format(data.memoryReserved) }}</p>
        <p><strong>Memória Heap:</strong> {{ format(data.memoryHeap) }}</p>
        <p><strong>Memória Stack:</strong> {{ format(data.memoryStack) }}</p>
        <p><strong>Memória Code:</strong> {{ format(data.memoryCode) }}</p>
        <p><strong>Número de Páginas:</strong> {{ data.numberOfPages }}</p>

        <h4>Threads: {{ data.threadCount }}</h4>
        <ul class="thread-list">
          <li
            v-for="(thread, index) in data.threads"
            :key="'thread-' + thread.id"
            @click="selectThread(index)"
            :class="{ selected: selectedThreadIndex === index }"
          >
            Thread {{ thread.id }}
          </li>
        </ul>

        <div v-if="selectedThreadIndex !== null">
          <h4>Detalhes da Thread</h4>
          <div v-if="data.threads[selectedThreadIndex]">
            <p><strong>ID:</strong> {{ data.threads[selectedThreadIndex].id }}</p>
            <p><strong>Priority Base:</strong> {{ data.threads[selectedThreadIndex].priorityBase }}</p>
            <p><strong>Priority Delta:</strong> {{ data.threads[selectedThreadIndex].priorityDelta }}</p>
          </div>
        </div>

        <!-- Semáforos -->
        <div v-if="data.semaphores && data.semaphores.length">
        <h4>Semáforos: {{ data.semaphores.length }}</h4>
        </div>

        <!-- Mutexes -->
        <div v-if="data.mutexes && data.mutexes.length">
        <h4>Mutexes: {{ data.mutexes.length }}</h4>
        </div>

        <!-- Arquivos em Disco -->
        <div v-if="data.diskFiles && data.diskFiles.length">
        <h4>Arquivos em Disco: {{ data.diskFiles.length }}</h4>
        <ul class="handle-list">
            <li v-for="(handle, index) in data.diskFiles" :key="'diskFile-' + index">
            <template v-if="handle.name">{{ handle.name }}</template>
            </li>
        </ul>
        </div>

        <!-- Dispositivos de Caracteres -->
        <div v-if="data.charFiles && data.charFiles.length">
        <h4>Dispositivos de Caracteres: {{ data.charFiles.length }}</h4>
        <ul class="handle-list">
            <li v-for="(handle, index) in data.charFiles" :key="'charFile-' + index">
            <template v-if="handle.name">{{ handle.name }}</template>
            </li>
        </ul>
        </div>

        <!-- Pipes -->
        <div v-if="data.pipeFiles && data.pipeFiles.length">
        <h4>Pipes: {{ data.pipeFiles.length }}</h4>
        <ul class="handle-list">
            <li v-for="(handle, index) in data.pipeFiles" :key="'pipeFile-' + index">
            <template v-if="handle.name">{{ handle.name }}</template>
            </li>
        </ul>
        </div>

        <!-- Arquivos Desconhecidos -->
        <div v-if="data.unknownFiles && data.unknownFiles.length">
        <h4>Arquivos Desconhecidos: {{ data.unknownFiles.length }}</h4>
        <ul class="handle-list">
            <li v-for="(handle, index) in data.unknownFiles" :key="'unknownFile-' + index">
            <template v-if="handle.name">{{ handle.name }}</template>
            </li>
        </ul>
        </div>

        <!-- Diretórios -->
        <div v-if="data.directories && data.directories.length">
        <h4>Diretórios: {{ data.directories.length }}</h4>
        <ul class="handle-list">
            <li v-for="(handle, index) in data.directories" :key="'directory-' + index">
            <template v-if="handle.name">{{ handle.name }}</template>
            </li>
        </ul>
        </div>

        <!-- Dispositivos -->
        <div v-if="data.devices && data.devices.length">
        <h4>Dispositivos: {{ data.devices.length }}</h4>
        <ul class="handle-list">
            <li v-for="(handle, index) in data.devices" :key="'device-' + index">
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
</template>

<script setup lang="ts">
import { ref } from 'vue';

const props = defineProps<{
  data: any;
  selectedThreadIndex: number | null;
}>();
const emit = defineEmits(['close', 'select-thread']);

function close() {
  emit('close');
}

function selectThread(index: number) {
  emit('select-thread', index);
}

function format(memoryInKb: number) {
  return memoryInKb >= 1048576
    ? (memoryInKb / 1048576).toFixed(2) + ' GB'
    : (memoryInKb / 1024).toFixed(2) + ' MB';
}

const categories = [
  { key: 'semaphores', label: 'Semáforos' },
  { key: 'mutexes', label: 'Mutexes' },
  { key: 'diskFiles', label: 'Arquivos em Disco' },
  { key: 'charFiles', label: 'Dispositivos de Caracteres' },
  { key: 'pipeFiles', label: 'Pipes' },
  { key: 'unknownFiles', label: 'Arquivos Desconhecidos' },
  { key: 'directories', label: 'Diretórios' },
  { key: 'devices', label: 'Dispositivos' },
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
  width: 800px;
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

.thread-list li.selected {
  font-weight: bold;
  background-color: #ddd;
}
</style>
