"""
config_tool.py — Công cụ cấu hình UWB Positioning System
=========================================================
Thay thế roleconfig.py với các tính năng mở rộng:
  - Gửi gói lệnh 'A' (có tọa độ x/y thực tế) cho anchor và tag
  - Hỗ trợ bất kỳ số lượng anchor (mặc định MAX=8)
  - Hỗ trợ 2 tag
  - Chế độ Interactive (CLI menu)  → python config_tool.py
  - Chế độ Batch từ file JSON       → python config_tool.py --batch nodes.json

Yêu cầu: pip install winrt-Windows.Devices.Bluetooth
          pip install winrt-Windows.Storage.Streams

Cài nhanh:
    pip install winrt-Windows.Devices.Bluetooth winrt-Windows.Storage.Streams

Protocol BLE gói 'A' (13 byte payload, Company ID 0x0059):
    [0]    = 0x41 ('A')   — magic byte
    [1..2] = target_mac   — 2 byte MAC cuối (little-endian), 0xFFFF = broadcast
    [3]    = role          — 1=TAG, 2=ANCHOR
    [4]    = node_id
    [5..8] = x (float32, little-endian)
    [9..12]= y (float32, little-endian)
"""

import asyncio
import struct
import sys
import json
import argparse
import time

try:
    from winrt.windows.devices.bluetooth.advertisement import (
        BluetoothLEAdvertisementPublisher,
        BluetoothLEAdvertisement,
        BluetoothLEManufacturerData
    )
    from winrt.windows.storage.streams import DataWriter
except ImportError:
    print("Lỗi: Thiếu thư viện winrt.")
    print("Cài đặt bằng lệnh:")
    print("  pip install winrt-Windows.Devices.Bluetooth winrt-Windows.Storage.Streams")
    sys.exit(1)

# ─────────────────────────────────────────────────────────────────────────────
COMPANY_ID    = 0x0059   # Nordic Semiconductor Company ID
MAGIC_CMD_A   = 0x41     # Gói lệnh mới: 'A' (có tọa độ)
BROADCAST_MAC = 0xFFFF   # Gửi cho tất cả mạch

ROLE_TAG    = 1
ROLE_ANCHOR = 2
ROLE_CALIB  = 99

ROLE_NAMES = {
    ROLE_TAG:    "TAG",
    ROLE_ANCHOR: "ANCHOR",
    ROLE_CALIB:  "CALIB (99)",
}
# ─────────────────────────────────────────────────────────────────────────────


def build_payload_a(mac_hex: str, role: int, node_id: int, x: float, y: float) -> bytes:
    """
    Tạo payload 13 byte theo gói lệnh 'A':
      struct { uint8 magic; uint16 mac; uint8 role; uint8 id; float x; float y; }
    """
    mac_int = int(mac_hex, 16) if mac_hex.upper() != "FFFF" else BROADCAST_MAC
    return struct.pack('<BHBBFF', MAGIC_CMD_A, mac_int, role, node_id, x, y)


async def send_ble_packet(payload: bytes, duration_s: float = 7.0, repeat: int = 1):
    """
    Phát gói BLE Manufacturer Data với payload cho trước.
    Phát `repeat` lần, mỗi lần `duration_s` giây, nghỉ 2s giữa các lần.
    """
    writer = DataWriter()
    writer.write_bytes(payload)

    md = BluetoothLEManufacturerData()
    md.company_id = COMPANY_ID
    md.data       = writer.detach_buffer()

    ad = BluetoothLEAdvertisement()
    ad.manufacturer_data.append(md)

    publisher = BluetoothLEAdvertisementPublisher(ad)

    for i in range(repeat):
        print(f"  [Lần {i+1}/{repeat}] Phát BLE ({duration_s}s)...", end='', flush=True)
        publisher.start()
        await asyncio.sleep(duration_s)
        publisher.stop()
        print(" XONG")
        if i < repeat - 1:
            await asyncio.sleep(2.0)


async def send_node_config(mac_hex: str, role: int, node_id: int, x: float, y: float,
                            repeat: int = 2, verbose: bool = True):
    """Gửi lệnh cấu hình cho 1 mạch."""
    payload = build_payload_a(mac_hex, role, node_id, x, y)

    if verbose:
        mac_display = "FFFF (Broadcast)" if mac_hex.upper() == "FFFF" else mac_hex.upper()
        print(f"\n{'─'*50}")
        print(f"  MAC:   {mac_display}")
        print(f"  Role:  {ROLE_NAMES.get(role, str(role))}")
        print(f"  ID:    {node_id}")
        print(f"  Tọa độ: ({x:.2f}, {y:.2f}) m")
        print(f"  Payload ({len(payload)} bytes): " + " ".join(f"{b:02X}" for b in payload))
        print(f"{'─'*50}")

    await send_ble_packet(payload, duration_s=7.0, repeat=repeat)


