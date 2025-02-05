#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>

#include "db.h"
#include "sdbsc.h"

int open_db(char *dbFile, bool should_truncate) {
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
    int flags = O_RDWR | O_CREAT;

    if (should_truncate)
        flags |= O_TRUNC;

    int fd = open(dbFile, flags, mode);
    if (fd == -1) {
        printf(M_ERR_DB_OPEN);
        return ERR_DB_FILE;
    }
    return fd;
}

int get_student(int fd, int id, student_t *s) {
    off_t offset = id * STUDENT_RECORD_SIZE;
    
    if (lseek(fd, offset, SEEK_SET) == -1) {
        return ERR_DB_FILE;
    }
    
    ssize_t bytes_read = read(fd, s, STUDENT_RECORD_SIZE);
    if (bytes_read != STUDENT_RECORD_SIZE) {
        return ERR_DB_FILE;
    }
    
    if (memcmp(s, &EMPTY_STUDENT_RECORD, STUDENT_RECORD_SIZE) == 0) {
        return SRCH_NOT_FOUND;
    }
    
    if (s->id == id) {
        return NO_ERROR;
    }
    
    return SRCH_NOT_FOUND;
}

int count_db_records(int fd) {
    student_t student;
    int count = 0;
    
    if (lseek(fd, 0, SEEK_SET) == -1) {
        printf(M_ERR_DB_READ);
        return ERR_DB_FILE;
    }
    
    while (read(fd, &student, STUDENT_RECORD_SIZE) == STUDENT_RECORD_SIZE) {
        if (memcmp(&student, &EMPTY_STUDENT_RECORD, STUDENT_RECORD_SIZE) != 0) {
            count++;
        }
    }
    
    if (count == 0) {
        printf(M_DB_EMPTY);
    } else {
        printf(M_DB_RECORD_CNT, count);
    }
    
    return count;
}

int add_student(int fd, int id, char *fname, char *lname, int gpa) {
    student_t new_student = {0};
    student_t existing_student = {0};
    
    int rc = get_student(fd, id, &existing_student);
    if (rc == NO_ERROR) {
        printf(M_ERR_DB_ADD_DUP, id);
        return ERR_DB_OP;
    }
    
    new_student.id = id;
    strncpy(new_student.fname, fname, sizeof(new_student.fname) - 1);
    strncpy(new_student.lname, lname, sizeof(new_student.lname) - 1);
    new_student.gpa = gpa;
    
    off_t offset = id * STUDENT_RECORD_SIZE;
    if (lseek(fd, offset, SEEK_SET) == -1) {
        printf(M_ERR_DB_WRITE);
        return ERR_DB_FILE;
    }
    
    if (write(fd, &new_student, STUDENT_RECORD_SIZE) != STUDENT_RECORD_SIZE) {
        printf(M_ERR_DB_WRITE);
        return ERR_DB_FILE;
    }
    
    printf(M_STD_ADDED, id);
    return NO_ERROR;
}

int del_student(int fd, int id) {
    student_t student = {0};
    
    int rc = get_student(fd, id, &student);
    if (rc != NO_ERROR) {
        printf(M_STD_NOT_FND_MSG, id);
        return ERR_DB_OP;
    }
    
    off_t offset = id * STUDENT_RECORD_SIZE;
    if (lseek(fd, offset, SEEK_SET) == -1) {
        printf(M_ERR_DB_WRITE);
        return ERR_DB_FILE;
    }
    
    if (write(fd, &EMPTY_STUDENT_RECORD, STUDENT_RECORD_SIZE) != STUDENT_RECORD_SIZE) {
        printf(M_ERR_DB_WRITE);
        return ERR_DB_FILE;
    }
    
    printf(M_STD_DEL_MSG, id);
    return NO_ERROR;
}

void print_student(student_t *s) {
    if (s == NULL || s->id == 0) {
        printf(M_ERR_STD_PRINT);
        return;
    }
    
    float gpa = s->gpa / 100.0;
    printf(STUDENT_PRINT_HDR_STRING, "ID", "FIRST_NAME", "LAST_NAME", "GPA");
    printf(STUDENT_PRINT_FMT_STRING, s->id, s->fname, s->lname, gpa);
}

int print_db(int fd) {
    student_t student = {0};
    bool is_empty = true;
    
    if (lseek(fd, 0, SEEK_SET) == -1) {
        printf(M_ERR_DB_READ);
        return ERR_DB_FILE;
    }
    
    while (read(fd, &student, STUDENT_RECORD_SIZE) == STUDENT_RECORD_SIZE) {
        if (memcmp(&student, &EMPTY_STUDENT_RECORD, STUDENT_RECORD_SIZE) != 0) {
            is_empty = false;
            break;
        }
    }
    
    if (is_empty) {
        printf(M_DB_EMPTY);
        return NO_ERROR;
    }
    
    lseek(fd, 0, SEEK_SET);
    printf(STUDENT_PRINT_HDR_STRING, "ID", "FIRST_NAME", "LAST_NAME", "GPA");
    
    while (read(fd, &student, STUDENT_RECORD_SIZE) == STUDENT_RECORD_SIZE) {
        if (memcmp(&student, &EMPTY_STUDENT_RECORD, STUDENT_RECORD_SIZE) != 0) {
            float gpa = student.gpa / 100.0;
            printf(STUDENT_PRINT_FMT_STRING, student.id, student.fname, student.lname, gpa);
        }
    }
    
    return NO_ERROR;
}

