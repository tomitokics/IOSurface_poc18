# IOSurface UaF (CVE-2024-44285 PoC)
## Introduction
This bug was introduced in iOS 18 and it is relatively shallow as it occurs in the second function after the base `s_method` call.  
The vulnerable function lies in `s_set_corevideo_bridged_keys`, which is method #54 on iOS and #57 on macOS.  

A lock was missing from the pre-patch version. Calling this method from multiple threads it is possible to over-release an OSArray. 
Funnily enough, it seems that that while in the KEXT the locking mechanism was missed, in the userland library this method is correctly called via locks

### A pre-patch version of the vulnerable function can be seen below:
```c
int64_t __fastcall IOSurfaceRootUserClient::set_corevideo_bridged_keys(IOSurfaceRootUserClient* this, uint64_t* structureInput, unsigned int structureInputSize) {
    int64_t v4;            // x21
    OSObject* v5;              // x0
    const OSObject* v6;        // x0
    OSArray* oarray;           // x0
    OSArray* user_oarray;      // x0
    OSArray* arr_from_user_in; // x0

    v4 = 0xE00002C2LL;
    v5 = OSUnserializeXML((const char*)structureInput + 12, structureInputSize - 12LL, 0LL);
    if (v5) {
        v6 = v5;
        oarray = (OSArray*)OSMetaClassBase::safeMetaCast(v5, (const OSMetaClass*)&OSArray::gMetaClass);
        if (oarray) {
            user_oarray = OSArray::withArray(oarray, 0);
            arr_from_user_in = this->corevideo_bridged_keys;
            if (arr_from_user_in)
                arr_from_user_in->release(arr_from_user_in);
            v4 = 0LL;
            this->corevideo_bridged_keys = user_oarray;
        }
        v6->release(v6);
    }
    return v4;
}
```

### Diffing the two versions shows the patch very clearly:
First Version (Second one coming)
```c
int64_t __fastcall IOSurfaceRootUserClient::set_corevideo_bridged_keys(IOSurfaceRootUserClient * this, uint64_t * structureInput, unsigned int structureInputSize) {
    int64_t v4;            // x21
    OSObject* v5;              // x0
    const OSObject* v6;        // x0
    OSMetaClassBase* oarray;   // x0
    const OSArray* osarray_parsed; // x22
    OSArray* user_oarray;      // x0
    OSArray* arr_from_user_in; // x0

    v4 = 0xE00002C2LL;
    v5 = OSUnserializeXML((const char*)structureInput + 12, structureInputSize - 12LL, 0LL);
    if (v5) {
        v6 = v5;
        oarray = (OSArray*)OSMetaClassBase::safeMetaCast(v5, (const OSMetaClass*)&OSArray::gMetaClass);
        if (oarray) {
            osarray_parsed = (const OSArray*)oarray;
            lck_rw_lock_exclusive(*(IORWLock**)&this->surface_lck_mtx);
            user_oarray = OSArray::withArray(osarray_parsed, 0);
            arr_from_user_in = this->corevideo_bridged_keys;
            if (arr_from_user_in)
                arr_from_user_in->release(arr_from_user_in);

            this->corevideo_bridged_keys = user_oarray;

            lck_rw_done(*(IORWLock**)&this->surface_lck_mtx);
            v4 = 0LL;
        }
        v6->release(v6);
    }
    return v4;
}
```
