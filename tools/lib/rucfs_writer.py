import time
import os

class rucfs_writer:

  inode_offset = 0
  names = ""
  name_offset = 0
  files = []
  data_offset = 0

  def __init__(self, path: str) -> None:
    # open file for writing
    self.fp = open(path, "wb+")

    # write header
    self.__write_header()

  def __write_header(self) -> None:
    self.write_str("rucfs\x00")     # magic
    self.write_uint(0x01, 1)        # version_major
    self.write_uint(0x00, 1)        # version_minor
    self.write_uint(int(time.time()), 4) # modded_time
    self.write_uint(0x00000000, 4)  # flag
    self.write_uint(0x00000020, 4)  # inode_table fixed at 0x20
    self.write_uint(0x00000000, 4)  # placeholder of data_table
    self.write_uint(0x00000000, 4)  # placeholder of string_table
    self.write_uint(0x00000000, 4)  # reserved

  def commit(self, dattab: int, strtab: int) -> None:
    self.fp.seek(0x14)
    self.write_uint(dattab, 4)     # inode_table offset
    self.write_uint(strtab, 4)     # data_table offset
    self.write_uint(0x00000000, 4) # string_table offset
    pass

  def write_str(self, data: str) -> None:
    self.fp.write(data.encode("utf8"))

  def write_binary(self, data: bytes) -> None:
    self.fp.write(data)

  def write_uint(self, data: int, align: int = 1) -> None:
    uint = data if data >= 0 else data + (1 << 32)
    self.fp.write(uint.to_bytes(align, "little"))

  def write_for_file(self, path: str):
    name = os.path.basename(path) + "\0"
    data_length = os.path.getsize(path)
    self.inode_offset += 13
    self.write_uint(2, 1) # type
    self.write_uint(self.name_offset, 4)  # name_offset
    self.write_uint(self.data_offset, 4)  # data_offset
    self.write_uint(data_length, 4)       # data_length
    self.data_offset += data_length
    self.files.append(path)
    self.names += name
    self.name_offset += len(name.encode("utf8"))

  def write_for_dir(self, path: str, pre_item_count: int):
    item_count = len(os.listdir(path))
    name = os.path.basename(path)
    if name == "":
      name = "/"
    name += "\0"
    self.inode_offset += 13
    self.write_uint(1, 1) # type
    self.write_uint(self.name_offset, 4) # name_offset
    self.write_uint(item_count, 4) # item_count
    if item_count == 0:
      self.write_uint(0xffffffff, 4) # ref_inode_offset
    else:
      self.write_uint(self.inode_offset + pre_item_count * 13, 4) # ref_inode_offset
    self.names += name
    self.name_offset += len(name.encode("utf8"))
