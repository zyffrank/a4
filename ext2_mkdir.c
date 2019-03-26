#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "ext2.h"


int count_entries(char *path){
	int count;
	for (int i = 0; i < strlen(path); i++){
		if (path[i] == '/'){
			count ++;
		}
	}
	return count;
}


int compare_entry_name(char *target, char *candidate, int length){
	if (strlen(target) != length){
		return 0;
	}
	for (int i = 0; i < length; i++){
		if (target[i]!=candidate[i]){
			return 0;
		}
	}
	return 1;
}

//search entry name in one block
unsigned int search_block_entry(char *name, struct ext2_dir_entry *start){
	int count = 0;
	while(count < 1024){
		struct ext2_dir_entry *current = start + count;
		if (compare_entry_name(name, current->name) == 1){
			return current->inode;
		}
		count = count + current->rec_len;
	}
	return -1;
}

//Get all entry names and ignore '/'
void get_entries(char **entries, char *path, int num){
	int count = 0;
	int length = 0;
	for (int i = 1; i < strlen(path);i++){
		if (count == num){
			return;
		}
		if(path[i] == '/' || path[i] == '\0'){
			count++;
			length = 0;
			continue;
		}
		entries[count][length] = path[i];
	}
}

int main(int argc, char **argv){
	if(argc != 3) {
        fprintf(stderr, "Usage: %s <image file name>\n", argv[0]);
        exit(1);
    }
    int fd = open(argv[1], O_RDWR);
	if(fd == -1) {
		perror("open");
		exit(1);
    }

    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    struct ext2_super_block *sb = (struct ext2_super_block *)(disk + 1024);
    struct ext2_group_desc *gd = (struct ext2_group_desc *)(disk + 2048);
    struct ext2_inode *inode_table = (struct ext2_inode *) (disk + EXT2_BLOCK_SIZE * gd->bg_inode_table);
    unsigned char *block_bitmap = (unsigned char *)(disk + (EXT2_BLOCK_SIZE * gd->bg_block_bitmap));
    unsigned char *inode_bitmap = (unsigned char *)(disk + (EXT2_BLOCK_SIZE * gd->bg_inode_bitmap));
    if (*(argv[2]) != '/'){
    	perror("invalid absolute path");
    	exit(1);
    }
    char *path = argv[2];
    int num = count_entries(path);
    if (path[strlen(path)-1] == '/'){
    	num = num - 1;
    }
    char *entries[num];
    for (int i = 0; i < num;i++){
    	entries[i] = malloc(sizeof(char)*255);
    }
    get_entries(entries, path, num);
    struct ext2_inode *current = inode_table + sizeof(struct ext2_inode);
    //loop over each entry_name
    for (int i = 0; i < num; i++){	
    	unsigned int find = -1;
    	//loop over one directory which is represented by an inode
    	for (int j = 0; j < current->i_blocks; j++){
    		if (j <= 11){
    			find = search_block_entry(entries[i], current->i_block[j] * EXT2_BLOCK_SIZE + disk);
        		if (find != -1){
        			break;
        		}
        		continue;
    		}
    		else{
    			find = search_block_entry(entries[i],current->i_block[12][j-12] * EXT2_BLOCK_SIZE + disk);
    			if (find != -1){
    				break;
    			}
    			continue;
    		}
    	}
    	if (find == -1){
    		return EEXIST;
    	}
    	current = inode_table + sizeof(struct ext2_inode) * (find - 1);
    }
    if (current->i_blocks <= 12){
    	unsigned int target_block_num = current->i_block[current->i_blocks-1];
    }
    else{
    	unsigned int target_block_num = current->i_block[12][current->i_blocks-13];
    }
    struct ext2_dir_entry *start = target_block_num * EXT2_BLOCK_SIZE + disk;
    int count = 0;
    while (count < 1024){
    	struct ext2_dir_entry *start = (struct ext2_dir_entry*)(target_block_num * EXT2_BLOCK_SIZE + disk + count);
        count += start->rec_len;
    }
    struct ext2_dir_entry *new_entry = start + sizeof(struct ext2_dir_entry);
    new_entry->name = "aaa";
    printf("aaa");












}