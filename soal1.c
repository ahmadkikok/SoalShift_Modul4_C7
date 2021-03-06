#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>

static const char *dirpath = "/home/ahmadkikok";

/*
static int xmp_cp(const char *to, const char *from){

    int fd_to, fd_from;
    char buf[1000];
    ssize_t nread;
    int saved_errno;

    if(strcmp(get_filename_ext(fd_from),"copy")==0){
	sprintf(cp, "%s%s.copy",fd_to,fd_from);
	system("zenity --error --text='File yang anda buka adalah file hasil salinan. File tidak bisa diubah maupun disalin kembali!'");
	return -1;
    }

    fd_from = open(from, O_RDONLY);
    if (fd_from < 0)
        return -1;

    fd_to = open(to, O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (fd_to < 0) goto out_error;
	
    while (nread = read(fd_from, buf, sizeof buf), nread > 0){
        char *out_ptr = buf;
        ssize_t nwritten;

        do {
            nwritten = write(fd_to, out_ptr, nread);

            if (nwritten >= 0){
                nread -= nwritten;
                out_ptr += nwritten;
            }
            else if (errno != EINTR){
                goto out_error;
            }
        } while (nread > 0);
    }

    if (nread == 0){
        if (close(fd_to) < 0){
            fd_to = -1;
            goto out_error;
        }
        close(fd_from);
    }
        return 0;
}
//up reference to https://stackoverflow.com/questions/2180079/how-can-i-copy-a-file-on-unix-using-c

static int xmp_chmod(const char *path, mode_t mode){
        int res;
        res = chmod(path, mode);
        if (res == -1)
                return -errno;
        return 0;

}

*/

static int xmp_mkdir(const char *path, mode_t mode)
{
	int res;

	res = mkdir(path, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rename(const char *from, const char *to)
{
	int res;

	res = rename(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_getattr(const char *path, struct stat *stbuf)
{
  int res;
	char fpath[1000];
	sprintf(fpath,"%s%s",dirpath,path);
	res = lstat(fpath, stbuf);

	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res = 0;

	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir(fpath);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		res = (filler(buf, de->d_name, &st, 0));
			if(res!=0) break;
	}

	closedir(dp);
	return 0;
}

//Reference https://stackoverflow.com/questions/5309471/getting-file-extension-in-c
const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	char fpath[1000];
	char renam[1000];
	char dirm[1000];

	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);

	if(strcmp(get_filename_ext(path),"doc")==0||
	   strcmp(get_filename_ext(path),"txt")==0||
	   strcmp(get_filename_ext(path),"pdf")==0){
		sprintf(renam, "%s%s.ditandai",dirpath,path);
		sprintf(dirm, "%s%srahasia",dirpath,path);
		system("zenity --error --text='Terjadi kesalahan! File berisi konten berbahaya.'");
		rename(fpath, renam);
		return -1;

	DIR *rahasia;
	rahasia = opendir(dirm);
	if (rahasia==NULL)mkdir(dirm, 0777);

	}


	int res = 0;
	int fd = 0 ;
	(void) fi;

	fd = open(fpath, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.readdir	= xmp_readdir,
	.read		= xmp_read,
	.rename		= xmp_rename,
	.mkdir		= xmp_mkdir,
//	.chmod		= xmp_chmod,
//	.cp		= xmp_cp,
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}

