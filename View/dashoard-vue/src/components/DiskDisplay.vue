<template>
  <div class="disk-container" @click="onDiskClick">
    <div class="disk-icon">
      <svg
        xmlns="http://www.w3.org/2000/svg"
        width="48"
        height="48"
        viewBox="0 0 24 24"
        fill="none"
        stroke="currentColor"
        stroke-width="1.5"
        stroke-linecap="round"
        stroke-linejoin="round"
        class="feather feather-hard-drive"
      >
        <line x1="22" y1="12" x2="2" y2="12"></line>
        <path
          d="M5.45 5.11L2 12v6a2 2 0 0 0 2 2h16a2 2 0 0 0 2-2v-6l-3.45-6.89A2 2 0 0 0 16.76 4H7.24a2 2 0 0 0-1.79 1.11z"
        ></path>
        <line x1="6" y1="16" x2="6.01" y2="16"></line>
        <line x1="10" y1="16" x2="10.01" y2="16"></line>
      </svg>
    </div>

    <div class="disk-info">
      <div class="disk-header">
        <span class="disk-name">{{ processedDisk.displayName }}</span>
        <span class="disk-usage-percent"
          >{{ processedDisk.usedPercentage.toFixed(1) }}% usado</span
        >
      </div>

      <div class="progress-bar">
        <div
          class="progress-fill"
          :style="{ width: processedDisk.usedPercentage + '%' }"
        ></div>
      </div>

      <div class="disk-details">
        <span
          >{{ processedDisk.formattedFree }} livres de
          {{ processedDisk.formattedTotal }}</span
        >
      </div>
    </div>
  </div>
</template>

<script setup>
import { computed } from "vue";

const emit = defineEmits(["disk-clicked"]);

// Definindo as props que o componente espera receber
const props = defineProps({
  disk: {
    type: Object,
    required: true,
  },
});

/**
 * Formata bytes em um formato legível (KB, MB, GB, TB).
 * @param {number} bytes - O número de bytes a ser formatado.
 * @returns {string} - A string formatada.
 */
function formatBytes(bytes, decimals = 2) {
  if (bytes === 0) return "0 Bytes";
  const k = 1024;
  const dm = decimals < 0 ? 0 : decimals;
  const sizes = ["Bytes", "KB", "MB", "GB", "TB"];
  const i = Math.floor(Math.log(bytes) / Math.log(k));
  return parseFloat((bytes / Math.pow(k, i)).toFixed(dm)) + " " + sizes[i];
}

function onDiskClick() {
  // O primeiro argumento é o nome do evento.
  // O segundo (e seguintes) são os dados que queremos enviar para o pai.
  emit("disk-clicked", props.disk.path);
}

// Uma propriedade computada para pré-calcular e formatar os dados do disco.
// Isso é mais eficiente do que fazer os cálculos diretamente no template.
const processedDisk = computed(() => {
  const { path, volumeName, freeMemory, usedMemory, totalMemory } = props.disk;

  // Define o nome de exibição como "Nome (C:)" ou "Disco Local (C:)" se o nome não existir.
  const driveLetter = path.substring(0, 2);
  const displayName = volumeName
    ? `${volumeName} (${driveLetter})`
    : `Disco Local (${driveLetter})`;

  // Calcula a porcentagem de uso
  const usedPercentage = totalMemory > 0 ? (usedMemory / totalMemory) * 100 : 0;

  return {
    displayName,
    usedPercentage,
    formattedFree: formatBytes(freeMemory),
    formattedUsed: formatBytes(usedMemory),
    formattedTotal: formatBytes(totalMemory),
  };
});
</script>

<style scoped>
.disk-container {
  display: flex;
  align-items: center;
  font-family: "Segoe UI", Tahoma, Geneva, Verdana, sans-serif;
  padding: 16px;
  background-color: #2a2d2e; /* Fundo cinza bem claro */
  border: 1px solid #343a40; /* Borda sutil */
  border-radius: 12px; /* Cantos mais arredondados */
  margin-bottom: 1.25rem;
  box-shadow: none; /* Remove a sombra antiga */
}

/* Ícone do disco */
.disk-icon {
  margin-right: 16px;
  color: #a4c1dd; /* Cor do ícone */
  flex-shrink: 0;
}

.disk-info {
  flex-grow: 1;
  min-width: 0;
}

.disk-header {
  display: flex;
  justify-content: space-between;
  align-items: baseline;
  margin-bottom: 8px;
}

/* Nome do disco, ex: "Disco Local (C:)" */
.disk-name {
  font-weight: 600;
  font-size: 1.1rem;
  color: #b6b6b6; /* Cor principal do texto */
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

/* Porcentagem de uso */
.disk-usage-percent {
  font-size: 0.9rem;
  font-weight: 500;
  color: #dee2e6;
  padding-left: 8px;
  white-space: nowrap;
}

/* Barra de progresso (trilha) */
.progress-bar {
  width: 100%;
  height: 14px; /* Um pouco mais alta */
  background-color: #dee2e6; /* Cor da trilha */
  border-radius: 7px;
  overflow: hidden;
  border: none;
}

/* Preenchimento da barra de progresso */
.progress-fill {
  height: 100%;
  background-color: #0d6efd; /* Azul vibrante (padrão Bootstrap) */
  border-radius: 7px;
  transition: width 0.5s ease-in-out;
}

/* Detalhes de memória (19.09 GB livres...) */
.disk-details {
  margin-top: 8px;
  font-size: 0.9rem;
  color: #dbdbdb; /* Cor secundária (cinza) */
}
</style>
