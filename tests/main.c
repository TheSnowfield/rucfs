#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../src/rucfs.h"

bool load_image(const char* path, uint8_t** image, size_t* length) {
  
  // open the file
  FILE* fd = NULL;
  fd = fopen(path, "r"); {

    if(fd == NULL) return false;
    
    // get file length
    fseek(fd, 0, SEEK_END); {
      *length = ftell(fd);
      if(*length == 0) return false;
    }

    // read the whole file
    fseek(fd, 0, SEEK_SET); {
      *image = malloc(*length); {
        if(*image == NULL) return false;
      }

      // read into
      fread(*image, *length, 1, fd);
    }
  }

  fclose(fd);
  return true;
}

void test_openfile(rucfs_ctx_t* ctx) {
  rucfs_file_t* file;
  if (!rucfs_ok(rucfs_fopen(ctx, "/banana/peel.txt", &file))) {
    printf("[****] No such file or directory.\n");
    return;
  }

  // the description of the file
  char*    fname = file->name;
  uint8_t* fdata = file->data;
  uint32_t flength = file->length;

  printf("[ OK ] File loaded. length = %d.\n", flength);
  rucfs_fclose(file);
  printf("[ OK ] File closed.\n");
}

void test_retrieve_path(rucfs_ctx_t* ctx) {
  size_t size;
  rucfs_enumerate_path(ctx, "/", NULL, &size);
  printf("[ OK ] Retrieve the files of the path. \"/\" contains %d item(s).\n", size);

  rucfs_path_enum_t* list = malloc(sizeof(rucfs_path_enum_t) * size); {
    rucfs_enumerate_path(ctx, "/", list, &size);
    for (size_t i = 0; i < size; ++i) {
      printf("  - %s%s\n", list[i].name, list[i].type == rucfs_inode_directory ? "/" : "");
    }
  }

  free(list);
}

int main() {

  uint8_t* image;
  size_t   length;
  if(!load_image("/mnt/m/Projects/rucfs/tests/build/rucfs.img", &image, &length)) {
    printf("[****] Filed to open the file.\n");
    return 1;
  }

  printf("[ OK ] Rucfs image opened. 0x%08x %d.\n", image, length);

  // load a rucfs image
  rucfs_ctx_t ctx;
  if (!rucfs_ok(rucfs_load(image, &ctx))) {
    printf("[****] Rucfs failed to load.\n");
    return 2;
  }

  printf("[ OK ] Rucfs image loaded. root dir = 0x%08x.\n", ctx.rootdir);

  // run tests
  test_openfile(&ctx);
  test_retrieve_path(&ctx);

  free(image);
}
