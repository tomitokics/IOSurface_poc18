//
//  poc.h
//  explt
//
//  Created by SalupovTech on 30/11/2024.
//

#ifndef POC_H
#define POC_H

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

extern io_connect_t glob_conn;
extern char *inStr;
extern size_t len;

void *race1();
void *race2();
int run();

#endif // POC_H
