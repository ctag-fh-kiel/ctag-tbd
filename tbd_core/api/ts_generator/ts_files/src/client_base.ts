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
import {TransportBase} from "./transport";
import {Packet, PacketType, PayloadData} from "./packets";

export type int_par     = number
export type uint_par    = number
export type float_par   = number
export type ufloat_par  = number
export type trigger_par = boolean
export type str_par     = string


type ResponseResolver = (arg: PayloadData) => void
type ErrorDescription = [string, string]


enum BaseEndpoints {
    // recover API
    get_api_version   = 0,
    update_device     = 1,
    reset_device      = 2,

    // core API
    get_num_endpoints = 3,
    get_endpoint_name = 4,
    get_num_events    = 5,
    get_event_name    = 6,
    get_num_errors    = 7,
    get_error_name    = 8,
    get_error_message = 9,

    // base API
    get_device_info   = 10,
}


export class ClientBase {
    protected constructor(transport: TransportBase) {
        this._transport = transport
        this._process_job = this._process_incoming()
    }

    protected async send_rpc(
        endpoint_id: number,
        payload: PayloadData
    ): Promise<PayloadData>
    {
        const request_id = this._request_counter
        this._request_counter += 1

        const packet = new Packet({
            type: PacketType.TYPE_RPC,
            handler: endpoint_id,
            id: request_id,
            payload_length: payload != null ? payload.length : 0,
            payload: payload,
            crc: 0,
        })
        return await new Promise<PayloadData>((res) => {
            this._activeRequests.set(request_id, res)
            this._transport.write(packet)
        })
    }

    async wait_for_shutdown(): Promise<void> {
        await this._process_job
    }

    private async _process_incoming() {
        while (!this._transport.has_closed()) {
            const packet = await this._transport.read()
            const request_id = packet.id
            const resolver = this._activeRequests.get(request_id)
            if (!resolver) {
                console.error(`unknown request ID ${request_id} in response`)
                return
            }
            this._activeRequests.delete(request_id)
            try {
                resolver(packet.payload)
            } catch (e) {
                console.error(`handler for ${packet.handler} failed`)
            }
        }
    }

    protected readonly _transport: TransportBase
    private _request_counter: number = 1
    private readonly _activeRequests: Map<number, ResponseResolver> =  new Map()
    protected _process_job: Promise<any>
}