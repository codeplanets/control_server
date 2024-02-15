#include <iostream>
#include <unistd.h>

using namespace std;

int main(int argc, char *argv[]) {
    
    /**
     * Setting named semaphore ( <== MUTEX )
     * Loading ini file
     * Getting ip, port....
     * Clear memory
     * Create parents server listener socket
     * ``` 
     * listener = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
     * 
     * ```
     * 
    **/

    pid_t pid = fork();

    if (pid == 0)
        cout << "Output from the child process." << endl;
    else
        cout << "Output from the parent process." << endl;

    return 0;
}
