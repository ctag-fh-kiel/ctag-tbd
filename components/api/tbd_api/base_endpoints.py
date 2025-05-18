BASE_ENDPOINTS = [
    'get_num_endpoints',
    'get_endpoint_name',
    'get_device_info',
]

BASE_ENDPOINT_IDS = {
    endpoint_name: request_id for request_id, endpoint_name in enumerate(BASE_ENDPOINTS)
}
