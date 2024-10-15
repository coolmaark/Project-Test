import socket
import json
def send_json_data(ip, port, data):
    # Convert data to JSON format
    json_data = json.dumps(data)
    
    # Create a UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    try:
        # Send JSON data over the socket
        sock.sendto(json_data.encode(), (ip, port))
        print(f"Sent JSON data to {ip}:{port}")
    finally:
        sock.close()
if __name__ == "__main__":
    # Data to send
    user_data = {
    "cmd_name": "Test Command",
    "data": "Some datajkandlkcandslkcnalskdcnlskadcnlsakdcnslkdcnlxmclksamclksdnckdncslkdcnslakdcnlkansdclkndaslckmlkmlkmxlaskdmnflkndckjnsdcjksndcjk",
    "range": "0-100",
    "status": "OK",
    "input_other": ["Input 1", "Input 2"],
    "output_other": ["Output 1", "Output 2"]
  }
    # Define the IP and port of the C++ server
    server_ip = "127.0.0.1"  # Replace with the server's IP
    server_port = 12345       # Replace with the correct port
    
    # Send the data
    send_json_data(server_ip, server_port, user_data)