#!/usr/bin/env python3
"""
Mochi Bot V1 - Windows Desktop Companion & Tray Application
===========================================================
Desktop companion with graphical Tkinter configuration interface and Windows
system tray integration. Streams live CPU, RAM, GPU, and temperature metrics
over Wi-Fi to Mochi Bot's OLED display, and provides instant notification alerts.

Features:
- Built-in Tkinter GUI with zero heavy Qt dependencies.
- Native ctypes Windows API fallback when psutil is absent.
- System tray icon via pystray (with seamless Tkinter minimization fallback).
- Auto-persisting configuration in mochi_pc_config.json.
- Live connection state machine (ONLINE / RECONNECTING / STALE / OFFLINE).
- Quick notification popup sender for Discord, Outlook, Steam, etc.
- Clean shutdown on Ctrl+C or window close without hanging background threads.
"""

import sys
import os
import time
import json
import signal
import shutil
import argparse
import threading
import subprocess
import urllib.request
import urllib.error
from dataclasses import dataclass, asdict

# ---------------------------------------------------------------------------
# Hardware & Environment Detection
# ---------------------------------------------------------------------------
# 1. psutil detection with ctypes fallback
try:
    import psutil
    HAS_PSUTIL = True
except ImportError:
    HAS_PSUTIL = False

# 2. Windows ctypes performance APIs
import ctypes
from ctypes import wintypes

class FILETIME(ctypes.Structure):
    _fields_ = [
        ("dwLowDateTime", wintypes.DWORD),
        ("dwHighDateTime", wintypes.DWORD)
    ]

class MEMORYSTATUSEX(ctypes.Structure):
    _fields_ = [
        ("dwLength", wintypes.DWORD),
        ("dwMemoryLoad", wintypes.DWORD),
        ("ullTotalPhys", ctypes.c_ulonglong),
        ("ullAvailPhys", ctypes.c_ulonglong),
        ("ullTotalPageFile", ctypes.c_ulonglong),
        ("ullAvailPageFile", ctypes.c_ulonglong),
        ("ullTotalVirtual", ctypes.c_ulonglong),
        ("ullAvailVirtual", ctypes.c_ulonglong),
        ("sullAvailExtendedVirtual", ctypes.c_ulonglong),
    ]

# 3. System Tray & Pillow detection
try:
    import pystray
    from PIL import Image, ImageDraw
    HAS_PYSTRAY = True
except ImportError:
    HAS_PYSTRAY = False

# 4. Tkinter GUI
import tkinter as tk
from tkinter import ttk, messagebox

# ---------------------------------------------------------------------------
# Configuration Model & Persistence
# ---------------------------------------------------------------------------
DEFAULT_CONFIG_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), "mochi_pc_config.json")

@dataclass
class CompanionConfig:
    host: str = "192.168.1.100"
    token: str = "mochi_secret_token"
    interval: float = 3.0
    send_cpu: bool = True
    send_ram: bool = True
    send_gpu: bool = True
    send_temp: bool = True
    minimize_to_tray_on_close: bool = True
    auto_start_stream: bool = True

    @classmethod
    def load(cls, filepath: str = DEFAULT_CONFIG_PATH) -> "CompanionConfig":
        if os.path.isfile(filepath):
            try:
                with open(filepath, "r", encoding="utf-8") as f:
                    data = json.load(f)
                    return cls(
                        host=str(data.get("host", "192.168.1.100")),
                        token=str(data.get("token", "mochi_secret_token")),
                        interval=float(data.get("interval", 3.0)),
                        send_cpu=bool(data.get("send_cpu", True)),
                        send_ram=bool(data.get("send_ram", True)),
                        send_gpu=bool(data.get("send_gpu", True)),
                        send_temp=bool(data.get("send_temp", True)),
                        minimize_to_tray_on_close=bool(data.get("minimize_to_tray_on_close", True)),
                        auto_start_stream=bool(data.get("auto_start_stream", True)),
                    )
            except Exception as e:
                print(f"[!] Warning: Failed loading config from {filepath}: {e}")
        return cls()

    def save(self, filepath: str = DEFAULT_CONFIG_PATH):
        try:
            with open(filepath, "w", encoding="utf-8") as f:
                json.dump(asdict(self), f, indent=2)
        except Exception as e:
            print(f"[!] Warning: Failed saving config to {filepath}: {e}")

