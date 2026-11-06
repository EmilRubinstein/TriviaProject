import socket
import json
import struct

# ── codes (must match server CODES enum) ──────────────────────────
LOGIN                    = 1
SIGNUP                   = 2
SIGNOUT                  = 3
GET_ROOMS                = 4
GET_PLAYERS_IN_ROOM      = 5
GET_STATISTICS           = 6
GET_HIGH_SCORE           = 7
JOIN_ROOM                = 8
CREATE_ROOM              = 9

S_PORT = 1024
E_PORT = 65535

# ── connection ────────────────────────────────────────────────────
def get_host():
    host = input("Enter server host (default 127.0.0.1): ").strip()
    return host if host else "127.0.0.1"

def get_port():
    while True:
        try:
            port = int(input("Enter server port (1024-65535): "))
            if S_PORT <= port <= E_PORT:
                return port
            print("[Error] invalid port range")
        except ValueError:
            print("[Error] enter numeric port")

# ── packet helpers ────────────────────────────────────────────────
def build_packet(code, data={}):
    j = json.dumps(data).encode('utf-8')
    packet = bytearray()
    packet.append(code)
    packet += struct.pack("<I", len(j))
    packet += j
    return packet

def recv_exact(sock, n):
    data = b""
    while len(data) < n:
        chunk = sock.recv(n - len(data))
        if not chunk:
            raise ConnectionError("server disconnected")
        data += chunk
    return data

def read_response(sock):
    code = recv_exact(sock, 1)[0]
    size = struct.unpack("<I", recv_exact(sock, 4))[0]
    body = recv_exact(sock, size)
    return code, json.loads(body.decode("utf-8"))

def send_and_recv(sock, code, data={}):
    sock.sendall(build_packet(code, data))
    return read_response(sock)

# ── auth screens ──────────────────────────────────────────────────
def do_login(sock):
    print("Press 'd' for default or any key for custom")
    if input("> ") == "d":
        data = {"username": "user1", "password": "1234"}
    else:
        data = {
            "username": input("username: "),
            "password": input("password: ")
        }
    code, msg = send_and_recv(sock, LOGIN, data)
    print(f"[Server]: {msg}")
    return msg.get("status") == 1, data["username"]

def do_signup(sock):
    print("Press 'd' for default or any key for custom")
    if input("> ") == "d":
        data = {"username": "user1", "password": "1234", "email": "user1@gmail.com"}
    else:
        data = {
            "username": input("username: "),
            "password": input("password: "),
            "email":    input("email: ")
        }
    code, msg = send_and_recv(sock, SIGNUP, data)
    print(f"[Server]: {msg}")
    return msg.get("status") == 1, data["username"]

# ── menu screens ──────────────────────────────────────────────────
def do_get_rooms(sock):
    code, msg = send_and_recv(sock, GET_ROOMS)
    rooms = msg.get("Rooms", [])
    if rooms:
        print("\n── Rooms ──")
        for i, r in enumerate(rooms):
            print(f"  {i+1}. {r}")
    else:
        print("[No rooms available]")
    return rooms

def do_get_players(sock):
    room_id = int(input("Enter room ID: "))
    code, msg = send_and_recv(sock, GET_PLAYERS_IN_ROOM, {"roomId": room_id})
    players = msg.get("PlayersInRoom", [])
    print(f"\n── Players in room {room_id} ──")
    for p in players:
        print(f"  {p}")

def do_get_statistics(sock):
    code, msg = send_and_recv(sock, GET_STATISTICS)
    stats = msg.get("UserStatistics", [])
    print("\n── Your Statistics ──")
    for s in stats:
        print(f"  {s}")

def do_get_highscore(sock):
    code, msg = send_and_recv(sock, GET_HIGH_SCORE)
    scores = msg.get("HighScores", [])
    print("\n── High Scores ──")
    for s in scores:
        print(f"  {s}")

def do_join_room(sock):
    room_id = int(input("Enter room ID to join: "))
    code, msg = send_and_recv(sock, JOIN_ROOM, {"roomId": room_id})
    print(f"[Server]: {msg}")
    return msg.get("status") == 1

def do_create_room(sock):
    print("Press 'd' for default or any key for custom")
    if input("> ") == "d":
        data = {"roomName": "MyRoom", "maxPlayers": 4, "timePerQuestion": 30}
    else:
        data = {
            "roomName":        input("Room name: "),
            "maxPlayers":      int(input("Max players: ")),
            "timePerQuestion": int(input("Time per question (seconds): "))
        }
    code, msg = send_and_recv(sock, CREATE_ROOM, data)
    print(f"[Server]: {msg}")
    return msg.get("status") == 1

def do_signout(sock):
    code, msg = send_and_recv(sock, SIGNOUT)
    print(f"[Server]: {msg}")
    return msg.get("status") == 1

# ── menus ─────────────────────────────────────────────────────────
def auth_menu(sock):
    while True:
        print("\n── Auth Menu ──")
        print("0. Exit")
        print("1. Login")
        print("2. Signup")
        try:
            choice = int(input("> "))
        except ValueError:
            continue

        if choice == 0:
            return False, None
        elif choice == 1:
            ok, username = do_login(sock)
            if ok:
                return True, username
        elif choice == 2:
            ok, username = do_signup(sock)
            if ok:
                return True, username

def main_menu(sock, username):
    while True:
        print(f"\n── Main Menu ({username}) ──")
        print("0. Exit")
        print("1. Get Rooms")
        print("2. Get Players In Room")
        print("3. My Statistics")
        print("4. High Scores")
        print("5. Join Room")
        print("6. Create Room")
        print("7. Signout")
        try:
            choice = int(input("> "))
        except ValueError:
            continue

        if choice == 0:
            return False   # exit program
        elif choice == 1:
            do_get_rooms(sock)
        elif choice == 2:
            do_get_players(sock)
        elif choice == 3:
            do_get_statistics(sock)
        elif choice == 4:
            do_get_highscore(sock)
        elif choice == 5:
            do_join_room(sock)
        elif choice == 6:
            do_create_room(sock)
        elif choice == 7:
            ok = do_signout(sock)
            if ok:
                return True  # back to auth menu

# ── main ──────────────────────────────────────────────────────────
def main():
    host = get_host()
    port = get_port()

    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((host, port))
        print(f"[Client] connected to {host}:{port}")
    except Exception as e:
        print("[Error]", e)
        return

    try:
        while True:
            # auth phase
            logged_in, username = auth_menu(sock)
            if not logged_in:
                break

            # menu phase
            back_to_auth = main_menu(sock, username)
            if not back_to_auth:
                break

    except Exception as e:
        print("[Error]", e)
    finally:
        sock.close()
        print("[Client] connection closed")

if __name__ == "__main__":
    main()