import sys
import os
import ctypes

def is_admin():
    try:
        return ctypes.windll.shell32.IsUserAnAdmin()
    except:
        return False

def list_removable_drives():
    """List potential drive letters on Windows."""
    drives = []
    bitmask = ctypes.windll.kernel32.GetLogicalDrives()
    for letter in range(26):
        if bitmask & (1 << letter):
            drive_letter = chr(65 + letter) + ":"
            drive_type = ctypes.windll.kernel32.GetDriveTypeW(drive_letter)
            # DRIVE_REMOVABLE = 2
            if drive_type == 2:
                drives.append(drive_letter)
    return drives

def read_raw_sectors(device_path, start_sector=1000, max_sectors=20000, sector_size=512):
    print(f"[*] Opening device: {device_path}")
    try:
        # Open the raw disk/drive in read-only binary mode
        # Note: On Windows, raw disk access requires Administrator privileges.
        fd = os.open(device_path, os.O_RDONLY | os.O_BINARY)
    except PermissionError:
        print("[!] Permission Denied! You MUST run this script as Administrator (Elevated Command Prompt/PowerShell).")
        return []
    except FileNotFoundError:
        print(f"[!] Device not found: {device_path}")
        return []
    except Exception as e:
        print(f"[!] Error opening device: {e}")
        return []

    log_entries = []
    try:
        # Seek to the start sector
        start_offset = start_sector * sector_size
        os.lseek(fd, start_offset, os.SEEK_SET)
        print(f"[*] Successfully seeked to sector {start_sector} (offset {start_offset} bytes).")
        print("[*] Reading data...")

        empty_sector_count = 0
        for sector_idx in range(start_sector, start_sector + max_sectors):
            try:
                data = os.read(fd, sector_size)
                if not data or len(data) < sector_size:
                    print(f"[*] Hit end of disk or read short sector at sector {sector_idx}.")
                    break
                
                # Check if the sector is entirely empty (all 0x00 or all 0xFF)
                if all(b == 0 or b == 0xFF for b in data):
                    empty_sector_count += 1
                    if empty_sector_count >= 10:
                        # Stop if we hit 10 consecutive empty sectors
                        print(f"[*] Stopped reading after hitting 10 consecutive empty sectors at sector {sector_idx}.")
                        break
                    continue
                
                empty_sector_count = 0
                
                # Each 512-byte sector contains 8 chunks of 64 bytes
                for chunk_idx in range(8):
                    chunk = data[chunk_idx * 64 : (chunk_idx + 1) * 64]
                    # Check if the chunk starts with 'UP:' (legacy) or '20' (new Beijing time format)
                    if chunk[0:3] == b'UP:' or chunk[0:2] == b'20':
                        try:
                            # Try to decode the line as ASCII
                            line = chunk.decode('ascii', errors='ignore').strip()
                            log_entries.append((sector_idx, chunk_idx, line))
                        except Exception as e:
                            pass
            except Exception as e:
                print(f"[!] Error reading sector {sector_idx}: {e}")
                break
    finally:
        os.close(fd)
    
    return log_entries

def main():
    print("====================================================")
    print("      STM32 SD Card Raw Sector Log Reader (PC)      ")
    print("====================================================")
    
    if not is_admin() and os.name == 'nt':
        print("[WARNING] This script is not running with Administrator privileges.")
        print("          Raw disk access on Windows requires admin rights.")
        print("          Please restart your terminal/IDE as Administrator.\n")
    
    if os.name == 'nt':
        removable = list_removable_drives()
        if removable:
            print("[*] Detected removable drive letters:")
            for d in removable:
                print(f"    - {d}")
        else:
            print("[*] No removable drives detected automatically.")
            
        print("\nEnter target drive:")
        print("Examples:")
        print("  - Enter drive letter (e.g. 'G' or 'G:') to read that volume.")
        print("  - Enter physical drive number (e.g. '1' for \\\\.\\PhysicalDrive1).")
        target = input("Target: ").strip()
        
        if not target:
            print("[!] Target cannot be empty.")
            return
            
        if target.isdigit():
            device_path = f"\\\\.\\PhysicalDrive{target}"
        else:
            letter = target.rstrip(':').upper()
            device_path = f"\\\\.\\{letter}:"
    else:
        # Linux / MacOS raw disk paths
        print("Enter device path (e.g., /dev/sdb or /dev/disk2):")
        device_path = input("Target: ").strip()
        if not device_path:
            return

    # Log file output path
    output_filename = "sd_log_extracted.txt"
    
    # Read the data
    entries = read_raw_sectors(device_path, start_sector=1000)
    
    if not entries:
        print("[*] No valid log records found starting at sector 1000.")
        return
        
    print(f"\n[*] Successfully read {len(entries)} valid records.")
    
    # Save to file
    try:
        with open(output_filename, "w", encoding="utf-8") as f:
            f.write(f"--- Extracted SD Log (Total Records: {len(entries)}) ---\n")
            for sector, chunk, line in entries:
                f.write(f"[Sector {sector:05d}, Chunk {chunk}] {line}\n")
        print(f"[+] Successfully saved logs to: {os.path.abspath(output_filename)}")
    except Exception as e:
        print(f"[!] Error writing output file: {e}")

if __name__ == "__main__":
    main()