# ---------------------------------------------------------------------------
# System Metrics Collector (psutil + native ctypes fallback)
# ---------------------------------------------------------------------------
class SystemMetricsCollector:
    """Gathers CPU %, RAM %, GPU %, and Temperature with ctypes fallback."""

    def __init__(self):
        self._prev_idle = 0
        self._prev_kernel = 0
        self._prev_user = 0
        self._has_nvidia_smi = shutil.which("nvidia-smi") is not None
        self._cached_gpu = 15
        self._cached_temp = 42
        self._last_gpu_query_time = 0.0

        if not HAS_PSUTIL and os.name == "nt":
            self._init_ctypes_cpu()

    def _filetime_to_int(self, ft: FILETIME) -> int:
        return (ft.dwHighDateTime << 32) + ft.dwLowDateTime

    def _init_ctypes_cpu(self):
        try:
            idle = FILETIME()
            kernel = FILETIME()
            user = FILETIME()
            if ctypes.windll.kernel32.GetSystemTimes(ctypes.byref(idle), ctypes.byref(kernel), ctypes.byref(user)):
                self._prev_idle = self._filetime_to_int(idle)
                self._prev_kernel = self._filetime_to_int(kernel)
                self._prev_user = self._filetime_to_int(user)
        except Exception:
            pass

    def get_cpu_percent(self) -> int:
        if HAS_PSUTIL:
            try:
                return int(psutil.cpu_percent(interval=None))
            except Exception:
                return 20

        # Ctypes GetSystemTimes fallback
        if os.name == "nt":
            try:
                idle = FILETIME()
                kernel = FILETIME()
                user = FILETIME()
                if ctypes.windll.kernel32.GetSystemTimes(ctypes.byref(idle), ctypes.byref(kernel), ctypes.byref(user)):
                    curr_idle = self._filetime_to_int(idle)
                    curr_kernel = self._filetime_to_int(kernel)
                    curr_user = self._filetime_to_int(user)

                    delta_idle = curr_idle - self._prev_idle
                    delta_kernel = curr_kernel - self._prev_kernel
                    delta_user = curr_user - self._prev_user

                    self._prev_idle = curr_idle
                    self._prev_kernel = curr_kernel
                    self._prev_user = curr_user

                    total = delta_kernel + delta_user
                    if total > 0 and delta_idle <= total:
                        pct = int(100.0 * (total - delta_idle) / total)
                        return max(0, min(100, pct))
            except Exception:
                pass
        return 20

    def get_ram_percent(self) -> int:
        if HAS_PSUTIL:
            try:
                return int(psutil.virtual_memory().percent)
            except Exception:
                return 50

        # Ctypes GlobalMemoryStatusEx fallback
        if os.name == "nt":
            try:
                stat = MEMORYSTATUSEX()
                stat.dwLength = ctypes.sizeof(MEMORYSTATUSEX)
                if ctypes.windll.kernel32.GlobalMemoryStatusEx(ctypes.byref(stat)):
                    return int(stat.dwMemoryLoad)
            except Exception:
                pass
        return 50

    def get_gpu_and_temp(self, cpu_usage: int) -> tuple[int, int]:
        now = time.time()
        # Query nvidia-smi at most once every 2 seconds to avoid overhead
        if self._has_nvidia_smi and (now - self._last_gpu_query_time > 2.0):
            self._last_gpu_query_time = now
            try:
                out = subprocess.check_output(
                    ["nvidia-smi", "--query-gpu=utilization.gpu,temperature.gpu", "--format=csv,noheader,nounits"],
                    timeout=0.6,
                    text=True,
                    stderr=subprocess.DEVNULL
                )
                parts = [p.strip() for p in out.strip().split(",")]
                if len(parts) >= 2:
                    self._cached_gpu = max(0, min(100, int(float(parts[0]))))
                    self._cached_temp = max(20, min(110, int(float(parts[1]))))
            except Exception:
                pass

        # If temperature is not provided by GPU, calculate a realistic thermal curve
        temp = self._cached_temp
        if not self._has_nvidia_smi:
            temp = int(38 + (cpu_usage * 0.35))

        return self._cached_gpu, temp

    def collect(self, config: CompanionConfig) -> tuple[int, int, int, int]:
        raw_cpu = self.get_cpu_percent()
        raw_ram = self.get_ram_percent()
        raw_gpu, raw_temp = self.get_gpu_and_temp(raw_cpu)

        cpu = raw_cpu if config.send_cpu else 0
        ram = raw_ram if config.send_ram else 0
        gpu = raw_gpu if config.send_gpu else 0
        temp = raw_temp if config.send_temp else 0
        return cpu, ram, gpu, temp

# ---------------------------------------------------------------------------
# Mochi Bot REST API Client
# ---------------------------------------------------------------------------
class MochiApiClient:
    @staticmethod
    def send_telemetry(host: str, token: str, cpu: int, ram: int, gpu: int, temp: int) -> tuple[bool, str]:
        url = f"http://{host}/api/v1/telemetry"
        payload = {
            "cpu_usage": cpu,
            "ram_usage": ram,
            "gpu_usage": gpu,
            "cpu_temp": temp
        }
        data = json.dumps(payload).encode("utf-8")
        headers = {
            "Content-Type": "application/json",
            "Authorization": f"Bearer {token}"
        }
        req = urllib.request.Request(url, data=data, headers=headers, method="POST")
        try:
            start_t = time.time()
            with urllib.request.urlopen(req, timeout=2.5) as resp:
                elapsed_ms = int((time.time() - start_t) * 1000)
                if resp.status == 200:
                    return True, f"OK ({elapsed_ms}ms)"
                return False, f"HTTP {resp.status}"
        except urllib.error.HTTPError as e:
            return False, f"HTTP {e.code}: {e.reason}"
        except urllib.error.URLError as e:
            return False, f"Network: {e.reason}"
        except Exception as e:
            return False, str(e)

    @staticmethod
    def send_notification(host: str, token: str, sender: str, msg: str, dur_ms: int = 5000) -> tuple[bool, str]:
        url = f"http://{host}/api/v1/notify"
        payload = {
            "sender": sender.strip() or "Windows PC",
            "msg": msg.strip(),
            "dur": int(dur_ms)
        }
        data = json.dumps(payload).encode("utf-8")
        headers = {
            "Content-Type": "application/json",
            "Authorization": f"Bearer {token}"
        }
        req = urllib.request.Request(url, data=data, headers=headers, method="POST")
        try:
            start_t = time.time()
            with urllib.request.urlopen(req, timeout=3.0) as resp:
                elapsed_ms = int((time.time() - start_t) * 1000)
                if resp.status == 200:
                    return True, f"Delivered to Mochi Screen ({elapsed_ms}ms)"
                return False, f"HTTP {resp.status}"
        except urllib.error.HTTPError as e:
            return False, f"Auth/HTTP {e.code}: {e.reason}"
        except urllib.error.URLError as e:
            return False, f"Unreachable: {e.reason}"
        except Exception as e:
            return False, str(e)

    @staticmethod
    def ping_status(host: str, token: str) -> tuple[bool, str, dict]:
        url = f"http://{host}/api/v1/status"
        headers = {
            "Authorization": f"Bearer {token}"
        }
        req = urllib.request.Request(url, headers=headers, method="GET")
        try:
            start_t = time.time()
            with urllib.request.urlopen(req, timeout=2.5) as resp:
                elapsed_ms = int((time.time() - start_t) * 1000)
                if resp.status == 200:
                    body = resp.read().decode("utf-8")
                    data = json.loads(body)
                    return True, f"Mochi Online ({elapsed_ms}ms)", data
                return False, f"HTTP {resp.status}", {}
        except Exception as e:
            return False, f"Failed: {e}", {}

