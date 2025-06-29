<script setup lang="ts">
import { ref, onMounted } from 'vue'

const data = ref<any>(null)
const loading = ref(true)
const error = ref<string | null>(null)

async function fetchData() {
  loading.value = true
  error.value = null
  try {
    const res = await fetch('http://localhost:8000/buffer')
    if (!res.ok) throw new Error(`Erro ${res.status}`)
    data.value = await res.json()
  } catch (err: any) {
    error.value = err.message || 'Erro desconhecido'
  } finally {
    loading.value = false
  }
}

onMounted(fetchData)
</script>

<template>
  <h1>Dashboard do Sistema</h1>

  <div v-if="loading">Carregando...</div>
  <div v-else-if="error">Erro: {{ error }}</div>
  <div v-else>
    <h2>Sistema</h2>
    <ul>
      <li><strong>CPU Count:</strong> {{ data.systemInfo.cpuCount }}</li>
      <li><strong>CPU Usage:</strong> {{ data.systemInfo.cpuUsage.toFixed(2) }}%</li>
      <li><strong>Idle Time:</strong> {{ data.systemInfo.idleTime.toFixed(2) }}%</li>
      <li><strong>Used Memory:</strong> {{ (data.systemInfo.usedMemory / 1024 / 1024).toFixed(2) }} MB</li>
      <li><strong>Memory Usage:</strong> {{ data.systemInfo.memoryUsage.toFixed(2) }}%</li>
      <li><strong>Disk Usage:</strong> {{ data.systemInfo.diskUsage.toFixed(2) }}%</li>
    </ul>

    <h2>Top 5 Processos</h2>
    <table border="1" cellpadding="5">
      <thead>
        <tr>
          <th>PID</th>
          <th>Nome</th>
          <th>Threads</th>
          <th>Memória (MB)</th>
        </tr>
      </thead>
      <tbody>
        <tr v-for="proc in data.processes.slice(0, 5)" :key="proc.id">
          <td>{{ proc.id }}</td>
          <td>{{ proc.name }}</td>
          <td>{{ proc.threadCount }}</td>
          <td>{{ (proc.memoryWorkingSet / 1024 / 1024).toFixed(2) }}</td>
        </tr>
      </tbody>
    </table>
  </div>
</template>

<style scoped>
table {
  margin-top: 1rem;
  border-collapse: collapse;
}
th {
  background-color: #f0f0f0;
}
td,
th {
  padding: 0.5rem;
}
</style>
