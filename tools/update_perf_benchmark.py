#!/usr/bin/env python
"""
Performance Benchmark Update Tool

Features:
1. Read baseline files and _new.json test files from benchmark/ directory
2. Compare performance data and update baseline if improvements found (smaller mid_ms)
3. Automatically add new statistical fields (std_dev, cv, iterations, total_time_ms)
4. Output updated files to benchmark_update/ directory

Supported Fields:
- mid_ms: Median time (milliseconds)
- avg_ms: Average time (milliseconds)
- min_ms: Minimum time (milliseconds)
- max_ms: Maximum time (milliseconds)
- std_dev: Standard deviation (new)
- cv: Coefficient of variation (new)
- iterations: Number of iterations (new)
- total_time_ms: Total test time (new)

Usage:
    cd <build_dir>
    python3 ../tools/update_perf_benchmark.py
"""

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

def check_new_fields(data):
    """Check if data contains new statistical fields"""
    new_fields = ['std_dev', 'cv', 'iterations', 'total_time_ms']
    
    for test_name, metrics in data.items():
        if isinstance(metrics, dict):
            for field in new_fields:
                if field in metrics:
                    return True
    return False

def get_field_coverage(data):
    """Get coverage statistics for new fields"""
    new_fields = ['std_dev', 'cv', 'iterations', 'total_time_ms']
    total_tests = len(data)
    
    if total_tests == 0:
        return {}
    
    coverage = {}
    for field in new_fields:
        count = sum(1 for metrics in data.values() 
                   if isinstance(metrics, dict) and field in metrics)
        coverage[field] = (count, total_tests, count / total_tests * 100)
    
    return coverage

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

    # Statistics
    stats = {
        'total_processed': 0,
        'with_improvements': 0,
        'with_new_fields': 0,
        'no_changes': 0,
        'new_baselines': 0
    }

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
                improvement_details = []
                
                for test_name in base_data:
                    if test_name in test_data:
                        if ("mid_ms" in test_data[test_name] and
                            "mid_ms" in base_data[test_name] and
                            test_data[test_name]["mid_ms"] < base_data[test_name]["mid_ms"]):
                            improvements = True
                            old_val = base_data[test_name]["mid_ms"]
                            new_val = test_data[test_name]["mid_ms"]
                            improvement_pct = ((old_val - new_val) / old_val) * 100
                            improvement_details.append(f"  - {test_name}: {old_val:.6f} → {new_val:.6f} (improved {improvement_pct:.1f}%)")

                if improvements:
                    # Create updated file with improvements
                    output_data = base_data.copy()

                    # Update values where test is better
                    for test_name in base_data:
                        if (test_name in test_data and
                            "mid_ms" in test_data[test_name] and
                            "mid_ms" in base_data[test_name] and
                            test_data[test_name]["mid_ms"] < base_data[test_name]["mid_ms"]):

                            # Update with all test values including new fields
                            # (std_dev, cv, iterations, total_time_ms)
                            output_data[test_name] = test_data[test_name]

                    # Write output file with formatted JSON
                    output_file = output_dir / f"{base_name}.json"
                    with open(output_file, 'w') as f:
                        json.dump(output_data, f, indent=4)

                    print(f"✓ Processed: {base_name}.json (merged {len(improvement_details)} improvements)")
                    for detail in improvement_details:
                        print(detail)
                    
                    stats['total_processed'] += 1
                    stats['with_improvements'] += 1
                else:
                    # No improvements, but update with new fields if they exist
                    output_data = base_data.copy()
                    updated_fields = False
                    
                    for test_name in base_data:
                        if test_name in test_data:
                            # Check if new fields exist in test data but not in baseline
                            new_fields = ['std_dev', 'cv', 'iterations', 'total_time_ms']
                            has_new_fields = any(field in test_data[test_name] and 
                                                field not in base_data[test_name] 
                                                for field in new_fields)
                            
                            if has_new_fields:
                                # Merge new fields into baseline data
                                output_data[test_name] = {**base_data[test_name], **test_data[test_name]}
                                updated_fields = True
                    
                    output_file = output_dir / f"{base_name}.json"
                    with open(output_file, 'w') as f:
                        json.dump(output_data, f, indent=4)
                    
                    stats['total_processed'] += 1
                    if updated_fields:
                        print(f"✓ Updated: {base_name}.json (added new statistical fields)")
                        stats['with_new_fields'] += 1
                    else:
                        print(f"  Copied: {base_name}.json (no improvements)")
                        stats['no_changes'] += 1

            except Exception as e:
                print(f"Error processing files: {base_file}, {test_file}")
                print(f"Error: {e}")
                # Copy baseline as fallback
                output_file = output_dir / f"{base_name}.json"
                shutil.copy2(base_file, output_file)
                stats['total_processed'] += 1
                stats['no_changes'] += 1
        else:
            # No test file, copy baseline with truncated floats
            output_file = output_dir / f"{base_name}.json"
            with open(base_file, 'r') as f:
                base_data = json.load(f)

            # Truncate floats
            base_data = truncate_floats_in_dict(base_data)

            with open(output_file, 'w') as f:
                json.dump(base_data, f, indent=4)
            print(f"  Copied: {base_name}.json (no corresponding test file)")
            stats['total_processed'] += 1
            stats['no_changes'] += 1

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

                print(f"✓ Created: {base_name}.json (using {base_name}_new.json as new baseline)")
                stats['total_processed'] += 1
                stats['new_baselines'] += 1

            except Exception as e:
                print(f"Error processing file: {test_file}")
                print(f"Error: {e}")

    # Print summary
    print("\n" + "=" * 60)
    print("Summary")
    print("=" * 60)
    print(f"Total files processed: {stats['total_processed']}")
    print(f"  - With performance improvements: {stats['with_improvements']}")
    print(f"  - With new fields added: {stats['with_new_fields']}")
    print(f"  - No changes: {stats['no_changes']}")
    print(f"  - New baselines created: {stats['new_baselines']}")
    print(f"\nAll files processed! Results saved in {output_dir} directory")
    print("=" * 60)

def main():
    # Check if benchmark directory exists
    if not Path("benchmark").exists():
        print("Error: benchmark directory not found")
        return

    process_benchmark_files()

if __name__ == "__main__":
    main()