# ---------------------------------------------------------------------------
# Background Telemetry Polling Worker
# ---------------------------------------------------------------------------
class TelemetryWorker:
    """Background worker managing metrics streaming and connection health."""

    STATE_OFFLINE = "OFFLINE"
    STATE_ONLINE = "ONLINE"
    STATE_RECONNECTING = "RECONNECTING"
    STATE_STALE = "STALE"

    def __init__(self, config: CompanionConfig, collector: SystemMetricsCollector, on_update_callback):
        self.config = config
        self.collector = collector
        self.on_update = on_update_callback

        self._stop_event = threading.Event()
        self._pause_event = threading.Event()
        self._thread: threading.Thread | None = None

        self.state = self.STATE_OFFLINE
        self.last_success_time = 0.0
        self.total_packets_sent = 0
        self.last_latency_str = "--"

    def start(self):
        if self._thread and self._thread.is_alive():
            return
        self._stop_event.clear()
        self._pause_event.clear()
        self._thread = threading.Thread(target=self._run_loop, name="MochiTelemetryWorker", daemon=True)
        self._thread.start()

    def pause(self):
        self._pause_event.set()
        self.state = self.STATE_OFFLINE

    def resume(self):
        self._pause_event.clear()

    def is_paused(self) -> bool:
        return self._pause_event.is_set()

    def stop(self):
        self._stop_event.set()
        self._pause_event.clear()
        if self._thread and self._thread.is_alive():
            self._thread.join(timeout=1.5)

    def _run_loop(self):
        while not self._stop_event.is_set():
            if self._pause_event.is_set():
                time.sleep(0.5)
                continue

            cpu, ram, gpu, temp = self.collector.collect(self.config)
            success, msg = MochiApiClient.send_telemetry(
                self.config.host, self.config.token, cpu, ram, gpu, temp
            )

            now = time.time()
            if success:
                self.state = self.STATE_ONLINE
                self.last_success_time = now
                self.total_packets_sent += 1
                self.last_latency_str = msg
            else:
                if self.last_success_time > 0 and (now - self.last_success_time > 15.0):
                    self.state = self.STATE_STALE
                else:
                    self.state = self.STATE_RECONNECTING
                self.last_latency_str = msg

            if self.on_update:
                try:
                    self.on_update(cpu, ram, gpu, temp, self.state, self.last_latency_str, self.total_packets_sent)
                except Exception:
                    pass

            # Sleep in small slices to allow prompt exit
            sleep_time = max(0.5, self.config.interval)
            step = 0.2
            elapsed = 0.0
            while elapsed < sleep_time and not self._stop_event.is_set() and not self._pause_event.is_set():
                time.sleep(step)
                elapsed += step

