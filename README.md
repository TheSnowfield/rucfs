## RUCFS
RUCFS `/ˈrɑːkˈɛfˈɛs/` is a _Read-only Un-Compressed File System_.  
It provides a simple and compact structure to store data up to 4GB.

It supports regular files, nested directories, and symbolic links.  
Very suit for embedded systems that pursues size and low memory. such as STM32, ESP.. etc.

![badge](https://img.shields.io/badge/RUCFS-1.0-lightgrey) ![badge](https://img.shields.io/badge/License-MIT-blue)

## Code Snippets
```c
// load a rucfs image
rucfs_ctx_t ctx;
if(!rucfs_chk(rucfs_load(image, &ctx))) {
  printf("[****] RUCFS failed to load.");
  return 1;
}

// open the file
rucfs_file_t *file;
if(!rucfs_chk(rucfs_fopen(&ctx, "/hello/world.jpg", &file))) {
  printf("[****] No such file or directory.");
  return 2;
}

// the description of the file
char     *name  = file->name;
uint8_t  *data  = file->data;
uint32_t length = file->length;
printf("[ OK ] File loaded. Length %d", length);
```

## TODOs
- [ ] Big Endian
- [ ] Read/Write HAL
- [ ] Benchmark and Tests
- [x] mkfs.rucfs

## License
Licensed under MIT with ❤.

## Thanks
- [@takayama-lily](https://github.com/takayama-lily)
- [@LXY1226](https://github.com/LXY1226)
