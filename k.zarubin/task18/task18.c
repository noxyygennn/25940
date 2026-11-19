#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

int main(int argc, char *argv[]){
    struct stat st;

    for (int i = 1; i < argc; i++){
        if (lstat(argv[i], &st) == -1){
            perror(argv[i]);
            continue;
        }
        /* тип файла (каталог/файл/другое) */
        if (S_ISDIR(st.st_mode)) putchar('d');
        else if (S_ISREG(st.st_mode)) putchar('-');
        else putchar('?');

        /* права хозяйна/группы/других */
        char roots[10];
        roots[0] = (st.st_mode & S_IRUSR) ? 'r' : '-';
        roots[1] = (st.st_mode & S_IWUSR) ? 'w' : '-';
        roots[2] = (st.st_mode & S_IXUSR) ? 'x' : '-';
        roots[3] = (st.st_mode & S_IRGRP) ? 'r' : '-';
        roots[4] = (st.st_mode & S_IWGRP) ? 'w' : '-';
        roots[5] = (st.st_mode & S_IXGRP) ? 'x' : '-';
        roots[6] = (st.st_mode & S_IROTH) ? 'r' : '-';
        roots[7] = (st.st_mode & S_IWOTH) ? 'w' : '-';
        roots[8] = (st.st_mode & S_IXOTH) ? 'x' : '-';
        roots[9] = '\0';

        for (int j = 0; j < 9; j++){ putchar(roots[j]); }

        /* количество связей файла */
        printf(" %3lu ", (unsigned long)st.st_nlink);

        /* имена владельца и группы */
        struct passwd *user; struct group *grp;
        user = getpwuid(st.st_uid); grp = getgrgid(st.st_gid);

        if (user != NULL) printf("%-8s ", user->pw_name);
        else printf("%-8lu ", (unsigned long)st.st_uid);
        if (grp != NULL) printf("%-8s ", grp->gr_name);
        else printf("%-8lu ", (unsigned long)st.st_gid);

        /* размер файла(!) */
        if (S_ISREG(st.st_mode)) printf("%10lld ", (long long)st.st_size);
        else printf("%10s ", "");

        /* дата */
        char timebuf[32];
        strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", localtime(&st.st_mtime));
        printf("%s ", timebuf);

        /* имя файла */
        const char *name = strrchr(argv[i], '/');
        printf("%s\n", name != NULL ? name + 1 : argv[i]);
    }

    return 0;
}
