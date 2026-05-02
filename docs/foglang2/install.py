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
        status1 = subprocess.call(f"gcc -o {filepath+'\\foglang2.exe'} main.c -lm")
        exit_code = os.WEXITSTATUS(status1)

        if exit_code != 0:
            print("     Kompilering misslyckades, installation avbruten")
            exit(-1)

        print(f"     Installerar Bandvagn package manager i {filepath}...")
        os.chdir("../bandvagn")
        status1 = subprocess.call(f"gcc -o {filepath+'\\vagn.exe'} main.c -lcurl")
        if exit_code != 0:
            print("     Kompilering misslyckades, installation avbruten")
            exit(-1)

    #unix
    elif name == "posix":
        #mac
        if platform.system() == "Darwin":
            user = os.environ.get("SUDO_USER")
            if not user:
                user = os.environ.get("USER")
            lib_path: str = f"/Users/{user}/Library/foglang2"
        #linux etc
        else:
            lib_path: str = "/usr/local/lib/foglang2"

        if not filepath:
            filepath = "/usr/local/bin"
        print(f"     Installerar för Linux/MacOS/BSD i {filepath}...")

        print(f"     Installerar Library i {lib_path}")
        subprocess.call(f"cp -u -r lib {lib_path}", shell=True)

        filepath_bin = filepath+"/foglang2"

        status1 = subprocess.run(["make", "clean", f"TARGET={filepath_bin}"]).returncode
        exit_code1 = os.WEXITSTATUS(status1)

        if exit_code1 != 0:
            print("     Rensning misslyckades, installation avbruten")
            exit(-1)

        status2 = subprocess.run(["make", f"TARGET={filepath_bin}"]).returncode
        exit_code2 = os.WEXITSTATUS(status2)

        if exit_code2 != 0:
            print("     Kompilering misslyckades, installation avbruten")
            exit(-1)




        print(f"     Installerar Bandvagn package manager i {filepath}...")
        os.chdir("../bandvagn")

        filepath_vagn = filepath+"/vagn"
        status1 = subprocess.run(["make", "clean", f"TARGET={filepath_vagn}"]).returncode
        exit_code1 = os.WEXITSTATUS(status1)

        if exit_code1 != 0:
            print("     Rensning misslyckades, installation avbruten")
            exit(-1)

        status2 = subprocess.run(["make", f"TARGET={filepath_vagn}"]).returncode
        exit_code2 = os.WEXITSTATUS(status2)

        if exit_code2 != 0:
            print("     Kompilering misslyckades, installation avbruten")
            exit(-1)
        if not os.access(f"/home/{os.getenv("SUDO_USER")}/.local/lib/foglang2/packages", os.R_OK):
            os.mkdir(f"/home/{os.getenv("SUDO_USER")}/.local/lib/foglang2/packages")



    else:
        print("Okänt operativsystem, installation misslyckades")
        exit(-1)
    
    print("     Foglang2 installerat!")

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n     Avbröt installationen")
        exit(0)
