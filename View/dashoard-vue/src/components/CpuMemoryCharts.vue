<script setup lang="ts">
import { defineProps, watch, ref } from 'vue'
import { Bar, Line } from 'vue-chartjs'
import {
  Chart, BarElement, LineElement, PointElement, CategoryScale, LinearScale, Tooltip, Legend,
} from 'chart.js'

Chart.register(BarElement, LineElement, PointElement, CategoryScale, LinearScale, Tooltip, Legend)

const props = defineProps<{
  systemInfo: any
}>()

const cpuData = ref({
  labels: [] as string[],
  datasets: [
    {
      label: 'Uso CPU (%) por Núcleo',
      backgroundColor: '#42b983',
      data: [] as number[],
    }
  ]
})

const memoryData = ref({
  labels: ['Usada', 'Livre'],
  datasets: [
    {
      label: 'Memória (MB)',
      backgroundColor: ['#f87979', '#7acbf9'],
      data: [] as number[],
    }
  ]
})

// Gráfico de linha para uso do CPU principal ao longo do tempo
const cpuUsageTimeline = ref({
  labels: [] as string[],
  datasets: [
    {
      label: 'Uso CPU Principal (%)',
      borderColor: '#ff6384',
      backgroundColor: '#ff6384',
      fill: false,
      data: [] as number[],
      tension: 0.3,
    }
  ]
})

const MAX_POINTS = 10

const chartOptions = {
  responsive: true,
  plugins: { legend: { position: 'top' } },
  scales: {
    y: { min: 0, max: 100 },
    x: {
      ticks: { display: false },      // Esconde os labels do eixo X
      grid: { drawTicks: false }       // Esconde os ticks do eixo X
    }
  }
}

watch(() => props.systemInfo, (newVal) => {
  if (!newVal) return

  // Atualiza gráfico de CPU por núcleo (imutável)
  const perCpuLabels = Object.keys(newVal.perCpuUsage)
  const perCpuData = Object.values(newVal.perCpuUsage).map((v: number) =>
    Number((v).toFixed(2))
  )

  cpuData.value = {
    labels: perCpuLabels,
    datasets: [{
      ...cpuData.value.datasets[0],
      data: perCpuData
    }]
  }

  // Atualiza gráfico de memória (imutável)
  const usedMB = newVal.usedMemory
  const totalMB = usedMB / (newVal.memoryUsage / 100)
  const freeMB = Math.max(totalMB - usedMB, 0)

  memoryData.value = {
    ...memoryData.value,
    datasets: [{
      ...memoryData.value.datasets[0],
      data: [usedMB, freeMB]
    }]
  }

  // Atualiza gráfico de linha (imutável)
  // Mantém as labels porque são necessárias, mas elas não aparecem visualmente
  const timeLabel = new Date().toLocaleTimeString()
  const newLabels = [...cpuUsageTimeline.value.labels, timeLabel].slice(-MAX_POINTS)
  const newData = [...cpuUsageTimeline.value.datasets[0].data, Number(newVal.cpuUsage.toFixed(2))].slice(-MAX_POINTS)

  cpuUsageTimeline.value = {
    labels: newLabels,
    datasets: [{
      ...cpuUsageTimeline.value.datasets[0],
      data: newData
    }]
  }

}, { immediate: true })
</script>

<template>
  <div style="max-width: 600px;">
    <h3>Uso da CPU por Núcleo</h3>
    <Bar :data="cpuData" :options="{ responsive: true, plugins: { legend: { position: 'top' } } }" />

    <h3 style="margin-top: 2rem;">Uso de Memória</h3>
    <Bar :data="memoryData" :options="{ responsive: true, plugins: { legend: { position: 'top' } } }" />

    <h3 style="margin-top: 2rem;">Uso do Processador Principal (Tempo Real)</h3>
    <Line :data="cpuUsageTimeline" :options="chartOptions" />
  </div>
</template>
