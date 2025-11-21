import subprocess
import re

class AudioManager:
    @staticmethod
    def get_audio_devices():
        devices = []
        try:
            result = subprocess.run(['wpctl', 'status'], capture_output=True, text=True)
            output = result.stdout
        except Exception as e:
            print(f"Error running wpctl: {e}")
            return []

        lines = output.splitlines()
        in_sinks = False
        
        # Regex to match sink lines:
        # Optional indent, optional *, ID, dot, Name, [vol: ...]
        # Example: │  *   64. Navi 10 HDMI Audio Digital Stereo (HDMI) [vol: 0.40]
        sink_pattern = re.compile(r'^\s*│\s*(\*)?\s*(\d+)\.\s+(.+?)\s+\[vol:\s*([\d\.]+)')

        for line in lines:
            if "Sinks:" in line:
                in_sinks = True
                continue
            if in_sinks and not line.strip().startswith("│"):
                # End of sinks section (empty line or next section)
                if line.strip() == "":
                    continue # could be empty line inside? usually structure is strict
                if "Sources:" in line or "Filters:" in line or "Streams:" in line:
                    in_sinks = False
                    break
            
            if in_sinks:
                match = sink_pattern.search(line)
                if match:
                    is_active = bool(match.group(1))
                    device_id = match.group(2)
                    name = match.group(3).strip()
                    volume_str = match.group(4)
                    volume = float(volume_str) if volume_str else 0.0
                    
                    # Icon heuristic
                    icon_name = "audio-speakers-symbolic"
                    lower_name = name.lower()
                    if "headphone" in lower_name:
                        icon_name = "audio-headphones-symbolic"
                    elif "hdmi" in lower_name or "tv" in lower_name or "display" in lower_name:
                        icon_name = "video-display-symbolic"
                    elif "bluetooth" in lower_name:
                        icon_name = "bluetooth-active-symbolic"
                    
                    devices.append((name, icon_name, is_active, volume, device_id))
        
        return devices

    @staticmethod
    def set_volume(device_id, value):
        try:
            # Set absolute volume
            subprocess.run(['wpctl', 'set-volume', str(device_id), str(value)], check=False)
        except Exception as e:
            print(f"Failed to set volume: {e}")

    @staticmethod
    def set_default_device(device_id):
        try:
            subprocess.run(['wpctl', 'set-default', str(device_id)], check=False)
        except Exception as e:
            print(f"Failed to set default device: {e}")