# ─────────────────────────────────────────────────────────────────────────────
# Chế độ Interactive
# ─────────────────────────────────────────────────────────────────────────────

def print_banner():
    print("=" * 60)
    print("  UWB Positioning System — Config Tool v2.0")
    print("  Hỗ trợ: tối đa 8 Anchor + 2 Tag")
    print("  Gói lệnh 'A': role + id + tọa độ (x, y)")
    print("=" * 60)


def get_float(prompt: str, default: float = 0.0) -> float:
    while True:
        s = input(f"  {prompt} [mặc định {default:.2f}]: ").strip()
        if s == "":
            return default
        try:
            return float(s)
        except ValueError:
            print("  [!] Nhập số thực hợp lệ (ví dụ: 1.5)")


def get_int(prompt: str, lo: int, hi: int, default: int = None) -> int:
    while True:
        dflt_str = f" [mặc định {default}]" if default is not None else ""
        s = input(f"  {prompt}{dflt_str}: ").strip()
        if s == "" and default is not None:
            return default
        try:
            v = int(s)
            if lo <= v <= hi:
                return v
            print(f"  [!] Nhập số từ {lo} đến {hi}")
        except ValueError:
            print(f"  [!] Nhập số nguyên hợp lệ")


def get_mac(prompt: str) -> str:
    while True:
        s = input(f"  {prompt} (ví dụ: 8629, hoặc FFFF để broadcast): ").strip().upper()
        if s == "FFFF":
            return s
        try:
            v = int(s, 16)
            if 0 <= v <= 0xFFFE:
                return f"{v:04X}"
        except ValueError:
            pass
        print("  [!] Nhập mã hex 4 ký tự (0000–FFFE) hoặc FFFF")


async def interactive_mode():
    print_banner()
    print()

    while True:
        print("\nMENU CHÍNH:")
        print("  [1] Cấu hình 1 mạch (nhập thủ công)")
        print("  [2] Cấu hình nhanh nhiều mạch (wizard)")
        print("  [3] Gửi lệnh calib tự động (Role 99)")
        print("  [0] Thoát")
        print()

        choice = input("Chọn: ").strip()

        if choice == "0":
            print("Thoát.")
            break

        elif choice == "1":
            await _single_node_config()

        elif choice == "2":
            await _wizard_multi_node()

        elif choice == "3":
            await _send_calib()

        else:
            print("  [!] Chọn lại.")


async def _single_node_config():
    print("\n--- CẤU HÌNH 1 MẠCH ---")
    mac      = get_mac("Nhập 2 byte MAC cuối của mạch")
    role_num = get_int("Role (1=TAG, 2=ANCHOR, 99=CALIB)", 1, 99, default=2)
    node_id  = get_int("Node ID", 0, 7, default=0)

    x, y = 0.0, 0.0
    if role_num == ROLE_ANCHOR:
        print("  Nhập tọa độ thực tế của anchor (mét):")
        x = get_float("  X", 0.0)
        y = get_float("  Y", 0.0)
    elif role_num == ROLE_TAG:
        print("  [INFO] Tag không cần tọa độ thực tế — để 0,0")

    repeat = get_int("Số lần phát lại (1–5)", 1, 5, default=2)

    confirm = input(f"\n  Xác nhận gửi lệnh? (y/n): ").strip().lower()
    if confirm == 'y':
        await send_node_config(mac, role_num, node_id, x, y, repeat=repeat)
        print("\n  ✓ Đã gửi xong!")
    else:
        print("  Huỷ.")


