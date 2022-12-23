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

typedef uint8_t rucfs_inode_type_t;
#define rucfs_inode_directory (1)
#define rucfs_inode_file      (2)
#define rucfs_inode_symlink   (3)

typedef uint32_t rucfs_flags_t;
#define rucfs_flag_endian_be  (0x00000001)

typedef struct {
  char magic[6];
  uint8_t version_major;
  uint8_t version_minor;
  uint32_t modded_time;
  rucfs_flags_t flags;
  uint32_t inode_table;
  uint32_t data_table;
  uint32_t string_table;
  uint32_t reserved;
} rucfs_superblock_t;

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

typedef struct {
  rucfs_inode_type_t type;
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

typedef struct {
  rucfs_inode_type_t type;
  char* name;
} rucfs_path_enum_t;

#define rucfs_ok(e) ((e) == rucfs_err_ok)
#define RUCFS_DEFAULT 0xFFFFFFFF

#define rucfs_open_symlink(ctx, inode)   \
  ((rucfs_inode_t *)(ctx->itab +         \
  ((rucfs_inode_symlink_t *)inode)->ref_inode_offset))

#define rucfs_open_directory(ctx, inode) \
  ((rucfs_inode_t *)(ctx->itab +         \
  ((rucfs_inode_directory_t *)inode)->ref_inode_offset))

#define rucfs_inode_name(ctx, inode)     \
  ((char* )ctx->strtab +                 \
  (inode)->name_offset)

/**
 * @brief load a rucfs binary
 *
 * @param data data pointer to a squashfs superblock
 * @param ctx if successfully return a context
 * @return if success return rucfs_err_ok
 */
rucfs_errcode_t rucfs_load(uint8_t* data, rucfs_ctx_t* ctx);

/**
 * @brief path to a inode by a string
 *
 * @param ctx rcufs context handle
 * @param file the path
 * @param inode the target inode
 * @return if success return rucfs_err_ok, not found return rucfs_err_notfound
*/
rucfs_errcode_t rucfs_path_to(rucfs_ctx_t* ctx, const char* file, rucfs_inode_t** inode);

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
 * @param fp file context
 * @return if success return rucfs_err_ok
 */
rucfs_errcode_t rucfs_fclose(rucfs_file_t* fp);

/**
 * @brief file exists
 *
 * @param ctx rcufs context handle
 * @param file path to file
 * @param err rucfs_errcode_t
 * @return if file/directory/symlink exist return true, otherwise return false
 */
bool rucfs_exist(rucfs_ctx_t* ctx, const char* file, rucfs_errcode_t* err);

/**
 * @brief enumerate the structure of path
 *
 * @param ctx rcufs context handle
 * @param path path to enumerate
 * @param list the directory structure, pass NULL to get the item amount of a path
 * @param size item amount
 * @param err rucfs_errcode_t
*/
rucfs_errcode_t rucfs_enumerate_path(rucfs_ctx_t* ctx, const char* path, rucfs_path_enum_t* list, size_t* size);

/**
 * @brief normalize the path string
 *
 * @param dst destination string
 * @param src source string
 * @param endslash append the end slash
 * @return return the length of normalized string
*/
size_t rucfs_normalize_path(char* dst, const char* src, bool endslash);

#endif /* _RUCFS_H */
