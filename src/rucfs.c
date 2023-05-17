#include <string.h>
#include <stdlib.h>

#include "rucfs.h"

static size_t _typelen[] = {
  0,
  sizeof(rucfs_inode_directory_t),
  sizeof(rucfs_inode_file_t),
  sizeof(rucfs_inode_symlink_t)
};

rucfs_errcode_t rucfs_load(uint8_t* data, rucfs_ctx_t* ctx) {

  // checks
  if(data == NULL || ctx == NULL)
    return rucfs_err_arguments;

  // convert type then run some checks
  rucfs_superblock_t* superblock = (rucfs_superblock_t *)data; {

    // check the magic code {'r','u','c','f','s','\0'}
    if(strcmp(superblock->magic, "rucfs") != 0)
      return rucfs_err_data_broken;

    // only supported rucfs 1.0 format
    if(superblock->version_major != 1)
      return rucfs_err_unsupported;
  }

  // checks are all passed then inflate the context
  ctx->itab    = data + superblock->inode_table;
  ctx->dattab  = data + superblock->data_table;
  ctx->strtab  = data + superblock->string_table;
  ctx->rootdir = (rucfs_inode_directory_t *)ctx->itab;

  // assert the root is a directory
  if(ctx->rootdir->common.type != rucfs_inode_directory)
    return rucfs_err_data_broken;

  // check the root directory name
  if(strcmp(rucfs_inode_name(ctx, &ctx->rootdir->common), "/") != 0)
    return rucfs_err_data_broken;

  // if root directory is empty
  // then assert the ref offset is 0xFFFFFFFF
  if(ctx->rootdir->item_count == 0 &&
    ctx->rootdir->ref_inode_offset != RUCFS_DEFAULT) {
    return rucfs_err_data_broken;
  }

  return rucfs_err_ok;
}

rucfs_errcode_t rucfs_path_to(rucfs_ctx_t* ctx, const char* file, rucfs_inode_t** inode) {

  // setup current inode as root directory
  size_t current_items = ctx->rootdir->item_count;
  rucfs_inode_t* current = &ctx->rootdir->common;

  // setup the file name we stepped in
  size_t filename_len = strlen(file);
  char* stepname = (char *)file;
  char* stepend  = stepname + filename_len;

  // if stepname is '/' indicates we're going to find '/' in the root,
  // but we're already in the root, so we can directly return :p.
  if (filename_len == 1 && (*stepname) == '/') {
    *inode = current;
    return rucfs_err_ok;
  }

  while(true) {

    // directory name
    const char* current_name = rucfs_inode_name(ctx, current);
    size_t length = strlen(current_name);

    // compare the name
    if(strncmp(stepname, current_name, length) == 0) {
      
      // if current char is '/' so step into this directory
      if((*stepname) == '/' || *(stepname + length) == '/') {

        if ((*stepname) != '/') ++stepname;
        stepname += length;

        // if a directory
        if(current->type == rucfs_inode_directory) {
          current_items = ((rucfs_inode_directory_t *)current)->item_count;
          current = rucfs_open_directory(ctx, current);
        }

        // if a symbol link
        else if(current->type == rucfs_inode_symlink) {
          current = rucfs_open_symlink(ctx, current); {
            if(current->type != rucfs_inode_directory)
              return rucfs_err_notfound;
          }
          current_items = ((rucfs_inode_directory_t *)current)->item_count;
        }

        // the regular file :P
        else if (current->type == rucfs_inode_file)
          return rucfs_err_notfound;

        continue;
      }

      // okay we have stepped to the end of the path
      else if(stepname + length == stepend) {
        break;
      }

      // no such file or directory
      else if(current_items <= 0)
        return rucfs_err_notfound;
    }

    else if(*stepname == '\0') break;

    // not deep enough....
    next_inode:

    // breadth-first search
    if(current_items > 0) {

      // move to next inode
      current = (rucfs_inode_t *)(((uint8_t *)current) + _typelen[current->type]);
      --current_items;
    }

    // no directory to step in
    else return rucfs_err_notfound;
  }

  *inode = current;
  return rucfs_err_ok;
}

