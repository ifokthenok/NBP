#include <vector>
#include <string>

#include "utils.h"
#include "log.h"
#include "process.h"

#define TAG "main"

using namespace std;

int main() {
    
    string file = "/bin/sh";
    vector<string> args = {"sh", "-c", "sleep 1; read x; echo $x; echo hello err 1>&2; sleep 3"};

    Process process;
    process.create(file, args, "in.txt", "out.txt", "err.txt");
    process.dump();

    vector<string> a2 = {"sh", "-c", "echo hello out > in.txt"};
    Process p2;
    p2.create(file, a2, "", "", "");
    p2.dump();

    
    LOGD("main", "isAlive(): %d", process.isAlive() ? 1 : 0);
    sleep(3);
    LOGD("main", "isAlive(): %d", process.isAlive() ? 1 : 0);
    process.kill();
    sleep(1);
    LOGD("main", "isAlive(): %d", process.isAlive() ? 1 : 0);
    
    return 0;
}
