#include <stdio.h>
#include "ext2_fs.h"
#include "read_ext2.h"
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

void copy_data(int fd, struct ext2_inode *inode, unsigned int inode_no, char *dir)
{
    // printf("copy_data: %u\n", inode_no);

    char filename[EXT2_NAME_LEN];
    // char actual_name[EXT2_NAME_LEN];
    char buffer[block_size];

    unsigned int sindirect[block_size / sizeof(unsigned int)];
    unsigned int dindirect[block_size / sizeof(unsigned int)];
    // unsigned int tindirect[block_size / sizeof(unsigned int)];

    int i_indirect_inodes = block_size / sizeof(unsigned int);

    sprintf(filename, "%s/file-%i.jpg", dir, inode_no);
    int fd_output = open(filename, O_WRONLY | O_TRUNC | O_CREAT, 0666);
    if (fd_output == -1)
    {
        perror("open");
	//printf("Filename: -%s-\n", filename);
        exit(1);
    }

    unsigned int filesize = inode->i_size;

    // print i_block numberss
    for (unsigned int i = 0; i < EXT2_N_BLOCKS; i++)
    {
        if (i < EXT2_NDIR_BLOCKS) /* direct blocks */
        {
            if (inode->i_block[i] == 0)
                continue;

            lseek(fd, BLOCK_OFFSET(inode->i_block[i]), SEEK_SET);
            int n;
            if (filesize > block_size)
                n = read(fd, buffer, block_size);
            else
                n = read(fd, buffer, filesize);
            filesize = filesize - n;
            write(fd_output, buffer, n);
            //printf("Block %2u : %u\n", i, inode->i_block[i]);
        }
        else if (i == EXT2_IND_BLOCK) /* single indirect block */
        {
            if (inode->i_block[i] == 0)
                continue;

            lseek(fd, BLOCK_OFFSET(inode->i_block[i]), SEEK_SET);
            read(fd, sindirect, block_size);
            for (int j = 0; j < i_indirect_inodes; j++)
            {
                if (sindirect[j] == 0)
                    continue;
                lseek(fd, BLOCK_OFFSET(sindirect[j]), SEEK_SET);

                int n;
                if (filesize > block_size)
                    n = read(fd, buffer, block_size);
                else
                    n = read(fd, buffer, filesize);
                filesize = filesize - n;
                write(fd_output, buffer, n);
            }
            //printf("Single   : %u\n", inode->i_block[i]);
        }
        else if (i == EXT2_DIND_BLOCK) /* double indirect block */
        {
            if (inode->i_block[i] == 0)
                continue;

            lseek(fd, BLOCK_OFFSET(inode->i_block[i]), SEEK_SET);
            read(fd, sindirect, block_size);
            for (int j = 0; j < i_indirect_inodes; j++)
            {
                if (sindirect[j] == 0)
                    continue;
                lseek(fd, BLOCK_OFFSET(sindirect[j]), SEEK_SET);
                read(fd, dindirect, block_size);
                for (int k = 0; k < i_indirect_inodes; k++)
                {
                    if (dindirect[k] == 0)
                        continue;
                    lseek(fd, BLOCK_OFFSET(dindirect[k]), SEEK_SET);
                    int n;
                    if (filesize > block_size)
                        n = read(fd, buffer, block_size);
                    else
                        n = read(fd, buffer, filesize);
                    filesize = filesize - n;
                    write(fd_output, buffer, n);
                }
            }
            //printf("Double   : %u\n", inode->i_block[i]);
        }
        else if (i == EXT2_TIND_BLOCK) /* triple indirect block */
        {
            continue;
            // if (inode->i_block[i] != 0)
            // {
            // 	printf("Triple   : %u\n", inode->i_block[i]);
            // }
        }
    }
    close(fd_output);
}

int is_jpg(char *buffer)
{
    int is_jpg = 0;
    if (buffer[0] == (char)0xff &&
        buffer[1] == (char)0xd8 &&
        buffer[2] == (char)0xff &&
        (buffer[3] == (char)0xe0 ||
         buffer[3] == (char)0xe1 ||
         buffer[3] == (char)0xe8))
    {
        if (debug)
            printf("Found a jpg\n");
        is_jpg = 1;
    }
    return is_jpg;
}

