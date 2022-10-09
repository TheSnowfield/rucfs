#!/usr/bin/python3

import os
import sys
from lib.rucfs_writer import rucfs_writer

inode_table = 0x20
data_table = 0x20
string_table = 0

def main():
  
  # Check for correct number of arguments
  if len(sys.argv) != 3:
    print("Usage: mkfs.rucfs.py <directory> <output>")
    sys.exit(1)

  # create output file
  path = sys.argv[1]
  if path[-1] != "/": path += "/"
  output = sys.argv[2]

  # listdir and write inode table
  writer = rucfs_writer(output)
  writer.write_for_dir(path, 0)
  packdir(path, writer)

  # write data table
  global names, files, data_table, string_table
  for filepath in writer.files:
    fd = open(filepath, "rb")
    writer.write_binary(fd.read())
  
  # write name table
  names_buffer = writer.names.encode("utf8")
  writer.write_binary(names_buffer)

  # commit
  data_table += writer.inode_offset
  string_table += data_table + writer.data_offset
  writer.commit(data_table, string_table)

def get_item_count_recursively(path):
  dirs = os.listdir(path)
  item_count = len(dirs)
  for file in dirs:
    currpath = os.path.join(path, file)
    if os.path.isdir(currpath):
      item_count += get_item_count_recursively(currpath)
  return item_count

def packdir(path: str, ruc: rucfs_writer) -> None:
  dirlist = []
  dirs = os.listdir(path)
  pre_item_count = len(dirs) - 1
  for file in dirs:
    currpath = os.path.join(path, file)
    if os.path.isdir(currpath):
      dirlist.append(currpath)
      ruc.write_for_dir(currpath, pre_item_count)
      pre_item_count += get_item_count_recursively(currpath)
    else:
      ruc.write_for_file(currpath)
    pre_item_count -= 1
  for dir in dirlist:
    packdir(dir, ruc)

if __name__ == "__main__":
  main()
