/* myFind.c

This function recursively lists all files starting with the given directory.
The output will list information about each of the files.  The user has the 
option to limit the listed files by modification time and owner.  Additionally,
the '-x' option forces the search to stay within the filesystem, and the '-l'
will only print symlinks to chosen target.

USAGE:
	myFind [-u uid/uname] [-m mtime] [-x] [-l target] startDirectory

Rappaport, Elliot D
ECE357: Operating Systems
October 9, 2013
*/

#include <dirent.h>
#include <errno.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

int desired_user, desired_mtime, require_user_check, require_mtime_check,
current_time, volStay, desiredVol, targetLim, desiredTarget;

int isnumeric( char *str ){
//loop through digits and confirm each one is num; also allow '-' in first digit
	if(!isdigit(*str) && strncmp(str, "-", 1)) return 0;
	str++;
    while(*str){
    	if(!isdigit(*str)) return 0;   
        str++;
	}
    return 1;
}

void int2perm(int integer, int loc, char *perm){
//get least-sig 9 binary digits from number and convert to 'rwx' string
	char p[] = "rwxrwxrwx";
	int y = integer%2;
	if (y==1) perm[loc] = p[loc]; else perm[loc] = *"-";
	if (loc>0) int2perm(integer/2, loc-1, perm);
	perm[9]=*"\0";
}

void makePath(char *starting_path, char *name, char **fullPath){
	size_t sp_len = strlen(starting_path); size_t n_len = strlen(name);
	*fullPath = malloc(sp_len+n_len+2);
	if (fullPath == NULL){
		fprintf(stderr, "Insufficient memory to make path %s\n", *fullPath);
		exit(EXIT_FAILURE);
	}
	strcpy(*fullPath, starting_path);
	strcat(*fullPath, "/");	strcat(*fullPath, name);
}

void printInfo(struct stat sb, char *fullPath){
	if (require_user_check == 1 && (int) sb.st_uid != desired_user) return;
	if (require_mtime_check == 1 && (current_time-sb.st_mtime) < desired_mtime ) return;
	if (require_mtime_check == 2 && !((current_time-sb.st_mtime + desired_mtime) <= 0) ) return;
	if (targetLim == 1){
		struct stat lb;
		if (stat(fullPath, &lb) != 0) fprintf(stderr, "Error stat-ing %s\n%s\n", fullPath, strerror(errno));
		if (lb.st_ino != desiredTarget) return;
	}

	char itype[] = "-dcbpls";
	int file_type = (S_ISREG(sb.st_mode)*1)+(S_ISDIR(sb.st_mode)*2)+(S_ISCHR(sb.st_mode)*3)+
		(S_ISBLK(sb.st_mode)*4)+(S_ISFIFO(sb.st_mode)*5)+(S_ISLNK(sb.st_mode)*6)+(S_ISSOCK(sb.st_mode)*7);
	char perms[10]; int2perm((int) sb.st_mode, 8, perms);
	if(S_ISVTX & sb.st_mode){
		if(!strcmp(&perms[9], "x")) perms[9]=*"t";
		else perms[9]=*"T";
	}
	if(S_ISGID & sb.st_mode){
		if(!strcmp(&perms[6], "x")) perms[9]=*"s";
		else perms[6]=*"S";
	}
	if(S_ISUID & sb.st_mode){
		if(!strcmp(&perms[3], "x")) perms[9]=*"s";
		else perms[3]=*"S";
	}
	struct passwd *ui; struct group *gi;
	ui = getpwuid(sb.st_uid);
	if(ui == NULL){
		fprintf(stderr, "Error finding user infomration for: %s\n", fullPath);
		exit(EXIT_FAILURE);
	}
	gi = getgrgid(sb.st_gid);
	if(gi == NULL){
		fprintf(stderr, "Error finding group infomration for: %s\n", fullPath);
		exit(EXIT_FAILURE);
	}
	char *userName = ui->pw_name;
	char *groupName = gi->gr_name;
	char *timeString = ctime(&sb.st_mtime)+4;
	size_t timeL = strlen(timeString); timeString[timeL-1] = *" ";
	
	printf("%04x/%ld\t%c%s  %ld", (uint) sb.st_dev, (long) sb.st_ino, itype[file_type-1], perms, (long) sb.st_nlink);
	if (userName != NULL) printf(" %s", userName); else printf(" %i", sb.st_uid);
	if (groupName != NULL) printf(" %s", groupName); else printf(" %i", sb.st_gid);
	if (file_type != 3 && file_type != 4) printf("\t%lld", (long long) sb.st_size);
	else printf("%04x", (uint) sb.st_rdev);
	printf("\t%s  %s\n", timeString, fullPath);
	if (S_ISLNK(sb.st_mode)){
		char *linkname = malloc(sb.st_size+1);
		if (linkname == NULL){
			fprintf(stderr, "Insufficient memory to resolve linkage of %s\n", fullPath);
			exit(EXIT_FAILURE);
		}
		ssize_t r = readlink(fullPath, linkname, sb.st_size+1);
		if (r < 0){
			fprintf(stderr, "Error reading link of %s\n%s", fullPath, strerror(errno));
			exit(EXIT_FAILURE);
		}
		linkname[sb.st_size] = '\0';
		printf("\t-> %s\n", linkname);
	}
}

