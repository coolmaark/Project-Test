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
    data = [
    
        {
            "rows": [
                        ["cmd_name", "Cell A2"],
                        ["data", "hellosdkadofkef"],
                        ["range", "Lorem ipsum, dolor sit amet consectetur adipisicing elit. Unde, consectetur? Iste unde qui veniam earum explicabo omnis aliquid error nesciunt quae itaque hic, sapiente incidunt, neque inventore similique at doloribus?"],
                        ["status", "Pass"]
                    ]
        }
    ]
    # Define the IP and port of the C++ server
    server_ip = "127.0.0.1"  # Replace with the server's IP
    server_port = 12345       # Replace with the correct port
    
    # Send the data
    send_json_data(server_ip, server_port, data)