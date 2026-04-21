import os
import ctypes
import subprocess


def is_admin():
    try:
        is_admin = (os.getuid() == 0)

    except AttributeError:
        is_admin = ctypes.windll.shell32.IsUserAnAdmin() != 0

    return is_admin


def main():

    if not is_admin():
        print("     Du måste köra detta program som administratör/root för att installera Foglang2")
        exit(-1)


    name = os.name

    filepath = input("  Var vill du lägga installationen? [sökväg] (enter för default): ")


    if name == "nt":

        filepath = filepath.replace("/", "\\")
        print(f"     Installerar för Windows i {filepath}...")

        status = subprocess.run(f".\\make.bat {filepath}")
        exit_code = os.WEXITSTATUS(status)

        if exit_code != 0:
            print("     Installation misslyckades, installation avbruten")
            exit(-1)

    elif name == "posix":

        if not filepath:
            filepath = "/usr/local/bin"
        print(f"     Installerar för Linux/MacOS/BSD i {filepath}...")

        filepath += "/foglang2"

        status1 = subprocess.run(["make", "clean", f"TARGET={filepath}"]).returncode
        exit_code1 = os.WEXITSTATUS(status1)


        if exit_code1 != 0:
            print("     Rensning misslyckades, installation avbruten")
            exit(-1)

        status2 = subprocess.run(["make", f"TARGET={filepath}"]).returncode
        exit_code2 = os.WEXITSTATUS(status2)

        if exit_code2 != 0:
            print("     Kompilering misslyckades, installation avbruten")
            exit(-1)

        print("     Foglang2 installerat!")
    else:
        print("Okänt operativsystem, installation misslyckades")
        exit(-1)

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n     Avbröt installationen")
        exit(0)