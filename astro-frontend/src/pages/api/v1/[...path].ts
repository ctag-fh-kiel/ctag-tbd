import type { APIContext } from "astro";

export function getStaticPaths() {
  return [];
}

export async function GET({ url }: APIContext) {
  if (process.env.NODE_ENV === "production") {
    return new Response();
  }

  return fetch(`http://localhost:8080${url.pathname}${url.search}`);
}

export async function POST({ url }: APIContext) {
  if (process.env.NODE_ENV === "production") {
    return new Response();
  }

  return fetch(`http://localhost:8080${url.pathname}${url.search}`, {
    method: "POST",
  });
}