async def _wizard_multi_node():
    print("\n--- WIZARD: CẤU HÌNH NHIỀU MẠCH ---")
    print("  Gửi lệnh cho từng mạch một. Mạch phải đang ở trạng thái chờ (LED nhấp nháy).")
    print()

    n_anchors = get_int("Số anchor", 3, 8, default=4)
    n_tags    = get_int("Số tag (tối đa 2)", 1, 2, default=2)

    nodes = []

    # Cấu hình các anchor
    for i in range(n_anchors):
        print(f"\n  === ANCHOR {i} ===")
        mac = get_mac(f"  MAC 2 byte của A{i}")
        x   = get_float(f"  Tọa độ X của A{i}", 0.0)
        y   = get_float(f"  Tọa độ Y của A{i}", 0.0)
        nodes.append({"mac": mac, "role": ROLE_ANCHOR, "id": i, "x": x, "y": y})

    # Cấu hình các tag
    for i in range(n_tags):
        print(f"\n  === TAG {i} ===")
        mac = get_mac(f"  MAC 2 byte của Tag{i}")
        nodes.append({"mac": mac, "role": ROLE_TAG, "id": i, "x": 0.0, "y": 0.0})

    # Preview
    print("\n" + "=" * 60)
    print("  XEM LẠI CẤU HÌNH:")
    print(f"  {'MAC':>8}  {'Role':>8}  {'ID':>3}  {'X':>7}  {'Y':>7}")
    print(f"  {'-'*8}  {'-'*8}  {'-'*3}  {'-'*7}  {'-'*7}")
    for n in nodes:
        rname = ROLE_NAMES.get(n['role'], str(n['role']))
        print(f"  {n['mac']:>8}  {rname:>8}  {n['id']:>3}  {n['x']:>7.2f}  {n['y']:>7.2f}")
    print("=" * 60)

    confirm = input("\n  Phát lệnh cho tất cả? (y/n): ").strip().lower()
    if confirm != 'y':
        print("  Huỷ.")
        return

    for node in nodes:
        print(f"\n>>> Đang gửi lệnh cho MAC {node['mac']}...")
        input("    [Nhấn ENTER khi mạch đã sẵn sàng]")
        await send_node_config(node['mac'], node['role'], node['id'], node['x'], node['y'], repeat=2)
        print(f"  ✓ Xong cho MAC {node['mac']}")

    print("\n✓ Hoàn tất tất cả cấu hình!")


async def _send_calib():
    print("\n--- GỬI LỆNH CALIB TỰ ĐỘNG (Role 99) ---")
    print("  Lệnh này yêu cầu TAG tự động calib tọa độ với Anchor.")
    mac    = get_mac("MAC của Tag cần calib (hoặc FFFF broadcast)")
    repeat = get_int("Số lần phát lại", 1, 5, default=3)
    confirm = input("\n  Xác nhận gửi? (y/n): ").strip().lower()
    if confirm == 'y':
        await send_node_config(mac, ROLE_CALIB, 0, 0.0, 0.0, repeat=repeat)
        print("\n  ✓ Đã gửi lệnh calib!")
    else:
        print("  Huỷ.")


# ─────────────────────────────────────────────────────────────────────────────
# Chế độ Batch từ file JSON
# ─────────────────────────────────────────────────────────────────────────────

async def batch_mode(json_file: str):
    """
    Đọc file JSON và phát lệnh lần lượt cho từng node.

    Ví dụ file JSON (nodes.json):
    {
      "repeat": 2,
      "delay_between_s": 1.0,
      "nodes": [
        {"mac": "8629", "role": 2, "id": 0, "x": 0.0,  "y": 0.0},
        {"mac": "A3F1", "role": 2, "id": 1, "x": 3.0,  "y": 0.0},
        {"mac": "B2C4", "role": 2, "id": 2, "x": 0.0,  "y": 2.5},
        {"mac": "D5E6", "role": 2, "id": 3, "x": 3.0,  "y": 2.5},
        {"mac": "F7A8", "role": 1, "id": 0, "x": 0.0,  "y": 0.0},
        {"mac": "C9B3", "role": 1, "id": 1, "x": 0.0,  "y": 0.0}
      ]
    }
    """
    print_banner()
    print(f"\nBATCH MODE — Đọc từ: {json_file}")

    try:
        with open(json_file, 'r', encoding='utf-8') as f:
            data = json.load(f)
    except FileNotFoundError:
        print(f"[LỖI] Không tìm thấy file: {json_file}")
        sys.exit(1)
    except json.JSONDecodeError as e:
        print(f"[LỖI] JSON không hợp lệ: {e}")
        sys.exit(1)

    nodes          = data.get("nodes", [])
    repeat         = data.get("repeat", 2)
    delay_between  = data.get("delay_between_s", 1.0)

    if not nodes:
        print("[LỖI] Không có node nào trong file JSON.")
        sys.exit(1)

    print(f"\nSố node: {len(nodes)} | Repeat: {repeat} | Delay giữa: {delay_between}s")
    print(f"\n{'─'*60}")
    print(f"  {'MAC':>8}  {'Role':>8}  {'ID':>3}  {'X':>7}  {'Y':>7}")
    print(f"  {'-'*8}  {'-'*8}  {'-'*3}  {'-'*7}  {'-'*7}")
    for n in nodes:
        rname = ROLE_NAMES.get(n.get('role', 0), str(n.get('role', 0)))
        print(f"  {n.get('mac','????'):>8}  {rname:>8}  {n.get('id', 0):>3}  "
              f"{n.get('x', 0.0):>7.2f}  {n.get('y', 0.0):>7.2f}")
    print(f"{'─'*60}\n")

    confirm = input("Bắt đầu phát lệnh? (y/n): ").strip().lower()
    if confirm != 'y':
        print("Đã huỷ.")
        return

    success = 0
    for idx, node in enumerate(nodes):
        mac     = node.get("mac", "FFFF")
        role    = node.get("role", ROLE_ANCHOR)
        node_id = node.get("id", 0)
        x       = node.get("x", 0.0)
        y       = node.get("y", 0.0)

        print(f"\n[{idx+1}/{len(nodes)}] Gửi lệnh → MAC {mac} | "
              f"Role={ROLE_NAMES.get(role,str(role))} | ID={node_id} | ({x:.2f},{y:.2f})")

        try:
            await send_node_config(mac, role, node_id, x, y, repeat=repeat, verbose=False)
            print(f"  ✓ Thành công")
            success += 1
        except Exception as e:
            print(f"  ✗ Lỗi: {e}")

        if idx < len(nodes) - 1:
            await asyncio.sleep(delay_between)

    print(f"\n{'='*60}")
    print(f"  Hoàn tất! Thành công: {success}/{len(nodes)}")
    print(f"{'='*60}")


