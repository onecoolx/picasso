#!/usr/bin/env python

import os
import json
import shutil
from pathlib import Path

def truncate_float(value, decimals=8):
    if isinstance(value, (int, float)):
        str_value = str(value)
        if '.' in str_value:
            integer_part, decimal_part = str_value.split('.')
            if len(decimal_part) > decimals:
                decimal_part = decimal_part[:decimals]
            return float(f"{integer_part}.{decimal_part}")
    return value

def truncate_floats_in_dict(data):
    if isinstance(data, dict):
        return {k: truncate_floats_in_dict(v) for k, v in data.items()}
    elif isinstance(data, list):
        return [truncate_floats_in_dict(item) for item in data]
    elif isinstance(data, (int, float)):
        return truncate_float(data)
    else:
        return data

def process_benchmark_files():
    # Define directory paths
    benchmark_dir = Path("benchmark")
    output_dir = Path("benchmark_update")

    # Ensure output directory exists
    output_dir.mkdir(exist_ok=True)

    # Get all json files
    json_files = list(benchmark_dir.glob("*.json"))

    # Separate baseline files and test files
    base_files = {}
    test_files = {}

    for file in json_files:
        filename = file.stem
        if filename.endswith("_new"):
            base_name = filename[:-4]  # Remove "_new"
            test_files[base_name] = file
        else:
            base_files[filename] = file

    # Process all files with corresponding relationships
    processed_base_names = set()

    # Process each baseline file
    for base_name, base_file in base_files.items():
        processed_base_names.add(base_name)
        test_file = test_files.get(base_name)

        if test_file:
            try:
                # Read both files
                with open(base_file, 'r') as f:
                    base_data = json.load(f)

                with open(test_file, 'r') as f:
                    test_data = json.load(f)

                # Truncate floats in both datasets to 8 decimal places
                base_data = truncate_floats_in_dict(base_data)
                test_data = truncate_floats_in_dict(test_data)

                # Check if any improvements exist
                improvements = False
                for test_name in base_data:
                    if test_name in test_data:
                        if ("mid_ms" in test_data[test_name] and
                            "mid_ms" in base_data[test_name] and
                            test_data[test_name]["mid_ms"] < base_data[test_name]["mid_ms"]):
                            improvements = True
                            break

                if improvements:
                    # Create updated file with improvements
                    output_data = base_data.copy()

                    # Update values where test is better
                    for test_name in base_data:
                        if (test_name in test_data and
                            "mid_ms" in test_data[test_name] and
                            "mid_ms" in base_data[test_name] and
                            test_data[test_name]["mid_ms"] < base_data[test_name]["mid_ms"]):

                            # Update with test values
                            output_data[test_name] = test_data[test_name]

                    # Write output file with formatted JSON
                    output_file = output_dir / f"{base_name}.json"
                    with open(output_file, 'w') as f:
                        json.dump(output_data, f, indent=4)

                    print(f"Processed: {base_name}.json (merged improvements from {base_name}_new.json)")
                else:
                    # No improvements, copy baseline with truncated floats
                    output_file = output_dir / f"{base_name}.json"
                    with open(output_file, 'w') as f:
                        json.dump(base_data, f, indent=4)
                    print(f"Copied: {base_name}.json (no improvements found in {base_name}_new.json)")

            except Exception as e:
                print(f"Error processing files: {base_file}, {test_file}")
                print(f"Error: {e}")
                # Copy baseline as fallback
                output_file = output_dir / f"{base_name}.json"
                shutil.copy2(base_file, output_file)
        else:
            # No test file, copy baseline with truncated floats
            output_file = output_dir / f"{base_name}.json"
            with open(base_file, 'r') as f:
                base_data = json.load(f)

            # Truncate floats
            base_data = truncate_floats_in_dict(base_data)

            with open(output_file, 'w') as f:
                json.dump(base_data, f, indent=4)
            print(f"Copied: {base_name}.json (no corresponding test file)")

    # Process test files without baseline
    for base_name, test_file in test_files.items():
        if base_name not in processed_base_names:
            try:
                # Read test file
                with open(test_file, 'r') as f:
                    test_data = json.load(f)

                # Truncate floats
                test_data = truncate_floats_in_dict(test_data)

                # Write as new baseline
                output_file = output_dir / f"{base_name}.json"
                with open(output_file, 'w') as f:
                    json.dump(test_data, f, indent=4)

                print(f"Created: {base_name}.json (using {base_name}_new.json as new baseline)")

            except Exception as e:
                print(f"Error processing file: {test_file}")
                print(f"Error: {e}")

    print(f"\nAll files processed! Results saved in {output_dir} directory")

def main():
    # Check if benchmark directory exists
    if not Path("benchmark").exists():
        print("Error: benchmark directory not found")
        return

    process_benchmark_files()

if __name__ == "__main__":
    main()
