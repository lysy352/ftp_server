#include "memory_access.h"
#include "fatfs.h"
#include "ff.h"
#include "usb_host.h"
#include <string.h>

int8_t mount_usb() {
	fs = malloc(sizeof(FATFS));

	if(f_mount(fs, "", 1) != FR_OK) {
		printf("usb mount error\r\n");
		free(fs);
		return 0;
	}

	printf("mounted USB\r\n");
	return 1;
}

int8_t unmount_usb() {
	if(f_mount(0, "", 0) != FR_OK) {
		printf("usb unmount error\r\n");
		return 0;
	}
	free(fs);

	printf("unmounted USB\r\n");
	return 1;
}

int8_t is_full_path(const char *path) {
	if(path[0] == '/' ) {
		return 1;
	}
	return 0;
}

char *get_full_path(const char *current_path, const char *path) {
    char *new_path = NULL;
    if(is_full_path(path)) {
        new_path = malloc(sizeof(char)*(strlen(path) + 1));
        strcpy(new_path, path);
    } else if(current_path == NULL) {
        new_path = malloc(sizeof(char) * (1 + strlen(path) + 1));
        sprintf(new_path, "/%s", path);
    } else if(strcmp(current_path, "/") == 0) {
        new_path = malloc(sizeof(char)*(strlen(current_path) + strlen(path) + 1));
        sprintf(new_path, "%s%s", current_path, path);
    } else {
        new_path = malloc(sizeof(char)*(strlen(current_path) + 1 + strlen(path) + 1));
        sprintf(new_path, "%s/%s", current_path, path);
    }
    return new_path;
}

char *get_final_path(const char *full_path) {
    char *new_path = malloc(sizeof(char)*(strlen(BASIC_PATH) + strlen(full_path) + 1));
    new_path[0] = '\0';
    sprintf(new_path, "%s%s", BASIC_PATH, full_path);
    return new_path;
}

char *get_final_path_2(const char *current_path, const char *path) {
    char *full_path = get_full_path(current_path, path);
    char *final_path = get_final_path(full_path);
    free(full_path);
    return final_path;
}

int8_t directory_exist(const char *full_path) {
    char *final_path = get_final_path(full_path);
    DIR dir;
    if(f_opendir(&dir, final_path) != FR_OK) {
		printf("cannot change directory: %s\n", full_path);
		free(final_path);
		return 0;
	}
	f_closedir(&dir);
	free(final_path);
	return 1;  
}

char *change_directory(const char *current_path, const char *path) {
    char *full_path = get_full_path(current_path, path);
    if(directory_exist(full_path)) {
        return full_path;
    }
    free(full_path);
    return NULL;
}

char *list_directory(const char *current_path) {
    char *dir_path = get_final_path(current_path);
    DIR dir;
	if(f_opendir(&dir, dir_path) != FR_OK) {
		printf("cannot open directory %s\r\n", dir_path);
		free(dir_path);
		return NULL;
	}
	free(dir_path);

    char *list = malloc(sizeof(char)*MAX_LIST);
    list[0] = '\0';

    FILINFO finfo;
	while(1) {
		if(f_readdir(&dir, &finfo) != FR_OK || finfo.fname[0] == '\0')
			break;

		if(finfo.fattrib & AM_DIR )
			strcat(list, "+/");
		else 
			sprintf(list + strlen(list), "+r,s%lu", finfo.fsize);
		
		strcat(list,",\t");
		strcat(list, finfo.fname);
		strcat(list, "\r\n" );
	}
	f_closedir(&dir);
	return list;
}

FIL *open_file(const char *current_path, const char *filename) {
    char *filepath = get_final_path_2(current_path, filename);
    FIL *file = malloc(sizeof(FIL));

    if(f_open(file, filepath, FA_OPEN_EXISTING | FA_READ) != FR_OK ) {
		printf("cannot open file %s\r\n", filepath);
		free(filepath);
		free(file);
		return NULL;
	}

    free(filepath);
    return file;
}

FIL *create_file(const char *current_path, const char *filename) {
    char *filepath = get_final_path_2(current_path, filename);
    FIL *file = malloc(sizeof(FIL));

	if(f_open(file, filepath, FA_CREATE_NEW | FA_WRITE) != FR_OK ) {
		printf("cannot create file %s\r\n", filepath);
		free(filepath);
		free(file);
		return NULL;
	}
	free(filepath);
	return file;
}

void close_file(FIL *file) {
	f_close(file);
	free(file);
	file = NULL;
}

int8_t delete_file(const char *current_path, const char *filename) {
    char *filepath = get_final_path_2(current_path, filename);

	if(f_unlink(filepath) != FR_OK) {
		printf("cannot delete file: %s\r\n", filepath);
		free(filepath);
		return 0;
	}
	
	printf("deleted file: %s\r\n", filepath);
	free(filepath);
	return 1;
}

int8_t create_dir(const char *current_path, const char *dir_name) {
    char *dir_path = get_final_path_2(current_path, dir_name);
    FRESULT res = f_mkdir(dir_path);
    free(dir_path);
    if(res != FR_OK) {
		printf("connot make directory: %s\r\n", dir_path);
        return 0;
    }
    return 1;
}

uint16_t write_to_file(FIL *file, char *buf, uint16_t size) {
	uint16_t bw;
	if(f_write(file, buf, size, &bw) != FR_OK) {
		printf("write to file error\r\n");
		return -1;
	}
	return bw;
}

uint16_t read_file(FIL *file, char *buf, uint16_t size) {
	uint16_t br;
	if(f_read(file, buf, size, &br) != FR_OK) {
		printf("read file error\r\n");
		return -1;
	}
	return br;
}

void USB_Process(ApplicationTypeDef Appli_state) {
	 switch(Appli_state) {
		 case APPLICATION_START:
			 printf("Device connected.\r\n");
			 break;
		 case APPLICATION_READY:
			 mount_usb();
			 printf("Device ready.\r\n");
			 break;
		 case APPLICATION_DISCONNECT:
			 unmount_usb();
			 printf("Device disconnected.\r\n");
			 break;
		 default:
		 break;
	}
}
