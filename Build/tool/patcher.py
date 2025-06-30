import sys
import os

def patch(input_file, patch_file, output_file):
    try:
        with open(input_file, 'rb') as f:
            data = bytearray(f.read())  # Read the entire file into a bytearray
    except FileNotFoundError:
        print(f"Error: Input file '{input_file}' not found.")
        return
    except Exception as e:
        print(f"Error reading input file: {e}")
        return

    try:
        with open(patch_file, 'r') as f:
            for line in f:
                line = line.strip()
                if line.startswith('#') or not line:  # Skip comments and empty lines
                    continue

                try:
                    address_str, value = line.split(',', 1)  # Split only once
                    address = int(address_str, 16)  # Convert hex address to integer

                    if value.startswith('"') and value.endswith('"'):
                        # It's a file path
                        file_path = value[1:-1]  # Remove quotes
                        try:
                            with open(file_path, 'rb') as file_to_insert:
                                patch_bytes = file_to_insert.read()
                        except FileNotFoundError:
                            print(f"Error: File '{file_path}' not found. Skipping.")
                            continue
                        except Exception as e:
                            print(f"Error reading file '{file_path}': {e}. Skipping.")
                            continue
                    else:
                        # It's a hex string
                        patch_bytes = bytes.fromhex(value)  # Convert hex string to bytes

                    # Check if the address is within the bounds of the data
                    if address < 0 or address + len(patch_bytes) > len(data):
                        print(f"Warning: Address 0x{address:X} out of bounds. Skipping.")
                        continue

                    # Apply the patch
                    data[address:address + len(patch_bytes)] = patch_bytes

                except ValueError:
                    print(f"Error: Invalid format in patch file: '{line}'. Skipping.")
                except Exception as e:
                    print(f"Error processing line '{line}': {e}. Skipping.")

    except FileNotFoundError:
        print(f"Error: Patch file '{patch_file}' not found.")
        return
    except Exception as e:
        print(f"Error reading patch file: {e}")
        return

    try:
        with open(output_file, 'wb') as f:
            f.write(bytes(data))  # Write the modified bytearray to the output file
        print(f"Successfully patched '{input_file}' and saved to '{output_file}'.")

    except Exception as e:
        print(f"Error writing to output file: {e}")
        return


if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: python patcher.py <input_file> <patch_file> <output_file>")
    else:
        input_file = sys.argv[1]
        patch_file = sys.argv[2]
        output_file = sys.argv[3]
        patch(input_file, patch_file, output_file)
