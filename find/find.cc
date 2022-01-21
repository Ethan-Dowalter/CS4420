/**
 *        @file: find.cc
 *      @author: Ethan Dowalter
 *        @date: November 29, 2021
 *       @brief: Program which implements a scaled down version of the unix utility 'find'
 */

#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include <iomanip>
#include <vector>
#include <dirent.h>
#include <cmath>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/statvfs.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <errno.h>
#include <string.h>
using namespace std;

/// Global variables
bool DEBUG = false;         //controlls debugging output
bool NAMEFILTER = false;    //whether or not the -name filter is applied
char *FILENAME;             //filename to search for with -name filter
bool TYPEFILTER = false;    //whether or not the -type filter is applied
unsigned char FILETYPE;     //file type of file to search for with -type filter
                            // f = regular file, d = directory, l = symbolic link
                            // b = block special (for devices), c = character special (for devices)
bool SIZEFILTER = false;    //whether or not the -size filter is applied
string SIZESPEC;            //size specification for filtering files with -size filter
                            // starts with a '-' means true if file is less than size specified
                            // starts with a '+' means true if file is greater than size specified
                            // starts with a digit means true if file is exactly size specified
                            // ends in a 'c' means size specified is in characters instead of blocks

/// Functions
/// Find - recursive function which searches the path for anything that matches the filters
void Find(char *path);
/// These 5 aren't used in the program, they were only used for debugging and were copied from readdir.c and sstat.c
static char *FileModeString(mode_t mode);
static char const *OwnerString(uid_t owner);
static char const *GroupString(uid_t group);
static char *TimeString(time_t date);
void RunStat(char *filename);



int main(int argc, char *argv[]){   
    /// Check for enough args
    if (argc < 2)
    {
        cout << "Not enough args!" << endl;
        exit(1);
    }
    
    /// Read through the arguments given to the program
    for (int i = 2; i < argc; i++)
    {
        if (strcmp(argv[i], "-d") == 0)
        {   /// Debug flag for debugging output
            DEBUG = true;
        }
        else if (strcmp(argv[i], "-name") == 0)
        {   /// Name filter
            /// Check for valid input
            if (strcmp(argv[i + 1], "-d") == 0 || strcmp(argv[i + 1], "-type") == 0 || strcmp(argv[i], "-size") == 0)
            {
                cout << "Error. No filename provided for '-name' filter" << endl;
                cout << "Usage: -name filename" << endl;
                exit(1);
            }
            else
            {
                NAMEFILTER = true;
                FILENAME = (char *) realloc(FILENAME, strlen(argv[i + 1]));
                memcpy(FILENAME, argv[i + 1], strlen(argv[i + 1]));
                i++;
            }
        }
        else if (strcmp(argv[i], "-type") == 0)
        {   /// Type filter
            /// Check for valid input
            if (strcmp(argv[i + 1], "f") != 0 &&
                strcmp(argv[i + 1], "d") != 0 &&
                strcmp(argv[i + 1], "l") != 0 &&
                strcmp(argv[i + 1], "b") != 0 &&
                strcmp(argv[i + 1], "c") != 0)
            {
                cout << "Error. No type provided for '-type' filter" << endl;
                cout << "Usage: -type f|d|l|b|c" << endl;
                exit(1);
            }
            else
            {
                TYPEFILTER = true;
                if (strcmp(argv[i + 1], "f") == 0) FILETYPE = DT_REG;
                else if(strcmp(argv[i + 1], "d") == 0) FILETYPE = DT_DIR;
                else if(strcmp(argv[i + 1], "l") == 0) FILETYPE = DT_LNK;
                else if(strcmp(argv[i + 1], "b") == 0) FILETYPE = DT_BLK;
                else if(strcmp(argv[i + 1], "c") == 0) FILETYPE = DT_CHR;
                else cout << "How did I get here?!" << endl;
                i++;
            }
        }
        else if (strcmp(argv[i], "-size") == 0)
        {   /// Size filter
            /// Check for valid input
            if ((argv[i + 1][0] != '+' && 
                argv[i + 1][0] != '-' && 
                !isdigit(argv[i + 1][0])) || 
                (argv[i + 1][strlen(argv[i + 1]) - 1] != 'c' && 
                !isdigit(argv[i + 1][strlen(argv[i + 1]) - 1])))
            {
                cout << "Error. No size provided for '-size' filter" << endl;
                cout << "Usage: -size [+-]?[0-9]+[c]?" << endl;
                exit(1);
            }
            else
            {
                SIZEFILTER = true;
                SIZESPEC = argv[i + 1];
                i++;
            }
        }
        else
        {
            cout << "Invalid argument used: " << argv[i] << endl;
            exit(1);
        }
    }

    /// Search through the provided directory for all files that satisfy the given arguments
    /// Special case for checking the starting directory
    {
        struct stat statbuf;
        int ret = lstat(argv[1],&statbuf);
        if (ret != 0)
        {
            perror(argv[1]);
            return 0;
        }

        bool ismatch = true;
        if (NAMEFILTER && strcmp(FILENAME, argv[1]) != 0)
        {
            ismatch = false;
        }
        if (TYPEFILTER && FILETYPE != DT_DIR)
        {
            ismatch = false;
        }
        if (SIZEFILTER)
        {
            int size;
            if (SIZESPEC[0] == '-') size = stoi(SIZESPEC.substr(1, SIZESPEC.size() - 1));
            else size = stoi(SIZESPEC);
            if (DEBUG) cout << statbuf.st_size << " " << size << endl;
            if (SIZESPEC[0] == '+')
            {
                if (SIZESPEC[SIZESPEC.size() - 1] == 'c')
                {
                    if (statbuf.st_size <= size) ismatch = false;
                }
                else
                {
                    if (ceil(statbuf.st_size / 512) <= size) ismatch = false;
                }
            }
            else if (SIZESPEC[0] == '-')
            {
                if (SIZESPEC[SIZESPEC.size() - 1] == 'c')
                {
                    if (statbuf.st_size >= size) ismatch = false;
                }
                else
                {
                    if (ceil(statbuf.st_size / 512) >= size) ismatch = false;
                }
            }
            else
            {
                if (SIZESPEC[SIZESPEC.size() - 1] == 'c')
                {
                    if (statbuf.st_size != size) ismatch = false;
                }
                else
                {
                    if (ceil(statbuf.st_size / 512) != size) ismatch = false;
                }
            }
        }
        if (ismatch) cout << argv[1] << endl;
    }
    // char *ch = (char *) "testdir";
    // RunStat(ch);
    // ch = (char *) "testdir/gf5k";
    // RunStat(ch);
    Find(argv[1]);

    return 0;
}