int compress_db(int fd) {
    student_t student = {0};
    int tmp_fd;
    
    tmp_fd = open_db(TMP_DB_FILE, true);
    if (tmp_fd < 0) {
        printf(M_ERR_DB_OPEN);
        return ERR_DB_FILE;
    }
    
    if (lseek(fd, 0, SEEK_SET) == -1) {
        close(tmp_fd);
        printf(M_ERR_DB_READ);
        return ERR_DB_FILE;
    }
    
    while (read(fd, &student, STUDENT_RECORD_SIZE) == STUDENT_RECORD_SIZE) {
        if (memcmp(&student, &EMPTY_STUDENT_RECORD, STUDENT_RECORD_SIZE) != 0) {
            off_t offset = student.id * STUDENT_RECORD_SIZE;
            
            if (lseek(tmp_fd, offset, SEEK_SET) == -1 ||
                write(tmp_fd, &student, STUDENT_RECORD_SIZE) != STUDENT_RECORD_SIZE) {
                close(tmp_fd);
                printf(M_ERR_DB_WRITE);
                return ERR_DB_FILE;
            }
        }
    }
    
    close(fd);
    close(tmp_fd);
    
    if (rename(TMP_DB_FILE, DB_FILE) != 0) {
        printf(M_ERR_DB_CREATE);
        return ERR_DB_FILE;
    }
    
    fd = open_db(DB_FILE, false);
    if (fd < 0) {
        printf(M_ERR_DB_OPEN);
        return ERR_DB_FILE;
    }
    
    printf(M_DB_COMPRESSED_OK);
    return fd;
}

int validate_range(int id, int gpa) {
    if ((id < MIN_STD_ID) || (id > MAX_STD_ID))
        return EXIT_FAIL_ARGS;

    if ((gpa < MIN_STD_GPA) || (gpa > MAX_STD_GPA))
        return EXIT_FAIL_ARGS;

    return NO_ERROR;
}

void usage(char *exename) {
    printf("usage: %s -[h|a|c|d|f|p|z] options.  Where:\n", exename);
    printf("\t-h:  prints help\n");
    printf("\t-a id first_name last_name gpa(as 3 digit int):  adds a student\n");
    printf("\t-c:  counts the records in the database\n");
    printf("\t-d id:  deletes a student\n");
    printf("\t-f id:  finds and prints a student in the database\n");
    printf("\t-p:  prints all records in the student database\n");
    printf("\t-x:  compress the database file [EXTRA CREDIT]\n");
    printf("\t-z:  zero db file (remove all records)\n");
}

int main(int argc, char *argv[]) {
    char opt;
    int fd;
    int rc;
    int exit_code;
    int id;
    int gpa;
    student_t student = {0};

    if ((argc < 2) || (*argv[1] != '-')) {
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1] + 1);

    if (opt == 'h') {
        usage(argv[0]);
        exit(EXIT_OK);
    }

    fd = open_db(DB_FILE, false);
    if (fd < 0) {
        exit(EXIT_FAIL_DB);
    }

    exit_code = EXIT_OK;
    switch (opt) {
    case 'a':
        if (argc != 6) {
            usage(argv[0]);
            exit_code = EXIT_FAIL_ARGS;
            break;
        }

        id = atoi(argv[2]);
        gpa = atoi(argv[5]);

        exit_code = validate_range(id, gpa);
        if (exit_code == EXIT_FAIL_ARGS) {
            printf(M_ERR_STD_RNG);
            break;
        }

        rc = add_student(fd, id, argv[3], argv[4], gpa);
        if (rc < 0)
            exit_code = EXIT_FAIL_DB;
        break;

    case 'c':
        rc = count_db_records(fd);
        if (rc < 0)
            exit_code = EXIT_FAIL_DB;
        break;

    case 'd':
        if (argc != 3) {
            usage(argv[0]);
            exit_code = EXIT_FAIL_ARGS;
            break;
        }
        id = atoi(argv[2]);
        rc = del_student(fd, id);
        if (rc < 0)
            exit_code = EXIT_FAIL_DB;
        break;

    case 'f':
        if (argc != 3) {
            usage(argv[0]);
            exit_code = EXIT_FAIL_ARGS;
            break;
        }
        id = atoi(argv[2]);
        rc = get_student(fd, id, &student);

        switch (rc) {
        case NO_ERROR:
            print_student(&student);
            break;
        case SRCH_NOT_FOUND:
            printf(M_STD_NOT_FND_MSG, id);
            exit_code = EXIT_FAIL_DB;
            break;
        default:
            printf(M_ERR_DB_READ);
            exit_code = EXIT_FAIL_DB;
            break;
        }
        break;

    case 'p':
        rc = print_db(fd);
        if (rc < 0)
            exit_code = EXIT_FAIL_DB;
        break;

    case 'x':
        fd = compress_db(fd);
        if (fd < 0)
            exit_code = EXIT_FAIL_DB;
        break;

    case 'z':
        close(fd);
        fd = open_db(DB_FILE, true);
        if (fd < 0) {
            exit_code = EXIT_FAIL_DB;
            break;
        }
        printf(M_DB_ZERO_OK);
        exit_code = EXIT_OK;
        break;

    default:
        usage(argv[0]);
        exit_code = EXIT_FAIL_ARGS;
    }

    close(fd);
    exit(exit_code);
}
