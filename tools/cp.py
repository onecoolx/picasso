#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Copy a file.

This module works much like the cp posix command - it takes 2 arguments:
(src, dst) and copies the file with path |src| to |dst|.
"""

import shutil
import sys
import os


def Main(src, dst):
  # Use copy instead of copyfile to ensure the executable bit is copied.
  path = os.path.dirname(dst)
  is_exit = os.path.exists(path)
  if not is_exit:
      os.makedirs(path)
  if os.path.isdir(src):
      if os.path.exists(dst):
          shutil.rmtree(dst)
      shutil.copytree(src, dst)
      return 0
  else:
      shutil.copy(src, dst)
      return 0

if __name__ == '__main__':
  sys.exit(Main(sys.argv[1], sys.argv[2]))
