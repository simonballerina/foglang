import os
import sys
import subprocess
import platform

def main():


    name = os.name

    filepath = input("  Var vill du lägga installationen? [sökväg] (enter för default): ")
    bs = "\\" 

    #windows
    print("name: ", name)
    if name == "nt":

        lib_path: str = "'C:\\Program Files\\foglang2\\lib'"

        if not filepath:
            filepath = "C:\\Program Files\\foglang2\\build"

        filepath = filepath.replace("/", "\\")
        print(f"     Installerar för Windows i {filepath}...")

        print(f"     Installerar Library i {lib_path}")
        subprocess.call(f"powershell -Command xcopy lib {lib_path} /s /y /i", shell=True)
        print("TODO: Lägg in följande miljövariabel \"C:\\Program Files\\foglang2\\build\"")
        status1 = subprocess.call(f"gcc -o {filepath+bs+'foglang2.exe'} main.c -lm")
        exit_code = os.WEXITSTATUS(status1)

        if exit_code != 0:
            print("     Kompilering misslyckades, installation avbruten")
            sys.exit(-1)

        print(f"     Installerar Bandvagn package manager i {filepath}...")
        os.chdir("../bandvagn")
        status1 = subprocess.call(f"gcc -o {filepath+bs+'vagn.exe'} main.c -lcurl")
        if exit_code != 0:
            print("     Kompilering misslyckades, installation avbruten")
            exit(-1)

    #unix
    elif name == "posix":
        #mac
        if platform.system() == "Darwin":
            user = os.environ.get("USER")
            lib_path: str = "/usr/local/lib/foglang2"
            bandvagn_path: str = f"/Users/{user}/Library/foglang2/packages"
        #linux etc
        else:
            user = os.environ.get("USER")
            if not os.access(f"/home/{user}/.local/lib/foglang2/packages", os.R_OK):
                os.mkdir(f"/home/{user}/.local/lib/foglang2/packages")
            lib_path: str = "/usr/local/lib/foglang2"
            bandvagn_path = f"/home/{user}/.local/lib/foglang2/packages"

        if not filepath:
            filepath = "/usr/local/bin"
        print(f"     Installerar för Linux/MacOS/BSD i {filepath}...")

        print(f"     Installerar Library i {lib_path}")
        os.system(f"sudo mkdir -p {lib_path} && sudo cp -r lib/* {lib_path}")
        filepath_bin = filepath+"/foglang2"

        os.system(f"sudo make clean TARGET={filepath_bin}")

        os.system(f"sudo make TARGET={filepath_bin}")




        print(f"     Installerar Bandvagn package manager i {filepath}...")
        os.chdir("../bandvagn")

        os.system(f"mkdir -p {bandvagn_path}")


        filepath_vagn = filepath+"/vagn"
        os.system(f"sudo make clean TARGET={filepath_vagn}")
        os.system(f"sudo make TARGET={filepath_vagn}")
 




    else:
        print("Okänt operativsystem, installation misslyckades")
        sys.exit(-1)
    
    print("     Foglang2 installerat!")

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n     Avbröt installationen")
        sys.exit(0)