void list_dir(char *starting_path){
	struct dirent *de;
	DIR *dirp=opendir(starting_path);
	if(!(dirp)){
		fprintf(stderr, "Cannot open %s.\n%s\n", starting_path, strerror(errno));
		exit(EXIT_FAILURE);
	}

	while(de=readdir(dirp)){
		struct stat sb; char *fullPath;
		if (!strcmp(de->d_name, "..") || !strcmp(de->d_name, ".")) continue;
		makePath(starting_path, de->d_name, &fullPath);
		if (lstat(fullPath, &sb) != 0) fprintf(stderr, "Error stat-ing %s\n%s\n", fullPath, strerror(errno));
		if (volStay == 1 && sb.st_dev != desiredVol){
			fprintf(stderr, "note: not crossing mount point at %s\n", fullPath);
			continue;
		}
		printInfo(sb, fullPath);

		if ((sb.st_mode&S_IFMT)==S_IFDIR){
			list_dir(fullPath);
		}
	}
	if (errno !=0) fprintf(stderr, "%s\n", strerror(errno));
	closedir(dirp);
}

int main(int argc, char *argv[])
{
	int opt;
	struct passwd *s;
	while((opt = getopt(argc, argv, "u:m:xl:")) != -1){
		switch(opt){
			case 'u':
				require_user_check = 1;
				if(isnumeric(optarg)) s = getpwuid( (uid_t) atoi(optarg));
				else s = getpwnam(optarg);
				if(s == NULL){
					fprintf(stderr, "Error getting finding user with user selection: %s\n", optarg);
					exit(EXIT_FAILURE);
				}
				else desired_user = s->pw_uid; 
				break;
			case 'm':
				if(isnumeric(optarg)) desired_mtime = atoi(optarg);
				else {
					fprintf(stderr, "mtime %s is not a valid time.\nmtime must be an integer.\n", optarg);
					exit(EXIT_FAILURE);
				}
				if (desired_mtime<0) require_mtime_check = 2; else require_mtime_check = 1;
				current_time = time(0);
				break;
			case 'x':
				volStay = 1;
				break;
			case 'l':
				targetLim = 1;
				struct stat targ;
				if (lstat(optarg, &targ) != 0) fprintf(stderr, "Error stat-ing %s\n%s\n", optarg, strerror(errno));
				desiredTarget = targ.st_ino;
				break;
			default:
				fprintf(stderr, "Usage error!\nUsage %s [-u uid/unam] [-m mtime] [-x] [-l target] path\n", argv[0]);
				exit(EXIT_FAILURE);
				break;
		}
	}
   if (optind >= argc) {
       fprintf(stderr, "Expected argument after options.\nUser must enter starting search path.");
       exit(EXIT_FAILURE);
   }

   	struct stat sb;
	if (lstat(argv[optind], &sb) != 0) fprintf(stderr, "Error stat-ing %s\n%s\n", argv[optind], strerror(errno));
	desiredVol = sb.st_dev; //if -x used, we will remain in this volume
	printInfo(sb, argv[optind]);

	list_dir(argv[optind]);
	return 0;
}