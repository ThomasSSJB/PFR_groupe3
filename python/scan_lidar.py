import sys
import time
from rplidar import RPLidar

PORT = '/dev/ttyUSB0'

def run():
    sys.stdout.reconfigure(line_buffering=True)


    try:
        print("Lidar init...", file=sys.stderr)

        lidar.stop()
        lidar.stop_motor()
        time.sleep(0.5)

        lidar.start_motor()
        time.sleep(1)

        lidar.clean_input()

        for scan in lidar.iter_scans():
            for (_, angle, distance) in scan:
                if 100 < distance < 6000:  # filtre bruit
                    sys.stdout.write(f"{angle:.2f},{distance:.1f}\n")

    except Exception as e:
        print(f"Erreur Lidar: {e}", file=sys.stderr)

    finally:
        lidar.stop()
        lidar.stop_motor()
        lidar.disconnect()

if __name__ == "__main__":
    run()