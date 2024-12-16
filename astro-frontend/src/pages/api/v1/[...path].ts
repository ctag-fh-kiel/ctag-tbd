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

export async function POST({ url, request }: APIContext) {
  if (process.env.NODE_ENV === "production") {
    return new Response();
  }

  const requestInit: RequestInit = {
    method: "POST",
    headers: request.headers,
  };

  if (request.body !== null) {
    requestInit["body"] = await request.text();
  }

  return fetch(
    `http://localhost:8080${url.pathname}${url.search}`,
    requestInit,
  );
}
