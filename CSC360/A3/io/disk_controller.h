#ifndef DISK_CONTROLLER
#define DISK_CONTROLLER

FILE* open_file(char* mode);
void create_vdisk();
void write_to_disk(int block_index, char* buffer);
char* read_from_disk(int block_index);

#endif