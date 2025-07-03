<template>
  <div class="file-explorer-container">
    <div class="header">
      <h1>Explorer</h1>
    </div>
    <div class="header">
      <h1 style="margin-bottom: 2rem">Discos</h1>

      <DiskDisplay
        v-for="disk in diskData"
        :key="disk.guid || disk.serialNumber"
        :disk="disk"
        @disk-clicked="handleDiskClick"
      />
    </div>
    <div class="header">
      <h1>{{ currentDirectory }}</h1>
    </div>
    <div class="table-wrapper">
      <table class="file-table">
        <thead>
          <tr>
            <!-- Column headers with click handlers for sorting -->
            <th @click="sortBy('fileName')">
              Name
              <span class="sort-arrow">{{ getSortArrow("fileName") }}</span>
            </th>
            <th @click="sortBy('type')">
              Type <span class="sort-arrow">{{ getSortArrow("type") }}</span>
            </th>
            <th @click="sortBy('fileSize')">
              Size
              <span class="sort-arrow">{{ getSortArrow("fileSize") }}</span>
            </th>
            <th @click="sortBy('lastWriteTime')">
              Last Modified
              <span class="sort-arrow">{{
                getSortArrow("lastWriteTime")
              }}</span>
            </th>
          </tr>
        </thead>
        <tbody>
          <!-- Handle case where there are no files -->
          <tr v-if="sortedFiles.length === 0">
            <td colspan="4" class="empty-state">This directory is empty.</td>
          </tr>
          <!-- Loop through and display each file/directory -->
          <tr
            v-for="item in sortedFiles"
            :key="item.id"
            @click="handleItemClick(item)"
            class="file-row"
          >
            <td>
              <span class="icon">{{
                item.type === "directory" ? "📁" : "📄"
              }}</span>
              <span class="name">{{ item.fileName }}</span>
            </td>
            <td>{{ item.type }}</td>
            <td>
              {{ item.type === "file" ? formatSize(item.fileSize) : "—" }}
            </td>
            <td>{{ formatDate(item.lastWriteTime) }}</td>
          </tr>
        </tbody>
      </table>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, type Ref, onMounted } from "vue";
import DiskDisplay from "./DiskDisplay.vue";

// --- TYPE DEFINITION ---
// Defines the structure for a file or directory item.
interface FileSystemItem {
  id: number;
  fileName: string;
  type: "file" | "directory" | undefined;
  fileSize: number; // Size in bytes
  lastWriteTime: Date;
  creationTime: Date;
  lastAccessTime: Date;
}

interface DiskInfoItem {
  guid: string;
  path: string;
  volumeName: string;
  freeMemory: number;
  serialNumber: number;
  usedMemory: number;
  totalMemory: number;
}

// --- STATE MANAGEMENT ---
// A reactive reference to hold the list of files and directories.
// This is mock data, but in a real application, you would fetch this from an API.
let temp: FileSystemItem[];
let volsTemp: DiskInfoItem[];
let files = ref<FileSystemItem[]>([]);
let currentDirectory = "";

onMounted(async () => {
  const res = await fetch(
    import.meta.env.VITE_API_BASE_URL +
      `/search?term=${encodeURIComponent(currentDirectory)}`
  );
  temp = await res.json();
  let index: number = 0;
  temp.forEach((x) => {
    x.lastWriteTime = new Date(x.lastWriteTime);
    x.creationTime = new Date(x.creationTime);
    x.lastAccessTime = new Date(x.lastAccessTime);
    x.type = x.fileName.includes("/") ? "directory" : "file";
    x.id = index++;
  });
  files.value = temp;
  console.log(files);
  const vols = await fetch(import.meta.env.VITE_API_BASE_URL + "/volumes");
  volsTemp = await vols.json();
  diskData.value = volsTemp;
});

// --- SORTING LOGIC ---
// Reactive references to track the current sort key and direction.
type SortKey = keyof FileSystemItem;
const sortKey = ref<SortKey>("fileName");
const sortDirection = ref<"asc" | "desc">("asc");

// A computed property that automatically sorts the files whenever the
// sort key or direction changes. Directories are always listed before files.
const sortedFiles = computed(() => {
  return [...files.value].sort((a, b) => {
    // Primary sort: directories before files
    if (a.type === "directory" && b.type === "file") return -1;
    if (a.type === "file" && b.type === "directory") return 1;
    if (a[sortKey.value] == undefined || b[sortKey.value] == undefined)
      return 0;

    // Secondary sort: based on the selected column (sortKey)
    let comparison = 0;

    if (a[sortKey.value] > b[sortKey.value]) {
      comparison = 1;
    } else if (a[sortKey.value] < b[sortKey.value]) {
      comparison = -1;
    }

    return sortDirection.value === "asc" ? comparison : -comparison;
  });
});

/**
 * Changes the sort key or reverses the direction if the key is the same.
 * @param key - The key of the FileSystemItem to sort by.
 */
function sortBy(key: SortKey) {
  if (sortKey.value === key) {
    sortDirection.value = sortDirection.value === "asc" ? "desc" : "asc";
  } else {
    sortKey.value = key;
    sortDirection.value = "asc";
  }
}

/**
 * Returns a sort arrow indicator for the UI.
 * @param key - The key of the column to check.
 */
