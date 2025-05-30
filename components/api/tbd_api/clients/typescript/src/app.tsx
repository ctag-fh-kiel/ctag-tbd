import * as React from 'react'
import { createRoot } from 'react-dom/client'
import {connect_tbd, Client} from './client'
import {useState, useContext, createContext} from "react";


const ApiContext = createContext<Client>(null)

function Info() {
    const [endpoints, setEndpoints] = useState<Array<string>>([])
    const api = useContext(ApiContext)

    const update_endpoints = async () => {
        const new_endpoint_list = []
        const num_endpoints = await api.get_num_endpoints()
        for (let i = 0; i < num_endpoints; ++i) {
            new_endpoint_list.push(await api.get_endpoint_name(i))
        }
        setEndpoints(new_endpoint_list)
    }

    const request_list = <div>{endpoints.map((item) => <div key={item}>{item}</div>)}</div>

    return <div>
        <div><button type="button" onClick={update_endpoints}>update</button></div>
        {request_list}
    </div>
}



function TbdApp({api}: {api: Client}) {
    return <ApiContext.Provider value={api}><Info/></ApiContext.Provider>
}

connect_tbd().then((client: Client) => {
    window.tbd_api = client
    const domNode = document.getElementById('root');
    if (domNode != null) {
        const root = createRoot(domNode);
        root.render(<TbdApp api={client}/>);
    }
}).catch((err) => {
        const domNode = document.getElementById('root');
    if (domNode != null) {
        const root = createRoot(domNode);
        root.render(<div>failed to connect to TBD {err}</div>);
    }
})


