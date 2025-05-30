import type {MessageInitShape} from '@bufbuild/protobuf/dist/esm/types'
import {create, fromBinary, Message, toBinary} from '@bufbuild/protobuf'
import {
    str_par_responseSchema,
    uint_par_requestSchema,
    uint_par_responseSchema,
    void_requestSchema,
    void_responseSchema,
    ApiVersion_responseSchema,
    ApiVersion,
} from "./api_types_pb";
import {GenMessage} from "@bufbuild/protobuf/codegenv2";

export type int_par     = number
export type uint_par    = number
export type float_par   = number
export type ufloat_par  = number
export type trigger_par = boolean
export type str_par     = string

type ResponseResolver = (data: Uint8Array) => void
type ErrorDescription = [string, string]


enum BaseEndpoints {
    // core API
    get_api_version   = 0,
    update_device     = 1,
    reset_device      = 2,

    // base API
    get_num_endpoints = 3,
    get_endpoint_name = 4,
    get_device_info   = 5,
    get_num_errors    = 6,
    get_error_name    = 7,
    get_error_message = 8,
}


export class ClientBase {
    constructor() {
        this._request_counter = 1
        let has_connected: () => void
        this._connection_established = new Promise((res) => {
            has_connected = res
        })

        const socket = new WebSocket('/ws')
        socket.onopen = () => has_connected()
        socket.onmessage = (event) => this._handle_data(event)
        socket.onclose = (event) => this._handle_disconnect(event)
        socket.onerror = (event) => this._handle_socket_error(event)
        this._websocket = socket
        this._activeRequests = new Map()
        this._errors = null
    }

    async wait_for_connection(): Promise<void> {
        return await this._connection_established
    }

    async send<InType extends Message, OutType extends Message>(
        in_type: GenMessage<InType>,
        request: MessageInitShape<GenMessage<InType>>,
        out_type: GenMessage<OutType>,
    ): Promise<OutType>
    {
        const request_id = this._request_counter
        this._request_counter += 1
        const req_message = create(in_type, {requestId: request_id, ...request})

        const req_data = toBinary(in_type, req_message)
        const res_data = await new Promise<Uint8Array>((res) => {
            this._activeRequests.set(request_id, res)
            this._websocket.send(req_data)
        })
        const res_message = fromBinary(out_type, res_data)
        if (res_message.status != 0) {
            throw `request ${request_id} failed: ${res_message.status}`
        }
        return res_message
    }

    async get_version(): Promise<ApiVersion> {
        const {output: api_version} = await this.send(void_requestSchema, {endpoint: BaseEndpoints.get_api_version}, ApiVersion_responseSchema)
        if (api_version == undefined) {
            throw Error('failed to determine API version')
        }
        return api_version
    }

    private async _handle_data(event: MessageEvent) {
        if (!(event.data instanceof Blob)) {
            console.error('received non binary message', event.data)
            return
        }
        const res_data = new Uint8Array(await event.data.arrayBuffer())
        const res_header = fromBinary(void_responseSchema, res_data)
        const request_id = res_header.requestId
        const resolver = this._activeRequests.get(request_id)
        if (!resolver) {
            console.error(`unknown request ID ${request_id} in response`)
            return
        }
        this._activeRequests.delete(request_id)
        resolver(res_data)
    }

    private _handle_disconnect(event: CloseEvent) {
        console.error('lost websocket', event)
    }

    private _handle_socket_error(event: Event) {
        console.error('websocket error', event)
    }

    private _request_counter: number
    private _activeRequests: Map<number, ResponseResolver>
    private _websocket: WebSocket
    private _connection_established: Promise<void>
    private _errors: Array<ErrorDescription> | null
}