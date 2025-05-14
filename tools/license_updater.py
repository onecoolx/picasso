#!/usr/bin/env python

import re
import os
import argparse
from datetime import datetime
import codecs

def replace_license_in_file(file_path, new_license):
    """Replace license information for code"""
    try:
        with open(file_path, 'rb') as f:
            raw_content = f.read()
        
        # check UTF-8 BOM
        if raw_content.startswith(codecs.BOM_UTF8):
            encoding = 'utf-8-sig'
            content = raw_content[3:].decode('utf-8')
        else:
            encoding = 'utf-8'
            content = raw_content.decode('utf-8')

        modified = False
        pattern = re.compile(r'^\s*(#ifndef|#include)', re.MULTILINE)
        match = pattern.search(content)
        
        if match:
            # replace (/* ... */)
            header_end = match.start()
            header = content[:header_end].rstrip()
            match_content = match.group().lstrip()
            new_content = new_license + '\n' + match_content + content[match.end():]
            modified = True

        if modified:
            # restore BOM
            new_content_bytes = new_content.encode('utf-8')
            with open(file_path, 'wb') as f:
                if encoding == 'utf-8-sig':
                    f.write(codecs.BOM_UTF8)
                f.write(new_content_bytes)
            print(f'Update success: {file_path}')
        else:
            print(f'No need modify: {file_path}')

    except Exception as e:
        print(f'Process {file_path} fail: {str(e)}')

def main():
    parser = argparse.ArgumentParser(
        description='Replace C/C++ code license'
    )
    parser.add_argument(
        'license_file',
        help='license template (use %year% for current year)'
    )
    parser.add_argument(
        'target',
        help='target file or directory'
    )
    parser.add_argument(
        '--year',
        type=int,
        help='optional: specify the year to replace %year%'
    )
    args = parser.parse_args()

    # read license template content
    with open(args.license_file, 'r', encoding='utf-8') as f:
        license_template = f.read()
    
    year = args.year if args.year is not None else datetime.now().year
    new_license = license_template.replace('%year%', str(year))

    # process dir
    if os.path.isfile(args.target):
        if os.path.basename(args.target).lower() != 'resource.h':
            replace_license_in_file(args.target, new_license)
        else:
            print(f'Ignore: {args.target}')
    elif os.path.isdir(args.target):
        for root, _, files in os.walk(args.target):
            for file in files:
                if file.lower().endswith(('.h', '.c', '.cpp', '.hpp', '.cc', '.cxx', '.hh', '.m', '.mm')):
                    if file.lower() != 'resource.h':
                        file_path = os.path.join(root, file)
                        replace_license_in_file(file_path, new_license)
                    else:
                        print(f'Ignore: {os.path.join(root, file)}')
    else:
        print('error：no file or dir')
        return

if __name__ == '__main__':
    main()