void Find(char *path){
    DIR *dp;
    struct dirent *dirp;

    if ((dp = opendir(path)) == NULL)
    {
        perror(path);
        exit(-1);
    }

    // cout << path << endl;

    while ((dirp = readdir(dp)) != NULL)
    {
        /// Check for parent and current directories to skip them
        if (strcmp(dirp->d_name, "..") == 0 || strcmp(dirp->d_name, ".") == 0)
        {
            continue;
        }
        
        char *newpath = (char *) malloc(strlen(path) + strlen(dirp->d_name) + 1);
        memset(newpath, '\0', strlen(path) + strlen(dirp->d_name) + 1);
        memcpy(newpath, path, strlen(path));
        strcat(newpath, "/");
        strcat(newpath, dirp->d_name);


        /// Else, check if file satisfies filters and print it if it does
        struct stat statbuf;
        int ret = lstat(newpath,&statbuf);
        if (ret != 0)
        {
            perror(newpath);
            return;
        }

        bool ismatch = true;
        if (NAMEFILTER && strcmp(FILENAME, dirp->d_name) != 0)
        {
            ismatch = false;
        }
        if (TYPEFILTER && dirp->d_type != FILETYPE)
        {
            ismatch = false;
        }
        if (SIZEFILTER)
        {
            int size;
            if (SIZESPEC[0] == '-') size = stoi(SIZESPEC.substr(1, SIZESPEC.size() - 1));
            else size = stoi(SIZESPEC);
            if (DEBUG) cout << statbuf.st_size << " " << size << endl;
            if (SIZESPEC[0] == '+')
            {
                if (SIZESPEC[SIZESPEC.size() - 1] == 'c')
                {
                    if (statbuf.st_size <= size) ismatch = false;
                }
                else
                {
                    if (ceil(statbuf.st_size / 512) <= size) ismatch = false;
                }
            }
            else if (SIZESPEC[0] == '-')
            {
                if (SIZESPEC[SIZESPEC.size() - 1] == 'c')
                {
                    if (statbuf.st_size >= size) ismatch = false;
                }
                else
                {
                    if (ceil(statbuf.st_size / 512) >= size) ismatch = false;
                }
            }
            else
            {
                if (SIZESPEC[SIZESPEC.size() - 1] == 'c')
                {
                    if (statbuf.st_size != size) ismatch = false;
                }
                else
                {
                    if (ceil(statbuf.st_size / 512) != size) ismatch = false;
                }
            }
        }

        if (ismatch) cout << newpath << endl;

        /// If dirp is a directory, then recurse
        if (dirp->d_type == DT_DIR)
        {
            if (DEBUG) cout << "found a dir" << endl;
            Find(newpath);
        }
    }

    if (closedir(dp) == -1)
        perror("closedir");
}

