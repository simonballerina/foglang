import os
import sys
import ctypes
import subprocess
import platform
import pwd

def is_admin():
    try:
        is_adm = (os.getuid() == 0)

    except AttributeError:
        is_adm = ctypes.windll.shell32.IsUserAnAdmin() != 0

    return is_adm

def get_version():
    try:
        with open("version.txt", "r", encoding="utf-8") as file:
            return file.read()
    except:
        return ""

def install_foglang_bin(plat, library, packages):
    print(f"Building Foglang executable for {plat}...")

    abs_path = os.path.abspath(__file__)
    for i in range(1, len(abs_path)-1):
        if abs_path[-i] == '/':
            abs_path = abs_path[:len(abs_path)-i+1]
            break
    abs_path += "main.c"
    build_path = ""
    if plat == "Linux" or plat == "Darwin" or plat == "FreeBSD":
        build_path = "/usr/local/bin/foglang2"
    elif plat == "Windows":
        abs_path.replace("/", "\\")
        build_path = "C:\\Program Files\\foglang2\\build\\foglang2.exe"

    print(f"     Building to {build_path}...")
    version = get_version()
    version_text =  f'-D VERSION="{version}"'*bool(version)
    # includes slash if not already included 
    lib_text =  (f'-D LIBPATH="{library}{"/"*bool(library[-1]!="/" and not plat == "Windows")}{"\\"*bool(library[-1]!="\\" and plat == "Windows")}"')*bool(library)
    pack_text =  (f'-D PACKPATH="{packages}{"/"*bool(packages[-1]!="/" and not plat == "Windows")}{"\\"*bool(packages[-1]!="\\" and plat == "Windows")}"')*bool(packages)
    ret_code = subprocess.call(f"gcc -o {build_path} {abs_path} -lm '{version_text}' '{lib_text}' '{pack_text}'", shell=True)
    if ret_code != 0:
        print("     Build failed, exiting install...")
        sys.exit(1)
    else:
        print("     Build successful!")
    

def install_foglang_lib(plat, is_custom) -> str:

    print(f"Installing Foglang library for {plat}...")
    abs_path = os.path.abspath(__file__)

    for i in range(1, len(abs_path)-1):
        if abs_path[-i] == '/' or abs_path[-i] == '\\':
            abs_path = abs_path[:len(abs_path)-i+1]
            break

    abs_path += "lib/*"
    stdlib_path = ""
    if is_custom:
        stdlib_path += input("Enter library path: ")

    if not stdlib_path:
        if plat == "Linux" or plat == "Darwin" or plat == "FreeBSD":
            stdlib_path = "/usr/local/lib/foglang2/"
        elif plat == "Windows":
            stdlib_path = "'C:\\Program Files\\foglang2\\lib'"
            abs_path.replace("/", "\\")

    print(f"     Installing to {stdlib_path}...")

    ret_code = ""
        
    if plat == "Linux" or plat == "Darwin" or plat == "FreeBSD":
        ret_code = subprocess.call(f"mkdir -p {stdlib_path} && cp -r {abs_path} {stdlib_path}", shell=True)

    elif plat == "Windows":
        ret_code = subprocess.call(f"powershell -Command xcopy {abs_path} {stdlib_path} /s /y /i", shell=True)

    if ret_code != 0:
        print("     Library install failed, exiting install...")
        sys.exit(1)
    else:
        print("     Library install successful!")

    return stdlib_path



        

def install_bandvagn(plat):
    print(f"Building Bandvagn executable for {plat}...")

    abs_path = os.path.abspath(__file__)
    for _ in range(2):
        for i in range(1, len(abs_path)-1):
            if abs_path[-i] == '/':
                abs_path = abs_path[:len(abs_path)-i]
                break
    abs_path += "/bandvagn/main.c"
    build_path = ""

    if plat == "Linux" or plat == "Darwin" or plat == "FreeBSD":
        build_path = "/usr/local/bin/vagn"

    elif plat == "Windows":
        abs_path.replace("/", "\\")
        build_path = "C:\\Program Files\\foglang2\\build\\vagn.exe"

    print(f"     Building to {build_path}...")

    ret_code = subprocess.call(f"gcc -o {build_path} {abs_path} -lcurl", shell=True)

    if ret_code != 0:
        print("     Build failed, exiting install...")
        sys.exit(1)
    else:
        print("     Build successful!")
    

def chown_recursive(path, uid, gid):
    for root, dirs, files in os.walk(path):
        os.chown(root, uid, gid)
        for d in dirs:
            os.chown(os.path.join(root, d), uid, gid)
        for f in files:
            os.chown(os.path.join(root, f), uid, gid)

def create_bandvagn_dir(plat, is_custom) -> str: 
    print(f"Creating Bandvagn package directory for {plat}...")

    user = os.environ.get("SUDO_USER")
    lib_dir = ""
    if is_custom:
        lib_dir += input("Enter bandvagn package path: ")
    if not lib_dir:
        if plat == "Linux" or plat == "FreeBSD":
            lib_dir = f"/home/{user}/.local/share/foglang2/packages/"
        elif plat == "Darwin":
            lib_dir = os.path.join(f"/Users/{user}/", "Library/Application Support/foglang2/packages/")
        elif plat == "Windows":
            lib_dir = "C:\\Program Files\\foglang2\\packages"
    ret_code = 0
    if not os.access(lib_dir, os.F_OK):
        ret_code = subprocess.run(["mkdir", "-p", lib_dir], check=True).returncode

    # change permissions for the folders
    chown_recursive(lib_dir, pwd.getpwnam(user).pw_uid, pwd.getpwnam(user).pw_gid)
    if ret_code != 0:
        print("     Creation failed, exiting install...")
        sys.exit(1)
    else:
        print("     Creation successful!")

    return lib_dir




def main():
    if not is_admin():
        print("You need to run the installation as root/administrator to install Foglang!")
        sys.exit(1)

    plat = platform.system()

    is_custom = "y" in input("Do you want custom library and package installation paths [y/N]? ").lower()
    
    lib_path = install_foglang_lib(plat, is_custom)

    install_bandvagn(plat)
    pack_path = create_bandvagn_dir(plat, is_custom)

    install_foglang_bin(plat, lib_path, pack_path)

    print("Foglang install successful!")

if __name__ == '__main__':
    main()