char *get_filename(int inode_no, int *i_array, char *filenames[], int count)
{
    for(int i = 0; i < count; i++)
    {
        if(i_array[i] == inode_no)
        {
            return filenames[i];
        }
    }
    return NULL;
}

int get_index(int inode_no, int *i_array, int count)
{
    for(int i = 0; i < count; i++)
    {
        if(i_array[i] == inode_no)
        {
            return i;
        }
    }
    //printf("get_index: %u not found.\n", inode_no);
    return -1;
}

void copy_data_name(int fd, struct ext2_inode *inode, char *dir, int index, char *filenames[])
{
    //printf("copy_data_name: %s\n", filenames[index]);

    char filename[EXT2_NAME_LEN * 2];
    // char actual_name[EXT2_NAME_LEN];
    char buffer[block_size];

    unsigned int sindirect[block_size / sizeof(unsigned int)];
    unsigned int dindirect[block_size / sizeof(unsigned int)];
    // unsigned int tindirect[block_size / sizeof(unsigned int)];

    int i_indirect_inodes = block_size / sizeof(unsigned int);

    sprintf(filename, "%s/%s", dir, filenames[index]);
    int fd_output = open(filename, O_WRONLY | O_TRUNC | O_CREAT, 0666);
    if (fd_output == -1)
    {
        perror("open");
	// printf("Filename: -%s-\n", filename);
        exit(1);
    }

    unsigned int filesize = inode->i_size;

    // print i_block numberss
    for (unsigned int i = 0; i < EXT2_N_BLOCKS; i++)
    {
        if (i < EXT2_NDIR_BLOCKS) /* direct blocks */
        {
            if (inode->i_block[i] == 0)
                continue;

            lseek(fd, BLOCK_OFFSET(inode->i_block[i]), SEEK_SET);
            int n;
            if (filesize > block_size)
                n = read(fd, buffer, block_size);
            else
                n = read(fd, buffer, filesize);
            filesize = filesize - n;
            write(fd_output, buffer, n);
            // printf("Block %2u : %u\n", i, inode->i_block[i]);
        }
        else if (i == EXT2_IND_BLOCK) /* single indirect block */
        {
            if (inode->i_block[i] == 0)
                continue;

            lseek(fd, BLOCK_OFFSET(inode->i_block[i]), SEEK_SET);
            read(fd, sindirect, block_size);
            for (int j = 0; j < i_indirect_inodes; j++)
            {
                if (sindirect[j] == 0)
                    continue;
                lseek(fd, BLOCK_OFFSET(sindirect[j]), SEEK_SET);

                int n;
                if (filesize > block_size)
                    n = read(fd, buffer, block_size);
                else
                    n = read(fd, buffer, filesize);
                filesize = filesize - n;
                write(fd_output, buffer, n);
            }
            // printf("Single   : %u\n", inode->i_block[i]);
        }
        else if (i == EXT2_DIND_BLOCK) /* double indirect block */
        {
            if (inode->i_block[i] == 0)
                continue;

            lseek(fd, BLOCK_OFFSET(inode->i_block[i]), SEEK_SET);
            read(fd, sindirect, block_size);
            for (int j = 0; j < i_indirect_inodes; j++)
            {
                if (sindirect[j] == 0)
                    continue;
                lseek(fd, BLOCK_OFFSET(sindirect[j]), SEEK_SET);
                read(fd, dindirect, block_size);
                for (int k = 0; k < i_indirect_inodes; k++)
                {
                    if (dindirect[k] == 0)
                        continue;
                    lseek(fd, BLOCK_OFFSET(dindirect[k]), SEEK_SET);
                    int n;
                    if (filesize > block_size)
                        n = read(fd, buffer, block_size);
                    else
                        n = read(fd, buffer, filesize);
                    filesize = filesize - n;
                    write(fd_output, buffer, n);
                }
            }
            // printf("Double   : %u\n", inode->i_block[i]);
        }
        else if (i == EXT2_TIND_BLOCK) /* triple indirect block */
        {
            continue;
            // if (inode->i_block[i] != 0)
            // {
            // 	printf("Triple   : %u\n", inode->i_block[i]);
            // }
        }
    }
    close(fd_output);
}


