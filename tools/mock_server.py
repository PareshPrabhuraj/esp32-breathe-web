#!/usr/bin/env python3
"""
Local Host & Hardware Bridge for ESP32 LED Breather Web UI
Serves web/index.html and bridges button presses to the physical ESP32-S3
over /dev/cu.usbmodem101, allowing instant live control from localhost:8080!
"""

import http.server
import json
import os
import socketserver
import sys
import time

# Auto-detect serial module from PlatformIO virtualenv if not in system python
try:
    import serial
except ImportError:
    penv_site = os.path.expanduser("~/.platformio/penv/lib/python3.14/site-packages")
    if os.path.exists(penv_site):
        sys.path.insert(0, penv_site)
        import serial

DEFAULT_PORT = 8080
state = {"color": "green"}

BASE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
HTML_PATH = os.path.join(BASE_DIR, "web", "index.html")

# Hardware Serial connection
ser = None
SERIAL_PORT = "/dev/cu.usbmodem101"

def init_serial():
    global ser
    try:
        if os.path.exists(SERIAL_PORT):
            ser = serial.Serial(SERIAL_PORT, 115200, timeout=0.3)
            ser.dtr = True
            ser.rts = True
            time.sleep(0.1)
            print(f"[hardware] Successfully linked to ESP32-S3 on {SERIAL_PORT}!")
    except Exception as e:
        ser = None
        print(f"[hardware] Serial note: {e}")

def send_hardware_cmd(cmd):
    global ser, state
    if ser is None or not ser.is_open:
        init_serial()
    if ser and ser.is_open:
        try:
            ser.write(f"{cmd}\n".encode("utf-8"))
            ser.flush()
            print(f"[hardware] Sent command to ESP32: {cmd}")
        except Exception as e:
            print(f"[hardware] Write error: {e}")
            ser = None

init_serial()

class MockBreatherHandler(http.server.BaseHTTPRequestHandler):
    def end_headers(self):
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")
        super().end_headers()

    def do_OPTIONS(self):
        self.send_response(204)
        self.end_headers()

    def do_GET(self):
        global state
        if self.path == "/" or self.path.startswith("/?"):
            if not os.path.exists(HTML_PATH):
                self.send_response(404)
                self.end_headers()
                self.wfile.write(b"web/index.html not found")
                return
            with open(HTML_PATH, "rb") as f:
                content = f.read()
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(content)))
            self.end_headers()
            self.wfile.write(content)
        elif self.path == "/api/state":
            body = json.dumps(state).encode("utf-8")
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
        else:
            self.send_response(404)
            self.end_headers()
            self.wfile.write(b"404 Not Found")

    def do_POST(self):
        global state
        content_length = int(self.headers.get("Content-Length", 0))
        raw_body = self.rfile.read(content_length).decode("utf-8") if content_length > 0 else ""

        if self.path == "/api/toggle":
            new_color = "red" if state["color"] == "green" else "green"
            state["color"] = new_color
            send_hardware_cmd("TOGGLE")
            print(f"[web] Toggled state -> {new_color.upper()} {'🟢' if new_color == 'green' else '🔴'}")
            body = json.dumps(state).encode("utf-8")
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
        elif self.path == "/api/color":
            target_color = ""
            try:
                data = json.loads(raw_body)
                target_color = data.get("color", "")
            except Exception:
                if "red" in raw_body.lower():
                    target_color = "red"
                elif "green" in raw_body.lower():
                    target_color = "green"

            if target_color in ("green", "red"):
                state["color"] = target_color
                send_hardware_cmd(target_color.upper())
                print(f"[web] Set color -> {target_color.upper()} {'🟢' if target_color == 'green' else '🔴'}")
                body = json.dumps(state).encode("utf-8")
                self.send_response(200)
                self.send_header("Content-Type", "application/json")
                self.send_header("Content-Length", str(len(body)))
                self.end_headers()
                self.wfile.write(body)
            else:
                body = json.dumps({"error": "Invalid color"}).encode("utf-8")
                self.send_response(400)
                self.send_header("Content-Type", "application/json")
                self.send_header("Content-Length", str(len(body)))
                self.end_headers()
                self.wfile.write(body)
        else:
            self.send_response(404)
            self.end_headers()
            self.wfile.write(b"404 Not Found")

    def log_message(self, format, *args):
        sys.stderr.write(f"[server] {args[0]} {args[1]} -> {args[2]}\n")

def run(port=DEFAULT_PORT):
    class ReuseAddrServer(socketserver.TCPServer):
        allow_reuse_address = True

    try:
        with ReuseAddrServer(("", port), MockBreatherHandler) as httpd:
            print("==================================================")
            print("  ESP32 LED BREATHER LOCAL HOST & HARDWARE BRIDGE ")
            print("==================================================")
            print(f"Serving web/index.html at:")
            print(f"  👉 http://localhost:{port}/")
            print("Physical hardware link:")
            print(f"  Target Port: {SERIAL_PORT}")
            print("==================================================")
            httpd.serve_forever()
    except OSError as e:
        if "Address already in use" in str(e):
            print(f"Port {port} in use, trying {port + 1}...")
            run(port + 1)
        else:
            raise

if __name__ == "__main__":
    port = int(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_PORT
    run(port)
