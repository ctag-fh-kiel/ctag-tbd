import dotenv from 'dotenv'
import esbuild from 'esbuild'
import { copy } from 'esbuild-plugin-copy'
import process from 'node:process'
import Fastify from 'fastify'
import proxy from '@fastify/http-proxy'
import http from 'node:http'
import urlParse from 'url-parse'

dotenv.config();
const args = process.argv;

const config = {
  logLevel: 'info',
  entryPoints: ['src/app.tsx'],
  outfile: 'public/main.js',
  bundle: true,
  define: {
    NODE_ENV: JSON.stringify(process.env.NODE_ENV || 'production'),
  },
  plugins: [
    copy({
      resolveFrom: 'cwd',
      assets: {
        from: ['./src/index.html'],
        to: ['./public/index.html'],
      },
      watch: true,
    })
  ]
}

if (args.includes('--build')) {
  try {
    await esbuild.build({
      ...config,
      minify: true,
      sourcemap: false,
    })
  } catch(e) {
    console.error(e)
    process.exit(1)
  }
}

if (args.includes('--start')) {
  try {
    const ctx = await esbuild.context({
      ...config,
      minify: false,
      sourcemap: true,
    })
    await ctx.watch()
    const {hosts, port} = await ctx.serve({
      servedir: 'public',
      onRequest: ({remoteAddress, method, path, status, timeInMS}) => {
        console.info(remoteAddress, status, `"${method} ${path}" [${timeInMS}ms]`)
      },
    })

    // const proxy = httpProxy.createProxyServer({ws: true})
    // proxy.on('upgrade', (req, socket, head) => {
    //     socket.on('error', (error) => {
    //        console.error(error);
    //    });
    //
    //   console.log('[WS] Redirecting (upgrade)...', req.url);
    //   proxy.ws(req, socket, head, {target: 'ws://localhost:7777/ws'});
    // });
    // const server = http.createServer((req, res) => {
    //   const { query } = urlParse(req.url, true);
    //   const redirectParams = { target: { host: query.csHost, port: parseInt(query.csPort, 10) } };
    //   console.log('[HTTP] Redirecting...', req.url);
    //   if (req.url.startsWith('/ws')) {
    //     proxy.ws(req, res, {target: 'ws://localhost:7777/ws'})
    //   }
    //   proxy.web(req, res, {target: 'http://localhost:8000'})
    // })
    // console.log('listening on :3000')
    // server.listen(3000)

    const server = Fastify()
    server.register(proxy, {
      upstream: 'http://localhost:8000/',
    })
    server.register(proxy, {
      websocket: true,
      prefix: '/ws',
      upstream: 'http://localhost:7777/ws',
      wsUpstream: 'ws://localhost:7777/ws',
      // wsHooks: {
      //   onDisconnect: () => {
      //     console.log('onDisconnect')
      //   },
      //   onPong: () => {
      //     console.log('onPong')
      //   },
      //   onReconnect: () => {
      //     console.log('reconnect')
      //   },
      //   onIncomingMessage: (ctx, source, target, {data, binary}) => {
      //     console.log('incomming', data, binary)
      //   }
      // },
      wsReconnect: {
        logs: true,
        pingInterval: 3_000,
        reconnectOnClose: true,
      }
    })
    await server.listen({port: 3000})
  } catch (e) {
    console.error(e)
    process.exit(1)
  }
}