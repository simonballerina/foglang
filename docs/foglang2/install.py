import os
import ctypes
import subprocess
import platform

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

    #windows
    if name == "nt":

        lib_path: str = "'C:\\Program Files\\foglang2\\lib'"

        if not filepath:
            filepath = "C:\\Program Files\\foglang2\\build"

        filepath = filepath.replace("/", "\\")
        print(f"     Installerar för Windows i {filepath}...")

        print(f"     Installerar Library i {lib_path}")
        subprocess.call(f"powershell -Command xcopy lib {lib_path} /s /y /i", shell=True)
        print("TODO: Lägg in följande miljövariabel \"C:\\Program Files\\foglang2\\build\"")
        exit_code = os.WEXITSTATUS(status)

        if exit_code != 0:
            print("     Installation misslyckades, installation avbruten")
            exit(-1)

    #unix
    elif name == "posix":
        #mac
        if platform.system() == "Darwin":
            user = os.environ.get("SUDO_USER")
            if not user:
                user = os.environ.get("USER")
            lib_path: str = f"~/Library/foglang2"
        #linux etc
        else:
            lib_path: str = "/usr/local/lib/foglang2"

        if not filepath:
            filepath = "/usr/local/bin"
        print(f"     Installerar för Linux/MacOS/BSD i {filepath}...")

        print(f"     Installerar Library i {lib_path}")
        subprocess.call(f"mkdir -p {lib_path} && cp -r lib/ {lib_path}", shell=True)

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