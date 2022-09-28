#ifndef _RUCFS_H
#define _RUCFS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef _MSC_VER
#define __attribute__(x)
#endif

typedef enum {
  rucfs_err_ok = 0,
  rucfs_err_arguments     = -1,
  rucfs_err_data_broken   = -2,
  rucfs_err_unsupported   = -3,
  rucfs_err_notfound      = -4,
  rucfs_err_out_of_memory = -5,
} rucfs_errcode_t;

typedef enum {
  rucfs_inode_directory        = 1,
  rucfs_inode_file             = 2,
  rucfs_inode_symlink          = 3,
} rucfs_inode_type_t;

typedef enum {
  rucfs_flag_endian_be = 0x00000001,
  rucfs_flag_max       = 0xFFFFFFFF
} rucfs_flags_t;

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

typedef struct {
  char magic[6];
  uint32_t modded_time;
  uint8_t version_major;
  uint8_t version_minor;
  rucfs_flags_t flags;
  uint32_t inode_table;
  uint32_t data_table;
  uint32_t string_table;
  uint32_t reserved;
} __attribute__((packed)) rucfs_superblock_t;

typedef struct {
  rucfs_inode_type_t type : 8;
  uint32_t name_offset;
} __attribute__((packed)) rucfs_inode_t;

typedef struct {
  rucfs_inode_t common;
  uint32_t item_count;
  uint32_t ref_inode_offset;
} __attribute__((packed)) rucfs_inode_directory_t;

typedef struct {
  rucfs_inode_t common;
  uint32_t data_offset;
  uint32_t data_length;
} __attribute__((packed)) rucfs_inode_file_t;

typedef struct {
  rucfs_inode_t common;
  uint32_t ref_inode_offset;
} __attribute__((packed)) rucfs_inode_symlink_t;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

typedef struct {
  uint8_t* itab;
  uint8_t* dattab;
  uint8_t* strtab;
  rucfs_inode_directory_t* rootdir;
} rucfs_ctx_t;

typedef struct {
  char* name;
  uint8_t* data;
  uint32_t length;
} rucfs_file_t;

#define rucfs_chk(e) ((e) == rucfs_err_ok)

/**
 * @brief load a rucfs binary
 *
 * @param data data pointer to a squashfs superblock
 * @param ctx if successfully return a context
 * @return if success return rucfs_err_ok
 */
rucfs_errcode_t rucfs_load(uint8_t* data, rucfs_ctx_t* ctx);

/**
 * @brief open file
 *
 * @param ctx rcufs context handle
 * @param file path to file
 * @param fp file context
 * @return if success return rucfs_err_ok
 */
rucfs_errcode_t rucfs_fopen(rucfs_ctx_t* ctx, const char* file, rucfs_file_t** fp);

/**
 * @brief close the file
 *
 * @param ctx rcufs context handle
 * @param fp file context
 * @return if success return rucfs_err_ok
 */
rucfs_errcode_t rucfs_fclose(rucfs_ctx_t* ctx, rucfs_file_t* fp);

/**
 * @brief file exists
 *
 * @param ctx rcufs context handle
 * @param file path to file
 * @param err rucfs_errcode_t
 * @return if file/directory/symlink exist return true, otherwise return false
 */
bool rucfs_exist(rucfs_ctx_t* ctx, const char* file, rucfs_errcode_t* err);

#endif /* _RUCFS_H */