void RunStat(char *filename){
    struct stat statbuf;
    int ret;
    int ftype;

	ret = lstat(filename,&statbuf);

    if (ret != 0) {
		perror(filename);
		return;
    }


    ftype = statbuf.st_mode & S_IFMT;

    printf("File: %s  <%s>\n",
	   filename,
	   (ftype == S_IFIFO)?"fifo":
	   (ftype == S_IFCHR)?"character special":
	   (ftype == S_IFDIR)?"directory":
	   (ftype == S_IFBLK)?"block special":
	   (ftype == S_IFREG)?"regular file":
	   (ftype == S_IFLNK)?"symbolic link":
	   (ftype == S_IFSOCK)?"socket":
	   "UNKNOWN/OTHER");

    printf("Mode: %s   owner: %s(%d)  group: %s(%d)\n",
	   FileModeString(statbuf.st_mode),
	   OwnerString(statbuf.st_uid), (int)statbuf.st_uid,
	   GroupString(statbuf.st_gid), (int)statbuf.st_gid);

    printf("Phys: inode %ld, %ld link, %ld blocks\n",
	   statbuf.st_ino,
	   statbuf.st_nlink,
	   statbuf.st_blocks);

    printf("Size: %ld bytes, I/O block size %ld\n",
	   statbuf.st_size,
	   statbuf.st_blksize);
	   
    printf("Access: %s\n", TimeString(statbuf.st_atime));
    printf("Change: %s\n", TimeString(statbuf.st_mtime));
    printf("Status: %s\n", TimeString(statbuf.st_ctime));
}


static char *FileModeString(mode_t mode){
    static char buf[100];
    
    sprintf(buf,
	    "%c%c%c%c%c%c%c%c%c",

	    (mode & (S_IREAD))? 'r':'-',
	    (mode & (S_IWRITE))?'w':'-',
	    (mode & (S_IEXEC))?
  	      ((mode & S_ISUID)?'s':'x'):'-',

	    (mode & (S_IREAD>>3))? 'r':'-',
	    (mode & (S_IWRITE>>3))?'w':'-',
	    (mode & (S_IEXEC>>3))?
  	      ((mode & S_ISGID)?'s':'x'):'-',

	    (mode & (S_IREAD>>6))? 'r':'-',
	    (mode & (S_IWRITE>>6))?'w':'-',
	    (mode & (S_IEXEC>>6))? 'x':'-'

	);

    return(buf);
}


static char *TimeString(time_t date){
    static char buf[100];
    char *pch;

    pch = ctime(&date);
    pch[24] = '\00';

    sprintf(buf,"%s", pch);

    return(buf);
}


static char const *OwnerString(uid_t owner){
    struct passwd *ppass;

    if ((ppass = getpwuid(owner)) == NULL)
    {
        char const *c = "NOUSER";
		return(c);
    }

    return(strdup(ppass->pw_name));
}

static char const *GroupString(uid_t group){
    struct group *pgroup;

    if ((pgroup = getgrgid(group)) == NULL)
    {
		char const *c = "NOGROUP";
        return(c);
    }

    return(strdup(pgroup->gr_name));
}