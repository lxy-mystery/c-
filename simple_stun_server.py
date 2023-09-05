import socket
import binascii
import struct
import sys
def parse_stun_binding_request(data):
    # 解析STUN Binding Request消息的逻辑
    # 获取消息类型、消息长度、事务ID等字段

    # 提取消息类型（2字节）
    message_type = data[0:2]

    # 提取消息长度（2字节）
    message_length = data[2:4]
    # 提取cookie
    message_cookie = data[4:8]

    # 提取事务ID（12字节）
    transaction_id = data[8:20]

    # 返回解析结果
    return transaction_id

def build_response_origin(server_ip, server_port):
    # 将服务器IP地址转换为网络字节序
    server_ip_bytes = socket.inet_pton(socket.AF_INET, server_ip)

    # 构建 RESPONSE-ORIGIN 属性
    attribute_type = b'\x80\x2b'
    attribute_length = b'\x00\x08'
    protocol_famaly = b'\x00\x01'
    response_origin_ip = bytes(server_ip_bytes)
    response_origin_port = server_port.to_bytes(2, 'big')

    response_origin_attr = attribute_type + attribute_length + protocol_famaly + response_origin_port + response_origin_ip

    return response_origin_attr

def build_mapped_address(ip_address, port):
    # 将IP地址转换为网络字节序
    ip_bytes = socket.inet_pton(socket.AF_INET, ip_address)

    # 构建MAPPED-ADDRESS属性
    attribute_type = b'\x00\x01'
    attribute_length = b'\x00\x08'
    protocol_famaly = b'\x00\x01'
    mapped_ip = bytes(ip_bytes)
    mapped_port = port.to_bytes(2, 'big')

    mapped_address_attr = attribute_type + attribute_length + protocol_famaly + mapped_port + mapped_ip

    return mapped_address_attr

def build_xor_mapped_address(ip_address, port):
    # 将IP地址转换为网络字节序
    ip_bytes = socket.inet_pton(socket.AF_INET, ip_address)

    # 特定魔数（Magic Cookie）
    magic_cookie = b'\x21\x12\xa4\x42'

    # 对IP地址进行XOR操作
    xor_mapped_address = bytearray()
    for i in range(len(ip_bytes)):
        xor_mapped_address.append(ip_bytes[i] ^ magic_cookie[i % 4])

    # 构建XOR-MAPPED-ADDRESS属性
    attribute_type = b'\x00\x20'
    attribute_length = b'\x00\x08'
    protocol_famaly = b'\x00\x01'
    xor_mapped_ip = bytes(xor_mapped_address)
    xor_mapped_port = bytes(x ^ y for x, y in zip(port.to_bytes(2, 'big'), magic_cookie[:2]))

    xor_mapped_address_attr = attribute_type + attribute_length + protocol_famaly + xor_mapped_port + xor_mapped_ip

    return xor_mapped_address_attr

def build_software_attribute(software_name):
    # 构建 SOFTWARE 属性
    attribute_type = b'\x80\x22'
    software_bytes = software_name.encode('utf-8')
    attribute_length = len(software_bytes).to_bytes(2, 'big')

    software_attr = attribute_type + attribute_length + software_bytes

    return software_attr

def calculate_fingerprint(message):
    # 计算 CRC32 校验和
    crc32_value = binascii.crc32(message) & 0xffffffff

    # 将 CRC32 值与 Magic Cookie 进行异或运算
    fingerprint_value = crc32_value ^ 0x5354554e

    # 构建 FINGERPRINT 属性
    fingerprint_attribute = struct.pack('!II', 0x80280004, fingerprint_value)

    return fingerprint_attribute


def build_stun_binding_success_response(transaction_id, client_address, server_address):
    # 构建STUN Binding Success Response消息的逻辑
    # 构建消息类型、消息长度、事务ID等字段

    # 构建消息类型为 Binding Success Response（2字节）
    message_type = b'\x01\x01'
    message_cookie = b'\x21\x12\xa4\x42'
    
    # 构建事务ID（12字节）
    response_transaction_id = transaction_id
    xor_mapped_address = build_xor_mapped_address(client_address[0], client_address[1])
    mapped_address = build_mapped_address(client_address[0], client_address[1])
    response_origin = build_response_origin(server_address[0], server_address[1])
    software = build_software_attribute("ldpStun Test")
    attributes = xor_mapped_address + mapped_address + response_origin + software

    message_length = (len(attributes) + 8).to_bytes(2, 'big')
    # 构建STUN Binding Success Response消息
    response_data = message_type + message_length + message_cookie + transaction_id + attributes

    # 返回构建的消息数据
    return response_data + calculate_fingerprint(response_data)

def run_udp_server(external_ip):
    HOST = '0.0.0.0'  # 监听所有网络接口
    PORT = 12345  # 选择一个合适的端口

    # 创建UDP套接字
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as server_socket:
        server_socket.bind((HOST, PORT))
        print('UDP服务器已启动，正在监听端口', PORT)

        while True:
            try:
                # 接收数据
                data, addr = server_socket.recvfrom(1024)
                print('接收到来自', addr, '的数据:', data)

                # 解析STUN Binding Request消息
                transaction_id = parse_stun_binding_request(data)

                # 构建STUN Binding Success Response消息
                response_data = build_stun_binding_success_response(transaction_id, addr, (external_ip, PORT))

                # 发送STUN Binding Success Response消息
                server_socket.sendto(response_data, addr)
            except KeyboardInterrupt:
                break

        print('UDP服务器已关闭')

# 运行UDP服务器
run_udp_server(sys.argv[1])
