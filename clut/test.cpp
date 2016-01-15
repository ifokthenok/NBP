#include <iostream>
#include "clut.h"

using namespace clut;
int main(int argc, char* argv[]) {
    openCLInit();
    
    char host[1024] = {0};
    cl_mem buffer = openCLCreateBuffer(sizeof(host));
    openCLWriteBuffer(buffer, host, sizeof(host));
    openCLReadBuffer(buffer, host, sizeof(host));
    openCLReleaseMemObject(buffer);
    
    
    cl_program program = openCLCreateProgram(argv[1], argv[2]);
    
    cl_kernel kernel = openCLCreateKernel(program, argv[3]);
    openCLReleaseKernel(kernel);

    openCLReleaseProgram(program);
    
    
    openCLDestroy();
    return 0;
}
