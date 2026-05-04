import os
import sys
import ctypes
import subprocess
import platform

def is_admin():
    try:
        is_adm = (os.getuid() == 0)

    except AttributeError:
        is_adm = ctypes.windll.shell32.IsUserAnAdmin() != 0

    return is_adm

def install_foglang_bin(plat):
    print(f"Building Foglang executable for {plat}...")

    abs_path = os.path.abspath(__file__)
    for i in range(1, len(abs_path)-1):
        if abs_path[-i] == '/':
            abs_path = abs_path[:len(abs_path)-i+1]
            break
    abs_path += "main.c"

    if plat == "Linux" or plat == "Darwin" or plat == "FreeBSD":
        build_path = "/usr/local/bin/foglang2"
    elif plat == "Windows":
        abs_path.replace("/", "\\")
        build_path = "C:\\Program Files\\foglang2\\build\\foglang2.exe"

    print(f"     Building to {build_path}...")

    ret_code = subprocess.call(f"gcc -o {build_path} {abs_path} -lm", shell=True)
    if ret_code != 0:
        print("     Build failed, exiting install...")
        sys.exit(1)
    else:
        print("     Build successful!")
    
#os.system(f"sudo mkdir -p {lib_path} && sudo cp -r lib/* {lib_path}")

def install_foglang_lib(plat):

    print(f"Installing Foglang library for {plat}...")
    abs_path = os.path.abspath(__file__)

    for i in range(1, len(abs_path)-1):
        if abs_path[-i] == '/' or abs_path[-i] == '\\':
            abs_path = abs_path[:len(abs_path)-i+1]
            break

    abs_path += "lib/*"

    if plat == "Linux" or plat == "Darwin" or plat == "FreeBSD":
        stdlib_path = "/usr/local/lib/foglang2/"

    elif plat == "Windows":
        stdlib_path = "'C:\\Program Files\\foglang2\\lib'"
        abs_path.replace("/", "\\")

    print(f"     Installing to {stdlib_path}...")
        
    if plat == "Linux" or plat == "Darwin" or plat == "FreeBSD":
        ret_code = subprocess.call(f"mkdir -p {stdlib_path} && cp -r {abs_path} {stdlib_path}", shell=True)

    elif plat == "Windows":
        ret_code = subprocess.call(f"powershell -Command xcopy {abs_path} {stdlib_path} /s /y /i", shell=True)

    if ret_code != 0:
        print("     Library install failed, exiting install...")
        sys.exit(1)
    else:
        print("     Library install successful!")



        

def install_bandvagn(plat):
    print(f"Building Bandvagn executable for {plat}...")

    abs_path = os.path.abspath(__file__)
    for a in range(2):
        for i in range(1, len(abs_path)-1):
            if abs_path[-i] == '/':
                abs_path = abs_path[:len(abs_path)-i]
                break
    abs_path += "/bandvagn/main.c"

    if plat == "Linux" or plat == "Darwin" or plat == "FreeBSD":
        build_path = "/usr/local/bin/foglang2"

    elif plat == "Windows":
        abs_path.replace("/", "\\")
        build_path = "C:\\Program Files\\foglang2\\build\\foglang2.exe"

    print(f"     Building to {build_path}...")

    ret_code = subprocess.call(f"gcc -o {build_path} {abs_path} -lcurl", shell=True)

    if ret_code != 0:
        print("     Build failed, exiting install...")
        sys.exit(1)
    else:
        print("     Build successful!")
    

def create_bandvagn_dir(plat): 
    print(f"Creating Bandvagn package directory for {plat}...")
    
    if plat == "Linux" or plat == "Darwin" or plat == "FreeBSD":
        user = os.environ.get("SUDO_USER")
        if plat == "Linux" or plat == "FreeBSD":
            lib_dir = f"/home/{user}/.local/share/foglang2/packages/"
        elif plat == "Darwin":
            lib_dir = f"/Users/{user}/Library/Application Support/foglang2/packages"
        elif plat == "Windows":
            lib_dir = f"C:\\Program Files\\foglang2\\packages"
        ret_code = 0
        if not os.access(lib_dir, os.F_OK):
            ret_code = subprocess.call(f"mkdir -p {lib_dir}", shell=True)
        if ret_code != 0:
            print("     Creation failed, exiting install...")
            sys.exit(1)
        else:
            print("     Creation successful!")
    elif plat == "Windows":
        pass


def main():
    if not is_admin():
        print("You need to run the installation as root/administrator to install Foglang!")
        sys.exit(1)

    plat = platform.system()

    install_foglang_bin(plat)
    install_foglang_lib(plat)

    install_bandvagn(plat)
    create_bandvagn_dir(plat)

    print("Foglang install successful!")

if __name__ == '__main__':
    main()