function getSortArrow(key: SortKey) {
  if (sortKey.value !== key) return "";
  return sortDirection.value === "asc" ? "▲" : "▼";
}

// --- HELPER FUNCTIONS ---
/**
 * Formats a file size from bytes into a human-readable string (KB, MB, etc.).
 * @param bytes - The size of the file in bytes.
 */
function formatSize(bytes: number): string {
  if (bytes === 0) return "0 B";
  const k = 1024;
  const sizes = ["B", "KB", "MB", "GB", "TB"];
  const i = Math.floor(Math.log(bytes) / Math.log(k));
  return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + " " + sizes[i];
}

/**
 * Formats a Date object into a locale-specific date and time string.
 * @param date - The date to format.
 */
function formatDate(date: Date): string {
  return date.toLocaleString();
}

// --- EVENT HANDLERS ---
/**
 * Handles clicks on a table row.
 * In a real app, this would navigate into a directory or open a file.
 * @param item - The file or directory that was clicked.
 */
async function handleItemClick(item: FileSystemItem) {
  if (item.type === "directory") {
    // In a real application, you would update the `files` ref
    // with the contents of the clicked directory.
    // For example: files.value = await fetchDirectoryContents(item.id);
    let newDirectory = currentDirectory;
    if (item.fileName == "./") return;
    if (item.fileName == "../") {
      let i = currentDirectory.length - 2;
      while (currentDirectory[i] != "/") {
        i--;
      }
      newDirectory = currentDirectory.substring(0, i + 1);
    } else {
      newDirectory = currentDirectory + item.fileName;
    }
    console.log(`Navigating into directory: ${newDirectory}`);
    try {
      const res = await fetch(
        import.meta.env.VITE_API_BASE_URL +
          `/search?term=${encodeURIComponent(newDirectory)}`
      );
      temp = await res.json();
    } catch (e) {
      return;
    }
    currentDirectory = newDirectory;
    let index: number = 0;
    temp.forEach((x) => {
      x.lastWriteTime = new Date(x.lastWriteTime);
      x.creationTime = new Date(x.creationTime);
      x.lastAccessTime = new Date(x.lastAccessTime);
      x.type = x.fileName.includes("/") ? "directory" : "file";
      x.id = index++;
    });
    files.value = temp;
  } else {
    console.log(`Opening file: ${item.fileName}`);
  }
}

let diskData = ref<DiskInfoItem[]>([]);

async function handleDiskClick(path: string) {
  currentDirectory = path.replace("\\", "/");
  try {
    const res = await fetch(
      import.meta.env.VITE_API_BASE_URL +
        `/search?term=${encodeURIComponent(currentDirectory)}`
    );
    temp = await res.json();
  } catch (e) {
    return;
  }
  currentDirectory = currentDirectory;
  let index: number = 0;
  temp.forEach((x) => {
    x.lastWriteTime = new Date(x.lastWriteTime);
    x.creationTime = new Date(x.creationTime);
    x.lastAccessTime = new Date(x.lastAccessTime);
    x.type = x.fileName.includes("/") ? "directory" : "file";
    x.id = index++;
  });
  files.value = temp;
}
</script>

<style scoped>
/* Scoped styles ensure these CSS rules only apply to this component */

.file-explorer-container {
  font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica,
    Arial, sans-serif;
  background-color: #1e1e1e;
  color: #d4d4d4;
  padding: 2rem;
  border-radius: 12px;
  max-width: 1000px;
  margin: 2rem auto;
  box-shadow: 0 10px 30px rgba(0, 0, 0, 0.3);
}

.header {
  margin-bottom: 1.5rem;
  padding-bottom: 1rem;
  border-bottom: 1px solid #333;
}

.header h1 {
  margin: 0;
  font-size: 1.8rem;
  color: #ce9178; /* A nice accent color */
}

.header p {
  margin: 0.25rem 0 0;
  color: #888;
}

.table-wrapper {
  overflow-x: auto; /* Ensures table is responsive on small screens */
}

.file-table {
  width: 100%;
  border-collapse: collapse;
  text-align: left;
}

.file-table th {
  padding: 12px 15px;
  font-weight: 600;
  color: #9cdcfe;
  cursor: pointer;
  user-select: none; /* Prevents text selection on click */
  transition: background-color 0.2s ease;
  white-space: nowrap;
}

.file-table th:hover {
  background-color: #2a2d2e;
}

.sort-arrow {
  display: inline-block;
  margin-left: 8px;
  font-size: 0.8em;
  color: #ce9178;
}

.file-table td {
  padding: 12px 15px;
  border-top: 1px solid #333;
  vertical-align: middle;
}

.file-row {
  transition: background-color 0.2s ease;
  cursor: pointer;
}

.file-row:hover {
  background-color: #2a2d2e;
}

.icon {
  margin-right: 12px;
  font-size: 1.2em;
}

.name {
  font-weight: 500;
}

.empty-state {
  text-align: center;
  color: #888;
  padding: 40px;
  font-style: italic;
}
#app-container {
  padding: 20px;
  max-width: 600px;
  margin: 40px auto;
  background-color: #fff;
  border-radius: 10px;
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1);
}

h1 {
  font-family: "Segoe UI", sans-serif;
  color: #222;
  border-bottom: 2px solid #f0f0f0;
  padding-bottom: 10px;
  margin-bottom: 20px;
}
</style>
