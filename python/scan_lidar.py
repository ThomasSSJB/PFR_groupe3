#!/usr/bin/env python3

import sys
import time
from rplidar import RPLidar

PORT = '/dev/ttyUSB0'

def run():

    sys.stdout.reconfigure(line_buffering=True)


    try:
        print("▶  Connexion au LiDAR...", file=sys.stderr)

        lidar.stop()
        lidar.stop_motor()

        time.sleep(1)

        print("⏳  Démarrage moteur — attente 2 s...", file=sys.stderr)

        lidar.start_motor()

        time.sleep(2)

        lidar.clean_input()

        print("🔄  Scan en cours...", file=sys.stderr)

        start = time.time()

        for scan in lidar.iter_scans(max_buf_meas=500):

            for (_, angle, distance) in scan:

                if 100 < distance < 6000:

                    print(f"{angle:.2f},{distance:.1f}")
                    sys.stdout.flush()

            # stop après 8 sec
            if time.time() - start > 8:
                break

    except Exception as e:

        print(f"Erreur Lidar: {e}", file=sys.stderr)

    finally:

        print("⏹  Arrêt du LiDAR...", file=sys.stderr)

        try:
            lidar.stop()
            lidar.stop_motor()
            lidar.disconnect()
        except:
            pass

        print("✅  LiDAR déconnecté.", file=sys.stderr)

if __name__ == "__main__":
    run()