# ---------------------------------------------------------------------------
# Tray Icon Helper (PIL Image Generator)
# ---------------------------------------------------------------------------
def create_tray_image(state: str = TelemetryWorker.STATE_OFFLINE) -> Image.Image:
    """Generates a dynamic 64x64 pixel robot face icon with status dot."""
    img = Image.new("RGBA", (64, 64), color=(0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    # Robot chassis
    d.rounded_rectangle([6, 12, 58, 56], radius=10, fill=(30, 30, 46), outline=(137, 180, 250), width=3)
    # Antenna
    d.line([32, 12, 32, 4], fill=(137, 180, 250), width=3)
    d.ellipse([29, 2, 35, 8], fill=(137, 180, 250))

    # Eyes
    eye_color = (166, 227, 161) if state == TelemetryWorker.STATE_ONLINE else (205, 214, 244)
    d.ellipse([16, 26, 26, 38], fill=eye_color)
    d.ellipse([38, 26, 48, 38], fill=eye_color)

    # Status LED in corner
    status_colors = {
        TelemetryWorker.STATE_ONLINE: (166, 227, 161),       # Green
        TelemetryWorker.STATE_RECONNECTING: (249, 226, 175), # Amber
        TelemetryWorker.STATE_STALE: (243, 139, 168),        # Red
        TelemetryWorker.STATE_OFFLINE: (108, 112, 134)       # Gray
    }
    led_color = status_colors.get(state, (108, 112, 134))
    d.ellipse([46, 6, 58, 18], fill=led_color, outline=(24, 24, 37), width=2)
    return img

# ---------------------------------------------------------------------------
# Graphical User Interface (Tkinter)
# ---------------------------------------------------------------------------
class MochiCompanionGUI:
    """Tkinter Desktop Interface for Mochi Bot."""

    def __init__(self, root: tk.Tk, config: CompanionConfig):
        self.root = root
        self.config = config
        self.collector = SystemMetricsCollector()
        self.worker = TelemetryWorker(self.config, self.collector, self._on_telemetry_tick)

        self.tray_icon = None
        self.is_minimized_to_tray = False

        # Theme Colors
        self.COLOR_BG = "#181825"
        self.COLOR_CARD = "#1e1e2e"
        self.COLOR_CARD_BORDER = "#313244"
        self.COLOR_ACCENT = "#89b4fa"
        self.COLOR_FG = "#cdd6f4"
        self.COLOR_MUTED = "#a6adc8"
        self.COLOR_SUCCESS = "#a6e3a1"
        self.COLOR_WARNING = "#f9e2af"
        self.COLOR_DANGER = "#f38ba8"

        self._setup_window()
        self._setup_styles()
        self._build_ui()
        self._setup_tray()

        # Handle window close & Ctrl+C
        self.root.protocol("WM_DELETE_WINDOW", self.on_window_close)
        self.root.bind("<Control-c>", lambda e: self.quit_application())

        if self.config.auto_start_stream:
            self.root.after(500, self.worker.start)

    def _setup_window(self):
        self.root.title("Mochi Bot V1 - Windows Companion")
        self.root.geometry("540x680")
        self.root.minsize(500, 620)
        self.root.configure(bg=self.COLOR_BG)

        # Center on screen
        self.root.update_idletasks()
        w = self.root.winfo_width()
        h = self.root.winfo_height()
        x = (self.root.winfo_screenwidth() // 2) - (w // 2)
        y = (self.root.winfo_screenheight() // 2) - (h // 2)
        self.root.geometry(f"+{x}+{y}")

    def _setup_styles(self):
        style = ttk.Style()
        style.theme_use("clam")

        # Configure notebook & tabs
        style.configure("TNotebook", background=self.COLOR_BG, borderwidth=0)
        style.configure(
            "TNotebook.Tab",
            background=self.COLOR_CARD,
            foreground=self.COLOR_FG,
            padding=[16, 8],
            font=("Segoe UI", 10, "bold")
        )
        style.map(
            "TNotebook.Tab",
            background=[("selected", self.COLOR_ACCENT)],
            foreground=[("selected", "#11111b")]
        )

        # Progress bars
        style.configure(
            "Horizontal.TProgressbar",
            background=self.COLOR_ACCENT,
            troughcolor=self.COLOR_CARD_BORDER,
            borderwidth=0
        )

    def _build_ui(self):
        # 1. Header Banner
        header_frame = tk.Frame(self.root, bg=self.COLOR_CARD, padx=16, pady=12)
        header_frame.pack(fill=tk.X, padx=12, pady=(12, 6))

        title_box = tk.Frame(header_frame, bg=self.COLOR_CARD)
        title_box.pack(side=tk.LEFT)

        title_lbl = tk.Label(
            title_box,
            text="🤖 MOCHI BOT V1",
            font=("Segoe UI", 14, "bold"),
            bg=self.COLOR_CARD,
            fg=self.COLOR_FG
        )
        title_lbl.pack(anchor="w")

        subtitle_lbl = tk.Label(
            title_box,
            text="Desktop Telemetry & System Tray Companion",
            font=("Segoe UI", 9),
            bg=self.COLOR_CARD,
            fg=self.COLOR_MUTED
        )
        subtitle_lbl.pack(anchor="w")

        # Connection Status Badge
        self.status_badge = tk.Label(
            header_frame,
            text="● OFFLINE",
            font=("Segoe UI", 9, "bold"),
            bg="#313244",
            fg=self.COLOR_MUTED,
            padx=10,
            pady=4,
            relief="flat"
        )
        self.status_badge.pack(side=tk.RIGHT)

        # 2. Main Tabbed Notebook
        notebook = ttk.Notebook(self.root)
        notebook.pack(fill=tk.BOTH, expand=True, padx=12, pady=6)

        tab_telemetry = tk.Frame(notebook, bg=self.COLOR_BG, padx=10, pady=10)
        tab_notify = tk.Frame(notebook, bg=self.COLOR_BG, padx=10, pady=10)
        tab_settings = tk.Frame(notebook, bg=self.COLOR_BG, padx=10, pady=10)

        notebook.add(tab_telemetry, text="📊 Telemetry")
        notebook.add(tab_notify, text="🔔 Notification")
        notebook.add(tab_settings, text="⚙ Settings")

        self._build_tab_telemetry(tab_telemetry)
        self._build_tab_notify(tab_notify)
        self._build_tab_settings(tab_settings)

        # 3. Bottom Action Bar
        bottom_bar = tk.Frame(self.root, bg=self.COLOR_BG, padx=12, pady=8)
        bottom_bar.pack(fill=tk.X)

        btn_min = tk.Button(
            bottom_bar,
            text="🔽 Minimize to Tray" if HAS_PYSTRAY else "🔽 Minimize to Background",
            font=("Segoe UI", 9),
            bg=self.COLOR_CARD,
            fg=self.COLOR_FG,
            activebackground=self.COLOR_CARD_BORDER,
            relief="flat",
            padx=12,
            pady=6,
            command=self.minimize_window
        )
        btn_min.pack(side=tk.LEFT)

        btn_quit = tk.Button(
            bottom_bar,
            text="❌ Quit",
            font=("Segoe UI", 9),
            bg=self.COLOR_CARD,
            fg=self.COLOR_DANGER,
            activebackground=self.COLOR_CARD_BORDER,
            relief="flat",
            padx=12,
            pady=6,
            command=self.quit_application
        )
        btn_quit.pack(side=tk.RIGHT)

    # -----------------------------------------------------------------------
    # TAB 1: Telemetry & Live Monitor
    # -----------------------------------------------------------------------
    def _build_tab_telemetry(self, parent):
        # Server connection inputs
        conn_frame = tk.LabelFrame(
            parent,
            text=" Mochi Bot Network Endpoint ",
            font=("Segoe UI", 9, "bold"),
            bg=self.COLOR_CARD,
            fg=self.COLOR_ACCENT,
            padx=12,
            pady=8
        )
        conn_frame.pack(fill=tk.X, pady=(0, 8))

        row1 = tk.Frame(conn_frame, bg=self.COLOR_CARD)
        row1.pack(fill=tk.X, pady=2)
        tk.Label(row1, text="IP Address:", width=12, anchor="w", bg=self.COLOR_CARD, fg=self.COLOR_FG).pack(side=tk.LEFT)
        self.entry_ip = tk.Entry(row1, bg="#313244", fg=self.COLOR_FG, insertbackground=self.COLOR_FG, relief="flat", font=("Segoe UI", 10))
        self.entry_ip.insert(0, self.config.host)
        self.entry_ip.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 6))

        btn_ping = tk.Button(
            row1,
            text="Ping",
            font=("Segoe UI", 8, "bold"),
            bg=self.COLOR_ACCENT,
            fg="#11111b",
            activebackground="#b4befe",
            relief="flat",
            padx=8,
            pady=2,
            command=self._on_ping_clicked
        )
        btn_ping.pack(side=tk.RIGHT)

        row2 = tk.Frame(conn_frame, bg=self.COLOR_CARD)
        row2.pack(fill=tk.X, pady=4)
        tk.Label(row2, text="Bearer Token:", width=12, anchor="w", bg=self.COLOR_CARD, fg=self.COLOR_FG).pack(side=tk.LEFT)
        self.entry_token = tk.Entry(row2, bg="#313244", fg=self.COLOR_FG, insertbackground=self.COLOR_FG, relief="flat", show="*", font=("Segoe UI", 10))
        self.entry_token.insert(0, self.config.token)
        self.entry_token.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 6))

        self.btn_show_token = tk.Button(
            row2,
            text="Show",
            font=("Segoe UI", 8),
            bg="#313244",
            fg=self.COLOR_FG,
            relief="flat",
            padx=6,
            command=self._toggle_token_visibility
        )
        self.btn_show_token.pack(side=tk.RIGHT)

        row3 = tk.Frame(conn_frame, bg=self.COLOR_CARD)
        row3.pack(fill=tk.X, pady=4)
        tk.Label(row3, text="Poll Interval:", width=12, anchor="w", bg=self.COLOR_CARD, fg=self.COLOR_FG).pack(side=tk.LEFT)
        self.lbl_interval_val = tk.Label(row3, text=f"{self.config.interval:.1f}s", width=6, bg=self.COLOR_CARD, fg=self.COLOR_ACCENT, font=("Segoe UI", 9, "bold"))
        self.lbl_interval_val.pack(side=tk.RIGHT)
        self.scale_interval = tk.Scale(
            row3,
            from_=1.0,
            to=10.0,
            resolution=0.5,
            orient=tk.HORIZONTAL,
            showvalue=False,
            bg=self.COLOR_CARD,
            fg=self.COLOR_ACCENT,
            troughcolor="#313244",
            highlightthickness=0,
            command=self._on_interval_slider
        )
        self.scale_interval.set(self.config.interval)
        self.scale_interval.pack(side=tk.LEFT, fill=tk.X, expand=True)

        # Metrics Toggles and Real-time display grid
        metrics_frame = tk.LabelFrame(
            parent,
            text=" Hardware Telemetry Stream ",
            font=("Segoe UI", 9, "bold"),
            bg=self.COLOR_CARD,
            fg=self.COLOR_ACCENT,
            padx=12,
            pady=8
        )
        metrics_frame.pack(fill=tk.BOTH, expand=True, pady=(0, 8))

        # Checkbox variables
        self.var_cpu = tk.BooleanVar(value=self.config.send_cpu)
        self.var_ram = tk.BooleanVar(value=self.config.send_ram)
        self.var_gpu = tk.BooleanVar(value=self.config.send_gpu)
        self.var_temp = tk.BooleanVar(value=self.config.send_temp)

        # 4 Metric Cards
        self.stat_cards = {}
        row_cfgs = [
            ("cpu", "CPU Usage", self.var_cpu, "0 %"),
            ("ram", "RAM Usage", self.var_ram, "0 %"),
            ("gpu", "GPU Usage", self.var_gpu, "0 %"),
            ("temp", "CPU / Die Temp", self.var_temp, "0 °C"),
        ]

        for idx, (key, label, var, default_val) in enumerate(row_cfgs):
            card = tk.Frame(metrics_frame, bg="#24253a", padx=8, pady=6)
            card.pack(fill=tk.X, pady=3)

            top_line = tk.Frame(card, bg="#24253a")
            top_line.pack(fill=tk.X)

            chk = tk.Checkbutton(
                top_line,
                text=label,
                variable=var,
                font=("Segoe UI", 9, "bold"),
                bg="#24253a",
                fg=self.COLOR_FG,
                selectcolor="#313244",
                activebackground="#24253a",
                activeforeground=self.COLOR_FG,
                command=self._sync_toggles
            )
            chk.pack(side=tk.LEFT)

            val_lbl = tk.Label(top_line, text=default_val, font=("Segoe UI", 10, "bold"), bg="#24253a", fg=self.COLOR_ACCENT)
            val_lbl.pack(side=tk.RIGHT)

            pbar = ttk.Progressbar(card, orient=tk.HORIZONTAL, mode="determinate", style="Horizontal.TProgressbar")
            pbar.pack(fill=tk.X, pady=(4, 2))

            self.stat_cards[key] = {"val_lbl": val_lbl, "pbar": pbar}

        # Stream Controls & Info footer
        ctrl_frame = tk.Frame(parent, bg=self.COLOR_BG)
        ctrl_frame.pack(fill=tk.X, pady=4)

        self.btn_stream_toggle = tk.Button(
            ctrl_frame,
            text="⏸ Pause Streaming",
            font=("Segoe UI", 10, "bold"),
            bg="#313244",
            fg=self.COLOR_FG,
            activebackground=self.COLOR_CARD_BORDER,
            relief="flat",
            pady=8,
            command=self._toggle_stream_state
        )
        self.btn_stream_toggle.pack(fill=tk.X)

        self.lbl_telemetry_footer = tk.Label(
            ctrl_frame,
            text="Packets: 0 | Latency: -- | Engine: " + ("psutil" if HAS_PSUTIL else "ctypes fallback"),
            font=("Segoe UI", 8),
            bg=self.COLOR_BG,
            fg=self.COLOR_MUTED,
            pady=4
        )
        self.lbl_telemetry_footer.pack()

    # -----------------------------------------------------------------------
    # TAB 2: Quick Notification Popup Sender
    # -----------------------------------------------------------------------
    def _build_tab_notify(self, parent):
        desc = tk.Label(
            parent,
            text="Dispatch instant popup notifications to Mochi Bot's OLED screen.\nAppears over active face or dock cards for the specified duration.",
            font=("Segoe UI", 9),
            bg=self.COLOR_BG,
            fg=self.COLOR_MUTED,
            justify=tk.LEFT
        )
        desc.pack(anchor="w", pady=(0, 10))

        # Preset sender chips
        chips_frame = tk.LabelFrame(
            parent,
            text=" Preset App Sources ",
            font=("Segoe UI", 9, "bold"),
            bg=self.COLOR_CARD,
            fg=self.COLOR_ACCENT,
            padx=10,
            pady=8
        )
        chips_frame.pack(fill=tk.X, pady=(0, 10))

        presets = ["Discord", "Outlook", "Steam", "Slack", "Windows", "GitHub"]
        btn_box = tk.Frame(chips_frame, bg=self.COLOR_CARD)
        btn_box.pack(fill=tk.X)
        for name in presets:
            btn = tk.Button(
                btn_box,
                text=name,
                font=("Segoe UI", 8),
                bg="#313244",
                fg=self.COLOR_FG,
                activebackground=self.COLOR_ACCENT,
                activeforeground="#11111b",
                relief="flat",
                padx=8,
                pady=2,
                command=lambda n=name: self._set_notify_preset(n)
            )
            btn.pack(side=tk.LEFT, padx=3, pady=2)

        # Form fields
        form_frame = tk.LabelFrame(
            parent,
            text=" Notification Content ",
            font=("Segoe UI", 9, "bold"),
            bg=self.COLOR_CARD,
            fg=self.COLOR_ACCENT,
            padx=12,
            pady=10
        )
        form_frame.pack(fill=tk.X, pady=(0, 10))

        tk.Label(form_frame, text="Sender / App Name:", font=("Segoe UI", 9), bg=self.COLOR_CARD, fg=self.COLOR_FG).pack(anchor="w")
        self.entry_notif_sender = tk.Entry(form_frame, bg="#313244", fg=self.COLOR_FG, insertbackground=self.COLOR_FG, relief="flat", font=("Segoe UI", 10))
        self.entry_notif_sender.insert(0, "Discord")
        self.entry_notif_sender.pack(fill=tk.X, pady=(2, 8))

        tk.Label(form_frame, text="Message Body (up to 3 lines):", font=("Segoe UI", 9), bg=self.COLOR_CARD, fg=self.COLOR_FG).pack(anchor="w")
        self.entry_notif_msg = tk.Entry(form_frame, bg="#313244", fg=self.COLOR_FG, insertbackground=self.COLOR_FG, relief="flat", font=("Segoe UI", 10))
        self.entry_notif_msg.insert(0, "Incoming voice call from Team...")
        self.entry_notif_msg.pack(fill=tk.X, pady=(2, 8))

        # Duration slider
        dur_box = tk.Frame(form_frame, bg=self.COLOR_CARD)
        dur_box.pack(fill=tk.X, pady=(0, 4))
        tk.Label(dur_box, text="Display Duration:", font=("Segoe UI", 9), bg=self.COLOR_CARD, fg=self.COLOR_FG).pack(side=tk.LEFT)
        self.lbl_dur_val = tk.Label(dur_box, text="5 sec", font=("Segoe UI", 9, "bold"), bg=self.COLOR_CARD, fg=self.COLOR_ACCENT)
        self.lbl_dur_val.pack(side=tk.RIGHT)

        self.scale_dur = tk.Scale(
            form_frame,
            from_=2,
            to=20,
            resolution=1,
            orient=tk.HORIZONTAL,
            showvalue=False,
            bg=self.COLOR_CARD,
            fg=self.COLOR_ACCENT,
            troughcolor="#313244",
            highlightthickness=0,
            command=lambda v: self.lbl_dur_val.configure(text=f"{int(float(v))} sec")
        )
        self.scale_dur.set(5)
        self.scale_dur.pack(fill=tk.X, pady=(0, 8))

        # Send Action Button
        self.btn_send_notif = tk.Button(
            parent,
            text="🚀 Send to Mochi Screen",
            font=("Segoe UI", 11, "bold"),
            bg=self.COLOR_ACCENT,
            fg="#11111b",
            activebackground="#b4befe",
            relief="flat",
            pady=10,
            command=self._on_send_notification_clicked
        )
        self.btn_send_notif.pack(fill=tk.X, pady=4)

        # Delivery status badge
        self.lbl_notif_status = tk.Label(
            parent,
            text="Ready to dispatch notifications.",
            font=("Segoe UI", 9),
            bg=self.COLOR_BG,
            fg=self.COLOR_MUTED,
            pady=6
        )
        self.lbl_notif_status.pack()

    # -----------------------------------------------------------------------
    # TAB 3: Settings & Diagnostics
    # -----------------------------------------------------------------------
    def _build_tab_settings(self, parent):
        pref_frame = tk.LabelFrame(
            parent,
            text=" Companion Preferences ",
            font=("Segoe UI", 9, "bold"),
            bg=self.COLOR_CARD,
            fg=self.COLOR_ACCENT,
            padx=12,
            pady=8
        )
        pref_frame.pack(fill=tk.X, pady=(0, 10))

        self.var_min_tray = tk.BooleanVar(value=self.config.minimize_to_tray_on_close)
        self.var_autostart = tk.BooleanVar(value=self.config.auto_start_stream)

        chk_tray = tk.Checkbutton(
            pref_frame,
            text="Minimize to Windows System Tray on close (X)",
            variable=self.var_min_tray,
            font=("Segoe UI", 9),
            bg=self.COLOR_CARD,
            fg=self.COLOR_FG,
            selectcolor="#313244",
            activebackground=self.COLOR_CARD,
            activeforeground=self.COLOR_FG,
            command=self._save_active_config
        )
        chk_tray.pack(anchor="w", pady=3)

        chk_auto = tk.Checkbutton(
            pref_frame,
            text="Auto-start telemetry streaming on launch",
            variable=self.var_autostart,
            font=("Segoe UI", 9),
            bg=self.COLOR_CARD,
            fg=self.COLOR_FG,
            selectcolor="#313244",
            activebackground=self.COLOR_CARD,
            activeforeground=self.COLOR_FG,
            command=self._save_active_config
        )
        chk_auto.pack(anchor="w", pady=3)

        # Diagnostics Information
        diag_frame = tk.LabelFrame(
            parent,
            text=" Runtime Engine Diagnostics ",
            font=("Segoe UI", 9, "bold"),
            bg=self.COLOR_CARD,
            fg=self.COLOR_ACCENT,
            padx=12,
            pady=8
        )
        diag_frame.pack(fill=tk.BOTH, expand=True, pady=(0, 10))

        gpu_src = "NVIDIA SMI" if self.collector._has_nvidia_smi else "Synthetic Thermal Model"
        tray_src = "pystray (System Tray Icon)" if HAS_PYSTRAY else "Fallback (Window Minimize)"
        perf_src = "psutil library" if HAS_PSUTIL else "Windows ctypes GetSystemTimes / GlobalMemoryStatusEx"

        diag_lines = [
            f"Python Runtime: {sys.version.split()[0]}",
            f"Performance Source: {perf_src}",
            f"Tray Integration: {tray_src}",
            f"GPU/Temp Sensor: {gpu_src}",
            f"Config Path: {DEFAULT_CONFIG_PATH}",
        ]

        for line in diag_lines:
            tk.Label(
                diag_frame,
                text=line,
                font=("Consolas", 8),
                bg=self.COLOR_CARD,
                fg=self.COLOR_MUTED,
                anchor="w",
                justify=tk.LEFT
            ).pack(fill=tk.X, pady=2)

        btn_save = tk.Button(
            parent,
            text="💾 Save Configuration File",
            font=("Segoe UI", 10, "bold"),
            bg=self.COLOR_CARD,
            fg=self.COLOR_FG,
            activebackground=self.COLOR_CARD_BORDER,
            relief="flat",
            pady=8,
            command=self._on_manual_save_config
        )
        btn_save.pack(fill=tk.X)

    # -----------------------------------------------------------------------
    # System Tray Management
    # -----------------------------------------------------------------------
    def _setup_tray(self):
        if not HAS_PYSTRAY:
            return

        image = create_tray_image(TelemetryWorker.STATE_OFFLINE)
        menu = pystray.Menu(
            pystray.MenuItem("Mochi Bot V1 Companion", None, enabled=False),
            pystray.Menu.SEPARATOR,
            pystray.MenuItem("Show Window", self._on_tray_show),
            pystray.MenuItem("Toggle Streaming", self._on_tray_toggle_stream),
            pystray.Menu.SEPARATOR,
            pystray.MenuItem("Exit", self._on_tray_exit)
        )
        self.tray_icon = pystray.Icon("mochi_bot_companion", image, "Mochi Bot V1 Companion", menu)
        # Run detached in a background daemon thread
        threading.Thread(target=self.tray_icon.run, daemon=True, name="MochiTrayThread").start()

    def _update_tray_icon(self, state: str):
        if self.tray_icon and HAS_PYSTRAY:
            try:
                new_img = create_tray_image(state)
                self.tray_icon.icon = new_img
                self.tray_icon.title = f"Mochi Bot: {state}"
            except Exception:
                pass

    def _on_tray_show(self, icon=None, item=None):
        self.root.after(0, self.restore_window)

    def _on_tray_toggle_stream(self, icon=None, item=None):
        self.root.after(0, self._toggle_stream_state)

    def _on_tray_exit(self, icon=None, item=None):
        self.root.after(0, self.quit_application)

    def minimize_window(self):
        if HAS_PYSTRAY and self.var_min_tray.get():
            self.root.withdraw()
            self.is_minimized_to_tray = True
        else:
            self.root.iconify()

    def restore_window(self):
        self.root.deiconify()
        self.root.lift()
        self.root.focus_force()
        self.is_minimized_to_tray = False

    def on_window_close(self):
        if HAS_PYSTRAY and self.var_min_tray.get():
            self.minimize_window()
        else:
            self.quit_application()

    # -----------------------------------------------------------------------
    # Event Handlers & Callbacks
    # -----------------------------------------------------------------------
    def _on_telemetry_tick(self, cpu: int, ram: int, gpu: int, temp: int, state: str, latency: str, packets: int):
        # Marshals background thread update into the Tkinter main loop
        self.root.after(0, self._update_ui_state, cpu, ram, gpu, temp, state, latency, packets)

    def _update_ui_state(self, cpu: int, ram: int, gpu: int, temp: int, state: str, latency: str, packets: int):
        # Update connection badge
        state_configs = {
            TelemetryWorker.STATE_ONLINE: ("● ONLINE", self.COLOR_SUCCESS, "#1e382b"),
            TelemetryWorker.STATE_RECONNECTING: ("● RECONNECTING", self.COLOR_WARNING, "#3d321d"),
            TelemetryWorker.STATE_STALE: ("● STALE (>15s)", self.COLOR_DANGER, "#3d1d24"),
            TelemetryWorker.STATE_OFFLINE: ("● OFFLINE", self.COLOR_MUTED, "#313244"),
        }
        text, fg, bg = state_configs.get(state, ("● UNKNOWN", self.COLOR_MUTED, "#313244"))
        self.status_badge.configure(text=text, fg=fg, bg=bg)

        # Update metric bars
        self.stat_cards["cpu"]["val_lbl"].configure(text=f"{cpu:2d} %")
        self.stat_cards["cpu"]["pbar"]["value"] = cpu

        self.stat_cards["ram"]["val_lbl"].configure(text=f"{ram:2d} %")
        self.stat_cards["ram"]["pbar"]["value"] = ram

        self.stat_cards["gpu"]["val_lbl"].configure(text=f"{gpu:2d} %")
        self.stat_cards["gpu"]["pbar"]["value"] = gpu

        self.stat_cards["temp"]["val_lbl"].configure(text=f"{temp:2d} °C")
        self.stat_cards["temp"]["pbar"]["value"] = min(100, temp)

        # Update footer diagnostics
        engine = "psutil" if HAS_PSUTIL else "native ctypes"
        self.lbl_telemetry_footer.configure(
            text=f"Packets: {packets} | Last: {latency} | Engine: {engine}"
        )

        # Update tray icon color
        self._update_tray_icon(state)

    def _on_interval_slider(self, val):
        flt_val = float(val)
        self.lbl_interval_val.configure(text=f"{flt_val:.1f}s")
        self.config.interval = flt_val
        self.worker.config.interval = flt_val
        self._save_active_config()

    def _toggle_token_visibility(self):
        if self.entry_token.cget("show") == "*":
            self.entry_token.configure(show="")
            self.btn_show_token.configure(text="Hide")
        else:
            self.entry_token.configure(show="*")
            self.btn_show_token.configure(text="Show")

    def _sync_toggles(self):
        self.config.send_cpu = self.var_cpu.get()
        self.config.send_ram = self.var_ram.get()
        self.config.send_gpu = self.var_gpu.get()
        self.config.send_temp = self.var_temp.get()
        self._save_active_config()

    def _save_active_config(self):
        self.config.host = self.entry_ip.get().strip()
        self.config.token = self.entry_token.get().strip()
        self.config.interval = float(self.scale_interval.get())
        self.config.send_cpu = self.var_cpu.get()
        self.config.send_ram = self.var_ram.get()
        self.config.send_gpu = self.var_gpu.get()
        self.config.send_temp = self.var_temp.get()
        self.config.minimize_to_tray_on_close = self.var_min_tray.get()
        self.config.auto_start_stream = self.var_autostart.get()
        self.config.save()

    def _on_manual_save_config(self):
        self._save_active_config()
        messagebox.showinfo("Mochi Companion", f"Configuration successfully saved to:\n{DEFAULT_CONFIG_PATH}")

    def _toggle_stream_state(self):
        if self.worker.is_paused() or self.worker.state == TelemetryWorker.STATE_OFFLINE:
            self._save_active_config()
            self.worker.resume()
            self.worker.start()
            self.btn_stream_toggle.configure(text="⏸ Pause Streaming", bg="#313244", fg=self.COLOR_FG)
        else:
            self.worker.pause()
            self.btn_stream_toggle.configure(text="▶ Resume Streaming", bg=self.COLOR_ACCENT, fg="#11111b")
            self._update_ui_state(0, 0, 0, 0, TelemetryWorker.STATE_OFFLINE, "Paused", self.worker.total_packets_sent)

    def _on_ping_clicked(self):
        host = self.entry_ip.get().strip()
        token = self.entry_token.get().strip()

        def do_ping():
            ok, msg, data = MochiApiClient.ping_status(host, token)
            def update_ping_result():
                if ok:
                    uptime = data.get("uptime_sec", 0)
                    heap = data.get("free_heap", 0)
                    messagebox.showinfo("Ping Success", f"Connected to Mochi Bot!\nResponse: {msg}\nUptime: {uptime}s | Free Heap: {heap} B")
                else:
                    messagebox.showerror("Ping Failed", f"Could not reach Mochi Bot at {host}:\n{msg}")
            self.root.after(0, update_ping_result)

        threading.Thread(target=do_ping, daemon=True).start()

    def _set_notify_preset(self, app_name: str):
        self.entry_notif_sender.delete(0, tk.END)
        self.entry_notif_sender.insert(0, app_name)
        presets_messages = {
            "Discord": "Voice channel joined: General",
            "Outlook": "Calendar reminder: Sprint Review in 10m",
            "Steam": "Friend is playing Portal 2",
            "Slack": "#dev-ops: Build #142 deployed",
            "Windows": "Focus session completed (25m)",
            "GitHub": "PR #42 approved and merged!"
        }
        msg = presets_messages.get(app_name, "Notification alert")
        self.entry_notif_msg.delete(0, tk.END)
        self.entry_notif_msg.insert(0, msg)

    def _on_send_notification_clicked(self):
        host = self.entry_ip.get().strip()
        token = self.entry_token.get().strip()
        sender = self.entry_notif_sender.get().strip() or "Windows PC"
        msg = self.entry_notif_msg.get().strip()
        dur_ms = int(float(self.scale_dur.get()) * 1000)

        if not msg:
            messagebox.showwarning("Empty Message", "Please type a notification message to send.")
            return

        self.btn_send_notif.configure(state=tk.DISABLED, text="Sending...")
        self.lbl_notif_status.configure(text="Dispatching notification payload...", fg=self.COLOR_MUTED)

        def do_send():
            ok, resp = MochiApiClient.send_notification(host, token, sender, msg, dur_ms)
            def on_done():
                self.btn_send_notif.configure(state=tk.NORMAL, text="🚀 Send to Mochi Screen")
                if ok:
                    self.lbl_notif_status.configure(text=f"✓ {resp}", fg=self.COLOR_SUCCESS)
                else:
                    self.lbl_notif_status.configure(text=f"✗ {resp}", fg=self.COLOR_DANGER)
            self.root.after(0, on_done)

        threading.Thread(target=do_send, daemon=True).start()

    def quit_application(self):
        """Performs clean shutdown of threads, tray icon, and UI."""
        self._save_active_config()
        self.worker.stop()
        if self.tray_icon and HAS_PYSTRAY:
            try:
                self.tray_icon.stop()
            except Exception:
                pass
        self.root.quit()
        self.root.destroy()

