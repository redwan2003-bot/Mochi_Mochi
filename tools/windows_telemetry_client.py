#!/usr/bin/env python3
"""
Mochi Bot V1 - Windows Telemetry Companion Script
Streams CPU, RAM, and GPU statistics over local Wi-Fi to Mochi Bot.

Endpoints:
- POST /api/v1/telemetry : JSON payload with cpu_usage, ram_usage, gpu_usage, cpu_temp
- POST /api/v1/notify    : Send transient desktop notification popup to Mochi
"""

import sys
import time
import json
import argparse
import urllib.request
import urllib.error

# Attempt to import psutil if installed, otherwise use fallback performance query
try:
    import psutil
    HAS_PSUTIL = True
except ImportError:
    HAS_PSUTIL = False
    import ctypes

def get_system_stats():
    """Queries system CPU and RAM usage."""
    if HAS_PSUTIL:
        cpu = int(psutil.cpu_percent(interval=1.0))
        ram = int(psutil.virtual_memory().percent)
        # Attempt to get CPU temperature if exposed by hardware
        cpu_temp = 0
        try:
            temps = psutil.sensors_temperatures()
            if temps:
                for name, entries in temps.items():
                    if entries:
                        cpu_temp = int(entries[0].current)
                        break
        except Exception:
            cpu_temp = 45 # Default fallback nominal temperature
        
        # GPU usage placeholder (requires vendor library like pynvml / GPUtil)
        gpu = 15
        return cpu, ram, gpu, cpu_temp
    else:
        # Minimal fallback using ctypes MemoryStatusEx
        class MEMORYSTATUSEX(ctypes.Structure):
            _fields_ = [
                ("dwLength", ctypes.c_ulong),
                ("dwMemoryLoad", ctypes.c_ulong),
                ("ullTotalPhys", ctypes.c_ulonglong),
                ("ullAvailPhys", ctypes.c_ulonglong),
                ("ullTotalPageFile", ctypes.c_ulonglong),
                ("ullAvailPageFile", ctypes.c_ulonglong),
                ("ullTotalVirtual", ctypes.c_ulonglong),
                ("ullAvailVirtual", ctypes.c_ulonglong),
                ("sullAvailExtendedVirtual", ctypes.c_ulonglong),
            ]
        stat = MEMORYSTATUSEX()
        stat.dwLength = ctypes.sizeof(MEMORYSTATUSEX)
        ctypes.windll.kernel32.GlobalMemoryStatusEx(ctypes.byref(stat))
        ram = int(stat.dwMemoryLoad)
        cpu = 20 # Nominal placeholder if psutil is not installed
        return cpu, ram, 10, 48

def send_telemetry(host, token, cpu, ram, gpu, temp):
    url = f"http://{host}/api/v1/telemetry"
    payload = {
        "cpu_usage": cpu,
        "ram_usage": ram,
        "gpu_usage": gpu,
        "cpu_temp": temp
    }
    data = json.dumps(payload).encode('utf-8')
    headers = {
        "Content-Type": "application/json",
        "Authorization": f"Bearer {token}"
    }

    req = urllib.request.Request(url, data=data, headers=headers, method="POST")
    try:
        with urllib.request.urlopen(req, timeout=3.0) as resp:
            return resp.status == 200
    except Exception as e:
        print(f"[!] Telemetry send failed: {e}")
        return False

def send_notification(host, token, sender, message, duration_ms=5000):
    url = f"http://{host}/api/v1/notify"
    payload = {
        "sender": sender,
        "msg": message,
        "dur": duration_ms
    }
    data = json.dumps(payload).encode('utf-8')
    headers = {
        "Content-Type": "application/json",
        "Authorization": f"Bearer {token}"
    }

    req = urllib.request.Request(url, data=data, headers=headers, method="POST")
    try:
        with urllib.request.urlopen(req, timeout=3.0) as resp:
            print(f"[+] Notification delivered to {host}!")
            return True
    except Exception as e:
        print(f"[!] Notification failed: {e}")
        return False

def main():
    parser = argparse.ArgumentParser(description="Mochi Bot Windows Telemetry Companion")
    parser.add_argument("--host", default="192.168.1.100", help="Mochi Bot IP address (shown on OLED card 4 or serial)")
    parser.add_argument("--token", default="mochi_secret_token", help="Bearer Auth token configured in Mochi settings")
    parser.add_argument("--interval", type=float, default=3.0, help="Reporting interval in seconds (default: 3.0)")
    parser.add_argument("--notify", nargs=2, metavar=("SENDER", "MESSAGE"), help="Send a test notification and exit")

    args = parser.parse_args()

    if args.notify:
        send_notification(args.host, args.token, args.notify[0], args.notify[1])
        return

    print("==================================================")
    print("  MOCHI BOT V1 - WINDOWS TELEMETRY CLIENT")
    print(f"  Target Mochi Bot: http://{args.host}/")
    print(f"  Update Interval : {args.interval}s")
    print("  Press Ctrl+C to stop streaming.")
    print("==================================================")

    if not HAS_PSUTIL:
        print("[*] Note: 'psutil' not found. For accurate CPU metrics, install via:")
        print("    pip install psutil\n")

    while True:
        try:
            cpu, ram, gpu, temp = get_system_stats()
            success = send_telemetry(args.host, args.token, cpu, ram, gpu, temp)
            status_str = "OK" if success else "TIMEOUT / ERROR"
            print(f"[STREAM] CPU: {cpu:2d}% | RAM: {ram:2d}% | GPU: {gpu:2d}% | Temp: {temp:2d}C -> {status_str}")
            time.sleep(args.interval)
        except KeyboardInterrupt:
            print("\nStopping telemetry stream. Mochi Bot will mark PC offline after 15s.")
            break

if __name__ == "__main__":
    main()
