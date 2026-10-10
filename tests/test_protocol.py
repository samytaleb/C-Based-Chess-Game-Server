import socket
import subprocess
import time

PORT = 27000
server = subprocess.Popen(['./server', str(PORT)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

def readline(sock):
    result = bytearray()
    while True:
        b = sock.recv(1)
        if not b:
            raise AssertionError('unexpected server disconnect')
        if b == b'\n':
            return result.decode('ascii')
        result.extend(b)
        if len(result) > 256:
            raise AssertionError('oversized line')

def expect(sock, prefix):
    line = readline(sock)
    assert line.startswith(prefix), (line, prefix)
    return line

try:
    time.sleep(.2)
    white = socket.create_connection(('127.0.0.1', PORT), timeout=3)
    white.settimeout(3)
    expect(white, 'INFO Waiting')
    black = socket.create_connection(('127.0.0.1', PORT), timeout=3)
    black.settimeout(3)
    expect(white, 'HELLO WHITE')
    expect(black, 'HELLO BLACK')
    expect(white, 'BOARD ')
    expect(black, 'BOARD ')
    expect(white, 'YOUR_TURN')
    white.sendall(b'MOVE e7e5\n')
    expect(white, 'ERROR ')
    expect(white, 'YOUR_TURN')
    for sock, move in [(white, b'f2f3'), (black, b'e7e5'), (white, b'g2g4'), (black, b'd8h4')]:
        sock.sendall(b'MOVE '+move+b'\n')
        expect(white, 'BOARD ')
        expect(black, 'BOARD ')
        if move == b'd8h4':
            break
        turn = white if sock is black else black
        expect(turn, 'YOUR_TURN')
    expect(white, 'GAMEOVER CHECKMATE BLACK_WINS')
    expect(black, 'GAMEOVER CHECKMATE BLACK_WINS')
    white.close()
    black.close()
    print('Network protocol tests OK')
finally:
    server.terminate()
    try:
        server.communicate(timeout=2)
    except subprocess.TimeoutExpired:
        server.kill()
        server.communicate()
