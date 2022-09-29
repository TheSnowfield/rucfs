#!/usr/bin/python3

import os
import sys
from lib.rucfs_writer import rucfs_writer

inode_offset = 0
names = ""
name_offset = 0
files = []
data_offset = 0
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
  pack(path, writer)

  # write data table
  global names, files, data_table, string_table
  for filepath in files:
    fd = open(filepath, "rb")
    writer.write_binary(fd.read())
  
  # write name table
  names_buffer = names.encode("utf8")
  writer.write_binary(names_buffer)

  # commit
  data_table += inode_offset
  string_table += data_table + data_offset
  writer.commit(data_table, string_table)

def pack(path: str, ruc: rucfs_writer) -> None:
  global inode_offset, names, name_offset, files, data_offset, data_table
  name = os.path.basename(path)
  if name == "":
    name = "/"
  name += "\0"
  isdir = os.path.isdir(path)
  ruc.write_uint((1 if isdir else 2), 1) # type
  ruc.write_uint(name_offset, 4)         # name_offset
  name_offset += len(name.encode("utf8"))
  names += name
  inode_offset = inode_offset + 13
  if isdir:
    dirlist = os.listdir(path)
    item_count = len(dirlist)
    ruc.write_uint(item_count, 4)     # item_count
    if item_count > 0:
      ruc.write_uint(inode_offset, 4) # ref_inode_offset
      for file in dirlist:
        pack(os.path.join(path, file), ruc)
    else:
      ruc.write_uint(0xffffffff, 4)   # ref_inode_offset
  else:
    data_length = os.path.getsize(path)
    ruc.write_uint(data_offset, 4)    # data_offset
    ruc.write_uint(data_length, 4)    # data_length
    data_offset += data_length
    files.append(path)

if __name__ == "__main__":
  main()
