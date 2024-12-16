// @ts-check
import { defineConfig } from "astro/config";

import tailwind from "@astrojs/tailwind";

import preact from "@astrojs/preact";

// https://astro.build/config
export default defineConfig({
  output: process.env.NODE_ENV === "production" ? "static" : "server",
  outDir: "../spiffs_image/www",
  integrations: [tailwind(), preact({ compat: true })],
});
