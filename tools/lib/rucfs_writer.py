import time

class rucfs_writer:
  def __init__(self, path: str) -> None:
    # open file for writing
    self.fp = open(path, "wb+")

    # write header
    self.__write_header()

  def __write_header(self) -> None:
    self.write_str("rucfs\x00")     # magic
    self.write_uint(int(time.time()), 4) # modded_time
    self.write_uint(0x01, 1)        # version_major
    self.write_uint(0x00, 1)        # version_minor
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
