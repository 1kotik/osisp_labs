#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>

#define MAX_RECORDS 10
#define RECORD_SIZE sizeof(Record)

char nameExample[10][20] = {"Alex", "Victor", "Zach", "John", "Shane", "Bob", "Chloe", "Peter", "Joe", "David"};

char addressExample[10][20] = {"Moscow", "Minsk", "New York", "Paris", "Dublin", "Berlin", "London", "Prague", "Barcelona", "Madrid"};

typedef struct {
    char name[20];
    char address[20];
    uint8_t semester;
}Record;

typedef struct {
    Record record;
    int recordID;
}CurrentRecord;

CurrentRecord currentRecord;

void printRecord(Record* record, int recordNumber);
void readRecord(int fd, int recordNumber, Record *record);
void writeRecord(int fd, int recordNumber, Record *record);
void lockRecord(int fd, int recordNumber);
void unlockRecord(int fd, int recordNumber);
Record generateRecord();
void chooseOption(int fd);
void printFile(int fd);
void getRecord(int fd, int recordNumber);
void fillInfo(int fd);
void modify(int fd, Record* record);
void put(int fd, Record record);

int main(int argc, char* argv[]) {
    srand(time(NULL));
    if(argc!=2){
        printf("Provide file name\n");
    }
    int fd = open(argv[1], O_RDWR | O_CREAT);
    //FILE* file = fopen(argv[1], "w+b");
    //int fd = fileno(file);
    chooseOption(fd);
    close(fd);

    return 0;
}

void chooseOption(int fd){
    char option[10];
    Record record;
    puts("l - list all records");
    puts("g<num> - get <num> record and make it current");
    puts("f - fill file with records");
    puts("m - modify current record (with  random data)");
    puts("p - complete modifying");
    do{
        fgets(option,10,stdin);
        switch(option[0]){
        case 'l':
            printFile(fd);
            break;
        case 'g':
            int recordNumber = atoi(option+1);
            getRecord(fd, recordNumber);
            break;
        case 'f':
            fillInfo(fd);
            break;
        case 'm':
            modify(fd, &record);
            break;
        case 'p':
            put(fd, record);
            break;
        }
        rewind(stdin);
    }while(option[0]!='q');
    exit(EXIT_SUCCESS);
}

void printFile(int fd){
    for (int i = 0; i < MAX_RECORDS; i++) {
        Record record;
        readRecord(fd, i, &record);
        printRecord(&record, i);
        }
}
void getRecord(int fd, int recordNumber){
    Record record;
    readRecord(fd, recordNumber, &record);
    printf("Record %d\n", recordNumber);
    printRecord(&record, recordNumber);
    currentRecord.record = record;
    currentRecord.recordID = recordNumber;
}
void fillInfo(int fd){
    Record records[MAX_RECORDS];
    for (int i = 0; i < MAX_RECORDS; i++) {
        records[i] = generateRecord();
    }

    lseek(fd, 0, SEEK_SET);
    write(fd, records, sizeof(records));
}
void modify(int fd, Record* record){
    Record REC_SAV;
    *record = generateRecord();
    puts("New record:");
    printf("Name:     %s\n", record->name);
    printf("Address:  %s\n", record->address);
    printf("Semester: %d\n", record->semester);
    puts("Press any key to continue");
    getchar();
            
    int flag = 0;
    do {
        if (flag == 1) {
            break;;
        }

        flag = 0;
        REC_SAV = currentRecord.record;
        lockRecord(fd, currentRecord.recordID);
        Record REC_NEW;
        readRecord(fd, currentRecord.recordID, &REC_NEW);

        if (memcmp(&REC_NEW, &REC_SAV, sizeof(Record)) != 0) {
            puts("Records are not equal");
            unlockRecord(fd, currentRecord.recordID);
            currentRecord.record = REC_NEW;
            flag = 1;
        }
    } while(flag);

    if (flag == 1) {
        puts("Current record:");
        printf("Record ID %d:\n", currentRecord.recordID);
        printf("Name:     %s\n", currentRecord.record.name);
        printf("Address:  %s\n", currentRecord.record.address);
        printf("Semester: %d\n", currentRecord.record.semester);
        return;
    }

    puts("Enter 'p' to to put record");
}
void put(int fd, Record record){
    puts("Press any key to put record");
    getchar();
    writeRecord(fd, currentRecord.recordID, &record);
    unlockRecord(fd, currentRecord.recordID);
    puts("Completed");
}

void printRecord(Record *record, int recordNumber) {
    printf("Record    %d\n", recordNumber);
    printf("Name:     %s\n", record->name);
    printf("Address:  %s\n", record->address);
    printf("Semester: %d\n\n", record->semester);
}

void readRecord(int fd, int recordNumber, Record *record) {
    lseek(fd, recordNumber * RECORD_SIZE, SEEK_SET);
    read(fd, record, RECORD_SIZE);
}

void writeRecord(int fd, int recordNumber, Record *record) {
    lseek(fd, recordNumber * RECORD_SIZE, SEEK_SET);
    write(fd, record, RECORD_SIZE);
}

void lockRecord(int fd, int recordNumber) {
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = recordNumber * RECORD_SIZE;
    lock.l_len = RECORD_SIZE;

    fcntl(fd, F_SETLK, &lock);

    int flags = fcntl(fd, F_GETFL); // Получение текущих флагов состояния файла
    flags |= O_NONBLOCK; // Добавление флага O_NONBLOCK
    fcntl(fd, F_SETFL, flags);
}

void unlockRecord(int fd, int recordNumber) {
    struct flock lock;
    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = recordNumber * RECORD_SIZE;
    lock.l_len = RECORD_SIZE;

    fcntl(fd, F_SETLK, &lock);

    int flags = fcntl(fd, F_GETFL); // Получение текущих флагов состояния файла
    flags |= O_NONBLOCK; // Добавление флага O_NONBLOCK
    fcntl(fd, F_SETFL, flags);
}

Record generateRecord() {
    Record record;
    strcpy(record.name, nameExample[rand() % 10]);
    strcpy(record.address, addressExample[rand() % 10]);
    record.semester = rand() % 8 + 1;

    return record;
}

