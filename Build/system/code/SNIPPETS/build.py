#!/usr/bin/env python3

import os
import sys
import subprocess
import shutil

def flush_output():
    """Flush both stdout and stderr to ensure proper ordering"""
    sys.stdout.flush()
    sys.stderr.flush()

def print_with_flush(*args, **kwargs):
    """Print and immediately flush output"""
    print(*args, **kwargs)
    flush_output()

def main():
    # Check for -c flag
    clean_flag = len(sys.argv) > 1 and sys.argv[1] == "-c"

    processes = []
    
    print_with_flush(f"Building snippets...\n")

    # Iterate over all subdirectories
    for entry in os.scandir("."):
        if entry.is_dir():
            makefile_path = os.path.join(entry.path, "Makefile")
            if not os.path.isfile(makefile_path):
                continue  # skip directories without a Makefile

            if clean_flag:
                cmd = "make clean && make -j"
            else:
                cmd = "make -j"
                
            # Start process in background
            proc = subprocess.Popen(
                cmd,
                cwd=entry.path,
                shell=True
            )
            processes.append(proc)

    # Wait for all builds to finish
    for proc in processes:
        proc.wait()

    # Choose Python command
    if shutil.which("py"):
        python_cmd = ["py", "-3"]
    else:
        python_cmd = ["python3"]

    # Run inject_file.py
    inject_cmd = python_cmd + [
        "../../../tool/inject_file.py",
        "inject.csv"
    ]

    subprocess.run(inject_cmd, check=True)

if __name__ == "__main__":
    main()
