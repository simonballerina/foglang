import sys
import subprocess
import pathlib
import os

fail_info = []

debug = False

def test_program(program: str, interpreter: str, valid_output: str):
    global debug
    global fail_info

    code = subprocess.run([interpreter, f"foglang-test/{program}.fg"], capture_output=True)
    exit_code = code.returncode
    err = code.stderr
    out = code.stdout

    ret_out = err+out

    if debug:
        print(code.stderr+code.stdout)

    out_ok = ret_out == valid_output
    exit_ok = exit_code == 0
    title = f"{(out_ok and exit_ok)*"PASS"+((not out_ok) or (not exit_ok))*"FAIL"}"
    output = f"{(out_ok)*"VALID"+(not out_ok)*"INVALID"}"


    log = f"{title}, exit code: {exit_code}, output: {output}"

    if not out_ok or not exit_ok:
        fail_info.append({"program": program, "exit": exit_code, "out": out.decode(), "err": err.decode()})

    return log









    





def main():

    global fail_info
    argv = sys.argv

    path = pathlib.Path(__file__).parent.resolve()
    os.chdir(path)
    os.chdir("../")

    interpreter = "build/foglang2"
    if len(argv) > 1:
        interpreter = argv[1]

    print(f"Testing using interpreter '{interpreter}'...\n")

    programs = {"foug": b"""\x1b[31mH\x1b[0m\x1b[31me\x1b[0m\x1b[31ml\x1b[0m\x1b[31ml\x1b[0m\x1b[31mo\x1b[0m\x1b[31m,\x1b[0m\x1b[31m \x1b[0m\x1b[31mw\x1b[0m\x1b[31mo\x1b[0m\x1b[31mr\x1b[0m\x1b[31ml\x1b[0m\x1b[31md\x1b[0m\x1b[31m!\x1b[0m\n\x1b[31mH\x1b[0m\x1b[31me\x1b[0m\x1b[31ml\x1b[0m\x1b[31ml\x1b[0m\x1b[31mo\x1b[0m\x1b[31m,\x1b[0m\x1b[31m \x1b[0m\x1b[31mw\x1b[0m\x1b[31mo\x1b[0m\x1b[31mr\x1b[0m\x1b[31ml\x1b[0m\x1b[31md\x1b[0m\x1b[31m \x1b[0m\x1b[31m1\x1b[0m\x1b[31m!\x1b[0m\nHello, world!\nHello, world 1!\n""", 
                "band": b"""1\n\xc3\xa4pple\n[1, 2, "\xc3\xa4pple", "Hello, world!", ["a", "b", [1, 2, 3], "c"]]\n2\n\xc3\xa4pple och p\xc3\xa4ron\n["test", 2, "\xc3\xa4pple", "Hello, world!", ["a", "b", "svets!", "c"]]\n\xc3\xa4pple och p\xc3\xa4ron\n1\n\xc3\xa4pple och p\xc3\xa4ron\n""", 
                "boul": b"""Before\nFunction called\nAfter\n3\n["Ok", "b"]\n[["Ok", "b"], 2, 3, "str"]\n[["Ok", "b"], 2, 3, "str"]\n823\n3628800\n112\nab\n"""}


    logs = {program: "" for program in programs.keys()}


    for program in programs.keys():
        log = test_program(program, interpreter, programs[program])
        logs[program] = log

    pass_amount = 0

    for log in logs.keys():
        if logs[log].lower().startswith("pass"):
            pass_amount+=1
        print(f"{log}: {logs[log]}")

    print()
    if len(fail_info):
        print("Printing info on failed tests...") #¤ vera was here 2026:3
    

    for fail in fail_info:
        print()
        for cat in fail.keys():
            print(f"{cat}: '{fail[cat]}'")
        print()
    
    print(f"Passed {pass_amount}/{len(programs)} tests, result: {(pass_amount==len(programs))*"PASS"+(not (pass_amount==len(programs)))*"FAIL"}")

main()