# ---------------------------------------------------------------------------
# Main Entry Point & CLI
# ---------------------------------------------------------------------------
def main():
    # Setup signal handler for graceful console Ctrl+C shutdown
    signal.signal(signal.SIGINT, lambda sig, frame: sys.exit(0))

    parser = argparse.ArgumentParser(description="Mochi Bot V1 - Windows Companion & Tray Application")
    parser.add_argument("--host", help="Mochi Bot IP or hostname (overrides config)")
    parser.add_argument("--token", help="Bearer auth token (overrides config)")
    parser.add_argument("--interval", type=float, help="Telemetry polling interval in seconds")
    parser.add_argument("--config", default=DEFAULT_CONFIG_PATH, help="Path to JSON configuration file")
    parser.add_argument("--minimized", action="store_true", help="Start minimized to Windows system tray")
    parser.add_argument("--notify", nargs=2, metavar=("SENDER", "MESSAGE"), help="Send a one-shot notification and exit")

    args = parser.parse_args()

    # Load configuration
    config = CompanionConfig.load(args.config)
    if args.host:
        config.host = args.host
    if args.token:
        config.token = args.token
    if args.interval:
        config.interval = max(0.5, args.interval)

    # Handle one-shot CLI notification
    if args.notify:
        sender, msg = args.notify
        ok, res = MochiApiClient.send_notification(config.host, config.token, sender, msg)
        print(f"[{'+' if ok else '!'}] {res}")
        sys.exit(0 if ok else 1)

    # Create root Tkinter instance
    root = tk.Tk()
    app = MochiCompanionGUI(root, config)

    if args.minimized:
        root.after(100, app.minimize_window)

    try:
        root.mainloop()
    except KeyboardInterrupt:
        app.quit_application()

if __name__ == "__main__":
    main()
