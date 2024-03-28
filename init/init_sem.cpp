
#include <iostream>
#include <semaphore.h>
#include <fcntl.h>   // O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define SERVER_SEMAPHORE "control_server"

int main(int argc, char const *argv[]) {
    sem_unlink(SERVER_SEMAPHORE);
    return 0;
}
