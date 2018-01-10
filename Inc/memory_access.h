#ifndef MEMORY_ACCESS_H_
#define MEMORY_ACCESS_H_

#define MAX_LIST 1024
#define BASIC_PATH ""

#include "memory_access.h"
#include "fatfs.h"
#include "ff.h"
#include "usb_host.h"
#include <string.h>

FATFS *fs;

int8_t mount_usb();

int8_t unmount_usb();

int8_t is_full_path(const char *path);

char *get_full_path(const char *current_path, const char *path);

char *get_final_path(const char *full_path);

char *get_final_path_2(const char *current_path, const char *path);

int8_t directory_exist(const char *full_path);

char *change_directory(const char *current_path, const char *path);

char *list_directory(const char *current_path);

FIL *open_file(const char *current_path, const char *filename);

FIL *create_file(const char *current_path, const char *filename);

void close_file(FIL *file);

int8_t delete_file(const char *current_path, const char *filename);

int8_t create_dir(const char *current_path, const char *dir_name);

uint16_t write_to_file(FIL *file, char *buf, uint16_t size);

uint16_t read_file(FIL *file, char *buf, uint16_t size);

void USB_Process(ApplicationTypeDef Appli_state);



#endif /* MEMORY_ACCESS_H_ */
