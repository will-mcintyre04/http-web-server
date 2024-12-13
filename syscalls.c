#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/dir.h>
#include <string.h>

typedef struct { /* portable directory entry */
    long ino; /* inode number */
    char name[14+1]; /* name + '\0' terminator */
} Dirent; 

void dirwalk(char *dir, void (*fcn)(char *)) {
    DIR *dfd;
    struct dirent *dp;
    char name[1024];
    
    if ((dfd = opendir(dir)) == NULL){
        fprintf(stderr, "dirwalk: cannot open %s\n", dir);
        return;
    }
    while((dp = readdir(dfd)) != NULL){
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
    continue;
}
        if(strlen(dir)+strlen(dp->d_name)+2 > sizeof(name)){
            fprintf(stderr, "dirwalk: name %s %s too long\n", dir, dp->d_name);
        }
        else{
            sprintf(name, "%s/%s", dir, dp->d_name);
            (*fcn)(name);
        }
        }
        closedir(dfd);
    }

void fsize(char * name){
    struct stat stbuf;

    if(stat(name, &stbuf) == -1){
        fprintf(stderr, "fsize: an't access %s\n", name);
        return;
    }

    if((stbuf.st_mode & __S_IFMT) == __S_IFDIR){ 
        dirwalk(name, fsize);
    }
    printf("%8ld %s\n", stbuf.st_size,name);
}

void printFileInfo(const char *filename) {
    struct stat fileStat;

    // Retrieve file information using stat()
    if (stat(filename, &fileStat) == 0) {
        // File information
        printf("File: %s\n", filename);

        // File type and permissions
        printf("File type: ");
        printf("%d\n", fileStat.st_mode);
        if (S_ISREG(fileStat.st_mode)) {
            printf("Regular file\n");
        } else if (S_ISDIR(fileStat.st_mode)) {
            printf("Directory\n");
        } else if (S_ISCHR(fileStat.st_mode)) {
            printf("Character device\n");
        } else if (S_ISBLK(fileStat.st_mode)) {
            printf("Block device\n");
        } else if (S_ISFIFO(fileStat.st_mode)) {
            printf("FIFO/pipe\n");
        } else if (S_ISLNK(fileStat.st_mode)) {
            printf("Symbolic link\n");
        } else if (S_ISSOCK(fileStat.st_mode)) {
            printf("Socket\n");
        } else {
            printf("Unknown type\n");
        }

        printf("Permissions: %o\n", fileStat.st_mode & 0777);  // Octal format

        // Number of hard links
        printf("Number of hard links: %ld\n", fileStat.st_nlink);

        // Owner and group IDs
        printf("Owner UID: %d\n", fileStat.st_uid);
        printf("Owner GID: %d\n", fileStat.st_gid);
        printf("Number: %ld\n", fileStat.st_ino);

        // File size in bytes
        printf("File size: %ld bytes\n", fileStat.st_size);

        // Time of last access
        printf("Last accessed: %s", ctime(&fileStat.st_atime));

        // Time of last modification
        printf("Last modified: %s", ctime(&fileStat.st_mtime));

        // Time of last status change
        printf("Last status change: %s", ctime(&fileStat.st_ctime));

        // Block size for file system I/O
        printf("Block size: %ld\n", fileStat.st_blksize);

        // Number of disk blocks allocated
        printf("Number of blocks: %ld\n", fileStat.st_blocks);
    } else {
        // Error handling if stat() fails
        perror("Error retrieving file information");
    }
}


int main (int argc, char * argv[]){
    __uint8_t test = 254;
    printf("%d\n\n", test);

    char buf[BUFSIZ];
    int n;
    int fd;

    fd = open("server.c", O_RDONLY);

    if (fd == -1){
        perror("Error opening file");
        return 1;
    }
    
    n = read(fd, buf, BUFSIZ);
    write(1, buf, n);
    close(fd);

    printFileInfo("server.c");

    fsize("testdir");

    FILE * file;
    file = fopen("server.c", "r");

    // Create a buffer to store the data we read from the file
    char buffer[5];
    
    // Use fread to read up to 255 characters (leaving space for the null terminator)
    size_t bytesRead = fread(buffer, sizeof(char), sizeof(buffer) - 1, file);

    // Null-terminate the string manually
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';  // Null-terminate the string
        printf("Text read from file: %s\n", buffer);
    } else {
        printf("Error or end of file reached\n");
    }

    char ch;
    if(fseek(file, 2, SEEK_SET) == 0){
        ch = fgetc(file);
        printf("%c\n", ch);
    }

    printf("%ld\n", ftell(file));

    fclose(file);  // Close the file

    return 0;
}