int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("expected usage: ./runscan inputfile outputfile\n");
        exit(0);
    }

    DIR *dir = opendir(argv[2]);
    if (dir != NULL)
    {
        printf("output directory %s already exists\n", argv[2]);
        exit(1);
    }

    if (mkdir(argv[2], 0777) == -1)
    {
        perror("mkdir");
        exit(1);
    }

    char buffer[block_size];
    int i_indirect_inodes = block_size / sizeof(unsigned int);
    unsigned int sindirect[i_indirect_inodes];
    unsigned int dindirect[i_indirect_inodes];
    // unsigned int tindirect[i_indirect_inodes];
    int fd;

    fd = open(argv[1], O_RDONLY); /* open disk image */

    ext2_read_init(fd);

    struct ext2_super_block super;
    struct ext2_group_desc group;

    // example read first the super-block and group-descriptor
    read_super_block(fd, 0, &super);
    read_group_desc(fd, 0, &group);

    // printf("There are %u inodes in an inode table block and %u blocks in the idnode table\n", inodes_per_block, itable_blocks);
    
    // iterate the first inode block
    off_t start_inode_table = locate_inode_table(0, &group);
    int i_num[block_size * 10];
    int count = 0;
    char *filenames[block_size * 10];
    // iterate through for directories and populate storing variables
    for(unsigned int n = 0; n < inodes_per_block * itable_blocks; n++)
    {
        struct ext2_inode *inode = malloc(sizeof(struct ext2_inode));
        read_inode(fd, 0, start_inode_table, n , inode);
        if(!(S_ISDIR(inode->i_mode) || S_ISREG(inode->i_mode)))
        {
            continue;
        }
        // printf("inode %u: \n", n);
        //unsigned int i_blocks = inode->i_blocks / (2 << super.s_log_block_size);
        // printf("number of blocks %u\n", i_blocks);
        // printf("number of links %u\n", inode->i_links_count);
        
        if(S_ISDIR(inode->i_mode))
        {
            // printf("Is directory? %s \n Is Regular file? %s\n",
            //      S_ISDIR(inode->i_mode) ? "true" : "false",
            //    S_ISREG(inode->i_mode) ? "true" : "false");
            struct ext2_dir_entry *dentry;
            off_t curr_offset = 24;
            for (unsigned int i = 0; i < EXT2_N_BLOCKS; i++)
            {
                if (i < EXT2_NDIR_BLOCKS) /* direct blocks */
                {
                    if (inode->i_block[i] == 0)
                        continue;
                    lseek(fd, BLOCK_OFFSET(inode->i_block[i]), SEEK_SET);
                    read(fd, buffer, block_size);
                    curr_offset = 24;
                    while(curr_offset < block_size)
                    {
                        dentry = (struct ext2_dir_entry*) & ( buffer[curr_offset] );
                        if(dentry->inode == 0)
                            break;
                        int name_len = dentry->name_len & 0xFF; // convert 2 bytes to 4 bytes properly
                        char name [EXT2_NAME_LEN];
                        strncpy(name, dentry->name, name_len);
                        name[name_len] = '\0';
                        //printf("Entry name is --%s--\n", name);
                        //printf("Entry inode is --%u--\n", dentry->inode);
                        // if (name[name_len-1] == 'g' && name[name_len-2] == 'p'
                        // && name[name_len-3] == 'j' && name[name_len-4] == '.')
                        // {
                            // printf("found .jpg\n");
                        i_num[count] = dentry->inode;
                        // filenames[count] = (char *)malloc(strlen(name));
                        // strncpy(filenames[count], name, strlen(name));
                        // strdup does above two lines
                        filenames[count] = strdup(name);
                        count = count + 1;
                        // }
                        if(name_len % 4 == 0)
                            curr_offset = curr_offset + (8 + name_len);
                        else 
                            curr_offset = curr_offset + (8 + name_len + (4 - (name_len % 4)));
                    }
                    //printf("Block %2u : %u\n", i, inode->i_block[i]);
                }
                else if (i == EXT2_IND_BLOCK) /* single indirect block */
                {
                    if (inode->i_block[i] == 0)
                        continue;
                    lseek(fd, BLOCK_OFFSET(inode->i_block[i]), SEEK_SET);
                    read(fd, sindirect, block_size);
                    for (int j = 0; j < i_indirect_inodes; j++)
                    {
                        if (sindirect[j] == 0)
                            continue;
                        lseek(fd, BLOCK_OFFSET(sindirect[j]), SEEK_SET);
                        read(fd, buffer, block_size);
                        curr_offset = 24;
                        while(curr_offset < block_size)
                        {
                            dentry = (struct ext2_dir_entry*) & ( buffer[curr_offset] );
                            if(dentry->inode == 0)
                                break;
                            int name_len = dentry->name_len & 0xFF; // convert 2 bytes to 4 bytes properly
                            char name [EXT2_NAME_LEN];
                            strncpy(name, dentry->name, name_len);
                            name[name_len] = '\0';
                            //printf("Entry name is --%s--\n", name);
                            //printf("Entry inode is --%u--\n", dentry->inode);
                            // if (name[name_len-1] == 'g' && name[name_len-2] == 'p'
                            // && name[name_len-3] == 'j' && name[name_len-4] == '.')
                            // {
                            //     printf("found .jpg\n");
                            i_num[count] = dentry->inode;
                            filenames[count] = strdup(name);
                            count = count + 1;
                            // }
                            if(name_len % 4 == 0)
                                curr_offset = curr_offset + (8 + name_len);
                            else 
                                curr_offset = curr_offset + (8 + name_len + (4 - (name_len % 4)));
                        }
                    }
                    //printf("Single   : %u\n", inode->i_block[i]);
                }
                else if (i == EXT2_DIND_BLOCK) /* double indirect block */
                {
                    if (inode->i_block[i] == 0)
                        continue;
                    lseek(fd, BLOCK_OFFSET(inode->i_block[i]), SEEK_SET);
                    read(fd, sindirect, block_size);
                    for (int j = 0; j < i_indirect_inodes; j++)
                    {
                        if (sindirect[j] == 0)
                            continue;
                        lseek(fd, BLOCK_OFFSET(sindirect[j]), SEEK_SET);
                        read(fd, dindirect, block_size);
                        for (int k = 0; k < i_indirect_inodes; k++)
                        {
                            if (dindirect[k] == 0)
                                continue;
                            lseek(fd, BLOCK_OFFSET(dindirect[k]), SEEK_SET);
                            read(fd, buffer, block_size);
                            curr_offset = 24;
                            while(curr_offset < block_size)
                            {
                                dentry = (struct ext2_dir_entry*) & ( buffer[curr_offset] );
                                if(dentry->inode == 0)
                                    break;
                                int name_len = dentry->name_len & 0xFF; // convert 2 bytes to 4 bytes properly
                                char name [EXT2_NAME_LEN];
                                strncpy(name, dentry->name, name_len);
                                name[name_len] = '\0';
                                //printf("Entry name is --%s--\n", name);
                                //printf("Entry inode is --%u--\n", dentry->inode);
                                // if (name[name_len-1] == 'g' && name[name_len-2] == 'p'
                                // && name[name_len-3] == 'j' && name[name_len-4] == '.')
                                // {
                                //     printf("found .jpg\n");
                                i_num[count] = dentry->inode;
                                filenames[count] = strdup(name);
                                count = count + 1;
                                // }
                                if(name_len % 4 == 0)
                                    curr_offset = curr_offset + (8 + name_len);
                                else 
                                    curr_offset = curr_offset + (8 + name_len + (4 - (name_len % 4)));
                            }
                        }
                    }
                    //printf("Double   : %u\n", inode->i_block[i]);
                }
                else if (i == EXT2_TIND_BLOCK) /* triple indirect block */
                {
                    continue;
                }
            }
        }
        free(inode);
    }
    //for(int a = 0; a < count; a++)
    //{
    //    printf("i_num %u: \n", i_num[a]);
    //    printf("filename %s: \n", filenames[a]);
    //}
    // exit(0);

    // iterate the first inode block
    start_inode_table = locate_inode_table(0, &group);
    // for (unsigned int i = 0; i < inodes_per_block; i++)
    // iterate through to copy files with filename and file-inode#
    for (unsigned int n = 0; n < inodes_per_block * itable_blocks; n++)
    {
        struct ext2_inode *inode = malloc(sizeof(struct ext2_inode));
        read_inode(fd, 0, start_inode_table, n, inode);
        /* the maximum index of the i_block array should be computed from i_blocks / ((1024<<s_log_block_size)/512)
         * or once simplified, i_blocks/(2<<s_log_block_size)
         * https://www.nongnu.org/ext2-doc/ext2.html#i-blocks
         */
        // printf("here\n");
        if (!(S_ISDIR(inode->i_mode) || S_ISREG(inode->i_mode)))
        {
            // printf("continue..\n");
            continue;
        }
        // if (S_ISREG(inode->i_mode))
        // {
        // 	char buffer[block_size];
        // 	off_t start_data_block = locate_data_blocks(0, &group);

        //printf("inode %u: \n", n);
        //unsigned int i_blocks = inode->i_blocks / (2 << super.s_log_block_size);
        //printf("number of blocks %u\n", i_blocks);
        //printf("number of links %u\n", inode->i_links_count);

        if (S_ISREG(inode->i_mode))
        {
            //printf("Is directory? %s \n Is Regular file? %s\n",
            //       S_ISDIR(inode->i_mode) ? "true" : "false",
            //       S_ISREG(inode->i_mode) ? "true" : "false");
            // print i_block numberss
            for (unsigned int i = 0; i < EXT2_N_BLOCKS; i++)
            {
                if (i < EXT2_NDIR_BLOCKS) /* direct blocks */
                {
                    if (inode->i_block[i] == 0)
                        continue;
                    lseek(fd, BLOCK_OFFSET(inode->i_block[i]), SEEK_SET);
                    read(fd, buffer, block_size);
                    if (is_jpg(buffer))
                    {
                        copy_data(fd, inode, n, argv[2]);
                        int temp = get_index(n, i_num, count);
                        if(temp != -1)
                            copy_data_name(fd, inode, argv[2], temp, filenames);
                        break;
                    }
                    //printf("Block %2u : %u\n", i, inode->i_block[i]);
                }
                else if (i == EXT2_IND_BLOCK) /* single indirect block */
                {
                    if (inode->i_block[i] == 0)
                        continue;
                    lseek(fd, BLOCK_OFFSET(inode->i_block[i]), SEEK_SET);
                    read(fd, sindirect, block_size);
                    for (int j = 0; j < i_indirect_inodes; j++)
                    {
                        if (sindirect[j] == 0)
                            continue;
                        lseek(fd, BLOCK_OFFSET(sindirect[j]), SEEK_SET);
                        read(fd, buffer, block_size);
                        if (is_jpg(buffer))
                        {
                            copy_data(fd, inode, n, argv[2]);
                            int temp = get_index(n, i_num, count);
                            if(temp != -1)
                                copy_data_name(fd, inode, argv[2], temp, filenames);
                            break;
                        }
                    }
                    //printf("Single   : %u\n", inode->i_block[i]);
                }
                else if (i == EXT2_DIND_BLOCK) /* double indirect block */
                {
                    if (inode->i_block[i] == 0)
                        continue;
                    lseek(fd, BLOCK_OFFSET(inode->i_block[i]), SEEK_SET);
                    read(fd, sindirect, block_size);
                    for (int j = 0; j < i_indirect_inodes; j++)
                    {
                        if (sindirect[j] == 0)
                            continue;
                        lseek(fd, BLOCK_OFFSET(sindirect[j]), SEEK_SET);
                        read(fd, dindirect, block_size);
                        for (int k = 0; k < i_indirect_inodes; k++)
                        {
                            if (dindirect[k] == 0)
                                continue;
                            lseek(fd, BLOCK_OFFSET(dindirect[k]), SEEK_SET);
                            read(fd, buffer, block_size);
                            if (is_jpg(buffer))
                            {
                                copy_data(fd, inode, n, argv[2]);
                                int temp = get_index(n, i_num, count);
                                if(temp != -1)
                                    copy_data_name(fd, inode, argv[2], temp, filenames);
                                break;
                            }
                        }
                    }
                    //printf("Double   : %u\n", inode->i_block[i]);
                }
                else if (i == EXT2_TIND_BLOCK) /* triple indirect block */
                {
                    continue;
                    // if (inode->i_block[i] == 0)
                    // 	continue;
                    // printf("Triple   : %u\n", inode->i_block[i]);
                }
            }
            //printf("--\n");
        }
        free(inode);
    }

    close(fd);
}
