import Dashboard from "../components/Dashboard.vue";
import { createRouter, createWebHistory } from "vue-router";
import FileExplorer from "../components/FileExplorer.vue";

const routes = [
  {
    path: "/",
    name: "home",
    component: Dashboard,
  },
  {
    path: "/filesystem",
    name: "explorer",
    component: FileExplorer,
  },
];

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes,
});

export default router;