rucfs_errcode_t rucfs_fopen(rucfs_ctx_t* ctx, const char* file, rucfs_file_t** fp) {
  
  // some checks
  if(ctx == NULL || fp == NULL || file == NULL)
    return rucfs_err_arguments;

  if(strlen(file) == 0)
    return rucfs_err_arguments;

  // if the root directory is empty
  if(ctx->rootdir->item_count == 0)
    return rucfs_err_notfound;

  // try to path to the file
  rucfs_inode_file_t *inode; {
    if(!rucfs_ok(rucfs_path_to(ctx, file, (rucfs_inode_t **)&inode)))
      return rucfs_err_notfound;

    // current inode is not a file
    if(inode->common.type != rucfs_inode_file)
      return rucfs_err_notfound;
  }

  // okay create a fp for it
  rucfs_file_t *tmp = (rucfs_file_t *)malloc(sizeof(rucfs_file_t)); {
    
    // out of memory
    if (tmp == NULL)
      return rucfs_err_out_of_memory;

    // fill the structure
    tmp->name = rucfs_inode_name(ctx, &inode->common);
    tmp->data = ctx->dattab + inode->data_offset;
    tmp->length = inode->data_length;
  }

  *fp = tmp;
  return rucfs_err_ok;
}

rucfs_errcode_t rucfs_fclose(rucfs_file_t *fp) {
  
  // checks
  if(fp == NULL)
    return rucfs_err_arguments;

  // do clear
  fp->name = NULL;
  fp->data = NULL;
  fp->length = 0;
  free(fp);

  return rucfs_err_ok;
}

bool rucfs_exist(rucfs_ctx_t* ctx, const char *file, rucfs_errcode_t *err) {
  
  // some checks
  if(ctx == NULL || file == NULL || strlen(file) == 0) {
    *err = rucfs_err_arguments;
    return false;
  }

  // try to path to the file
  rucfs_inode_file_t *inode; {
    if(!rucfs_ok(rucfs_path_to(ctx, file, (rucfs_inode_t **)&inode)))
      return false;
  }

  return true;
}

size_t rucfs_normalize_path(char* dst, const char* src, bool endslash) {
  
  size_t i = 0, j = -1;

  for (; *src != '\0'; ++src, ++i) {

    // copy normal characters
    if(*src != '/') {
      dst[++j] = *src;
    }
    
    // copy and keep '/' single
    else if(*src == '/' && dst[j] != '/') {
      dst[++j] = '/';
    }
  }

  // sealed the string
  if(endslash && !(i == 1 && dst[j] == '/')) {
    dst[++j]   = '/';
    dst[++j] = '\0';
  }
  
  else if (!endslash) {
    dst[j] = '\0';
  }

  else dst[++j] = '\0';

  return j;
}

rucfs_errcode_t rucfs_enumerate_path(rucfs_ctx_t* ctx, const char* path, rucfs_path_enum_t* list, size_t* size) {

  // some checks
  if(ctx == NULL || path == NULL || strlen(path) == 0) {
    return rucfs_err_arguments;
  }

  else if(list == NULL && size == NULL) {
    return rucfs_err_arguments;
  }

  // path to inode first
  rucfs_inode_t* inode;
  if(!rucfs_ok(rucfs_path_to(ctx, path, &inode))) {
    return rucfs_err_notfound;
  }

  // if this inode is not a directory
  if(inode->type != rucfs_inode_directory) {
    return rucfs_err_notfound;
  }

  size_t inode_items = ((rucfs_inode_directory_t *)inode)->item_count;
  
  // count the files of a path
  if(list == NULL && size != NULL) {
    *size = inode_items;
    return rucfs_err_ok;
  }

  // retrieve the directory structure of a path
  else if (list != NULL && size != NULL) {

    // exceeds the max items that this directory contains
    if(*size > inode_items)
      return rucfs_err_arguments;

    inode = rucfs_open_directory(ctx, inode);
    for(size_t i = 0; i < *size; ++i) {

      // fill the list
      list->name = rucfs_inode_name(ctx, inode);
      list->type = inode->type;

      // move to next inode
      inode = (rucfs_inode_t *)(((uint8_t *)inode) + _typelen[inode->type]);
      ++list;
    }
    return rucfs_err_ok;
  }

  else return rucfs_err_arguments;
}
