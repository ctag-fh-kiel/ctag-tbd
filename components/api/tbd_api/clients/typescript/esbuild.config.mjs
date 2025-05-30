import dotenv from 'dotenv'
import esbuild from 'esbuild'
import { copy } from 'esbuild-plugin-copy'
import process from 'node:process'
import Fastify from 'fastify'
import proxy from '@fastify/http-proxy'

dotenv.config();
const args = process.argv;
const vars = process.env

const BUILD_TYPE = args.NODE_ENV || 'production'
const TBD_HOST = vars.TBD_HOST || 'localhost'
const TBD_PORT_STR = vars.TBD_PORT || '7777'
const TBD_PORT = parseInt(TBD_PORT_STR)
const TBD_URL = `${TBD_HOST}:${TBD_PORT}/ws`


const config = {
  logLevel: 'info',
  entryPoints: ['src/app.tsx'],
  outfile: 'public/main.js',
  bundle: true,
  define: {
    NODE_ENV: JSON.stringify(BUILD_TYPE),
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
    const {hosts: webapp_hosts, port: webapp_port} = await ctx.serve({
      servedir: 'public',
      onRequest: ({remoteAddress, method, path, status, timeInMS}) => {
        console.info(remoteAddress, status, `"${method} ${path}" [${timeInMS}ms]`)
      },
    })
    const webapp_url = `http://${webapp_hosts[0]}:${webapp_port}/`

    const server = Fastify()
    server.register(proxy, {
      upstream: webapp_url,
    })
    server.register(proxy, {
      websocket: true,
      prefix: '/ws',
      upstream: `http://${TBD_URL}`,
      wsUpstream: `ws://${TBD_URL}`,
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
    console.log(`forwarding tbd requests to ${TBD_URL}`)
    await server.listen({port: 3000})
  } catch (e) {
    console.error(e)
    process.exit(1)
  }
}