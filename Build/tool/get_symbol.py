import subprocess
import re
import sys

def get_symbol_offset(symbol_name, objdump_file_path):
    try:
        with open(objdump_file_path, 'r') as f:
            objdump_output = f.read()
    except FileNotFoundError:
        print(f"Error: File not found: {objdump_file_path}")
        return None

    pattern = re.compile(r"^([0-9a-f]+)\s+.*?\s+" + re.escape(symbol_name) + "$", re.MULTILINE)


    match = pattern.search(objdump_output)

    if match:
        return match.group(1)  # Return the offset (group 1)
    else:
        print(f"Symbol '{symbol_name}' not found in {objdump_file_path}")
        return None


def main():
    """
    Example usage:  Gets the offset of a symbol from a file.
    """
    symbol_name = sys.argv[1]
    objdump_file_path = sys.argv[2]

    offset = get_symbol_offset(symbol_name, objdump_file_path)
   
    print(f"0x{offset}")


if __name__ == "__main__":
    main()





