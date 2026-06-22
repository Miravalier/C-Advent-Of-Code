import argparse
import datetime
import os
import secrets
import subprocess
from pathlib import Path
from watchfiles import watch


def find_files_with_extension(path: Path, extension: str) -> list[Path]:
    results = []
    for subpath in path.iterdir():
        if subpath.is_dir():
            results.extend(find_files_with_extension(subpath, extension))
        elif subpath.suffix == extension:
            results.append(subpath)
    return results


def run_code():
    program_name = f"/tmp/{secrets.token_hex(12)}"
    c_source = [str(source) for source in find_files_with_extension(Path("src"), ".c")]
    try:
        print("[{}] Compiling Code".format(datetime.datetime.now().strftime("%H:%M:%S")))
        subprocess.run(["clang", "-I", "src", *c_source, "-lm", "-o", program_name], check=True)
        print()

        print("[{}] Running Code".format(datetime.datetime.now().strftime("%H:%M:%S")))
        result = subprocess.run([program_name], stderr=subprocess.STDOUT)

        print("\n[{}] Done - Return Code {}".format(datetime.datetime.now().strftime("%H:%M:%S"), result.returncode))
        os.unlink(program_name)
    except subprocess.CalledProcessError:
        print()


def main():
    parser = argparse.ArgumentParser()
    args = parser.parse_args()

    subprocess.run(["tput", "reset"])
    run_code()
    for _ in watch('.'):
        run_code()


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print()
