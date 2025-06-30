import argparse
from pathlib import Path
import csv

def prRed(skk): print("\033[91m {}\033[00m".format(skk))

def process_injection(source_path, dest_path, offset_hex, maxsize_hex, output_path=None):
    try:
        # Remove "0x" prefix if present and convert to int
        offset = int(offset_hex.lower().replace('0x', ''), 16)
        maxsize = int(maxsize_hex.lower().replace('0x', ''), 16)
       
        source = source_path.read_bytes()
        if len(source) > maxsize:
            prRed(f"File {source_path} exceeds max size! ({len(source):X} > {maxsize:X})")
            return 1

        dest = bytearray(dest_path.read_bytes())
        end_pos = offset + len(source)
        
        if end_pos > len(dest):
            prRed(f"Max size exceeded for {source_path}! (End position {end_pos:X} > file size {len(dest):X})")
            return 1
            
        dest[offset:end_pos] = source

        output_path = output_path or dest_path
        output_path.write_bytes(dest)
        print(f"Injected {source_path} into {output_path} at 0x{offset:X} (size: 0x{len(source):X})")
        return 0
        
    except Exception as e:
        prRed(f"Error processing {source_path}: {str(e)}")
        return 1

def main(argv=None):
    parser = argparse.ArgumentParser(description='Binary file injection tool')
    parser.add_argument('source', type=Path, help='file to insert or CSV list of injections')
    parser.add_argument('dest', type=Path, nargs='?', help='file to insert into (when not using CSV)')
    parser.add_argument('offset', nargs='?', help='offset in hex (with or without 0x)')
    parser.add_argument('maxsize', nargs='?', help='max size in hex (with or without 0x)')
    parser.add_argument('output', type=Path, nargs='?',
                       help='output file (default: overwrite dest)')
                       
    args = parser.parse_args(argv)

    # CSV batch processing mode
    if args.source.suffix.lower() == '.csv':
        if not all(v is None for v in [args.dest, args.offset, args.output, args.maxsize]):
            prRed("When using CSV file, only the SOURCE argument should be provided.")
            return 1

        try:
            with open(args.source, 'r') as f:
                csv_reader = csv.reader(f, quotechar='"', skipinitialspace=True)
                total_errors = 0
                processed = 0
                
                for row in csv_reader:
                    if not row:  # Skip empty lines
                        continue
                        
                    clean_row = [field.strip().strip('"') for field in row if field.strip()]
                    
                    if len(clean_row) < 4:
                        prRed(f"Skipping invalid row (expected 4-5 fields): {clean_row}")
                        continue
                        
                    source_path = Path(clean_row[0])
                    dest_path = Path(clean_row[1])
                    offset_hex = clean_row[2]
                    maxsize_hex = clean_row[3]
                    output_path = Path(clean_row[4]) if len(clean_row) > 4 else None
                    
                    result = process_injection(
                        source_path, dest_path, offset_hex, maxsize_hex, output_path
                    )
                    total_errors += result
                    processed += 1
                
                print(f"Processed {processed} injections with {total_errors} errors")
                return 0 if total_errors == 0 else 1
                
        except Exception as e:
            prRed(f"Error processing CSV file: {str(e)}")
            return 1
    else:
        if None in [args.dest, args.offset, args.maxsize]:
            prRed("For single file mode, all arguments are required: source dest offset maxsize [output].")
            return 1
            
        return process_injection(
            args.source, args.dest, args.offset, args.maxsize, args.output
        )

if __name__ == '__main__':
    raise SystemExit(main())