# ─────────────────────────────────────────────────────────────────────────────
# Tạo file JSON mẫu
# ─────────────────────────────────────────────────────────────────────────────

def create_sample_json(filename: str = "nodes.json"):
    sample = {
        "_comment": "File mẫu cấu hình UWB Positioning System. Chỉnh sửa MAC và tọa độ thực tế.",
        "repeat": 2,
        "delay_between_s": 1.0,
        "nodes": [
            {"mac": "8629", "role": 2, "id": 0, "x": 0.00, "y": 0.00, "_note": "Anchor 0 - Góc gốc"},
            {"mac": "A3F1", "role": 2, "id": 1, "x": 3.00, "y": 0.00, "_note": "Anchor 1"},
            {"mac": "B2C4", "role": 2, "id": 2, "x": 0.00, "y": 2.50, "_note": "Anchor 2"},
            {"mac": "D5E6", "role": 2, "id": 3, "x": 3.00, "y": 2.50, "_note": "Anchor 3"},
            {"mac": "F7A8", "role": 1, "id": 0, "x": 0.00, "y": 0.00, "_note": "Tag 0"},
            {"mac": "C9B3", "role": 1, "id": 1, "x": 0.00, "y": 0.00, "_note": "Tag 1"}
        ]
    }
    with open(filename, 'w', encoding='utf-8') as f:
        json.dump(sample, f, indent=2, ensure_ascii=False)
    print(f"✓ Đã tạo file mẫu: {filename}")
    print("  Chỉnh sửa MAC và tọa độ, sau đó chạy:")
    print(f"    python config_tool.py --batch {filename}")


# ─────────────────────────────────────────────────────────────────────────────
# Entry point
# ─────────────────────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(
        description="UWB Positioning Config Tool — Gửi lệnh cấu hình BLE cho anchor/tag",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Ví dụ sử dụng:
  python config_tool.py                      # Chế độ interactive (menu)
  python config_tool.py --batch nodes.json   # Chế độ batch từ file JSON
  python config_tool.py --sample             # Tạo file nodes.json mẫu

Lưu ý: Chạy với quyền Administrator nếu BLE không hoạt động.
        """
    )
    parser.add_argument('--batch',  metavar='JSON_FILE', help='Chế độ batch: đọc cấu hình từ file JSON')
    parser.add_argument('--sample', action='store_true',  help='Tạo file nodes.json mẫu')
    args = parser.parse_args()

    if args.sample:
        create_sample_json()
        return

    try:
        if args.batch:
            asyncio.run(batch_mode(args.batch))
        else:
            asyncio.run(interactive_mode())
    except KeyboardInterrupt:
        print("\n\nĐã dừng chương trình.")


if __name__ == "__main__":
    main()
