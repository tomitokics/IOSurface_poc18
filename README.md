# (CVE-2024-44285) IOSurface Use-After-Free PoC
> Thanks to [slds1](https://github.com/slds1/explt) this Proof of Concept can now be sideloaded onto your device for testing...<br>
> **TODO: Implement Textarea element, to show printf-logs**

## Introduction
This PoC demonstrates a use-after-free (UaF) vulnerability in the `IOSurface` framework, identified as **CVE-2024-44285**. The bug was introduced in **iOS 18** and is relatively shallow, occurring in the second function after the base `s_method` call. The vulnerable function resides in `s_set_corevideo_bridged_keys`, which is method #54 on iOS and #57 on macOS.

The root cause of the issue is a missing lock in the pre-patch version of the function. This allows multiple threads to call the method concurrently, potentially causing an over-release of an `OSArray`. Interestingly, while the kernel extension (KEXT) lacks the locking mechanism, the equivalent userland library correctly applies locks when invoking this method.

---

## Vulnerability Details

The issue arises in the `IOSurfaceRootUserClient::set_corevideo_bridged_keys` function. A pre-patch version of this function is shown below, highlighting the missing locking mechanism.

### Pre-Patch Vulnerable Function
<details>
<summary>See Code Example</summary>

```c
int64_t __fastcall IOSurfaceRootUserClient::set_corevideo_bridged_keys(IOSurfaceRootUserClient* this, uint64_t* structureInput, unsigned int structureInputSize) {
    int64_t v4;                  // x21
    OSObject* v5;               // x0
    const OSObject* v6;         // x0
    OSArray* oarray;            // x0
    OSArray* user_oarray;       // x0
    OSArray* arr_from_user_in;  // x0

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
</details>

---

### Patch Analysis

The patched version of the function introduces proper locking to prevent race conditions. A comparison of the two versions highlights this critical addition.

<details>
<summary>Patched Version</summary>

```c
int64_t __fastcall IOSurfaceRootUserClient::set_corevideo_bridged_keys(IOSurfaceRootUserClient * this, uint64_t * structureInput, unsigned int structureInputSize) {
    int64_t v4;                     // x21
    OSObject* v5;                  // x0
    const OSObject* v6;            // x0
    OSMetaClassBase* oarray;       // x0
    const OSArray* osarray_parsed; // x22
    OSArray* user_oarray;          // x0
    OSArray* arr_from_user_in;     // x0

    v4 = 0xE00002C2LL;
    v5 = OSUnserializeXML((const char*)structureInput + 12, structureInputSize - 12LL, 0LL);
    if (v5) {
        v6 = v5;
        oarray = (OSArray*)OSMetaClassBase::safeMetaCast(v5, (const OSMetaClass*)&OSArray::gMetaClass);
        if (oarray) {
            osarray_parsed = (const OSArray*)oarray;
            lck_rw_lock_exclusive(*(IORWLock**)&this->surface_lck_mtx); // Added Lock
            user_oarray = OSArray::withArray(osarray_parsed, 0);
            arr_from_user_in = this->corevideo_bridged_keys;
            if (arr_from_user_in)
                arr_from_user_in->release(arr_from_user_in);

            this->corevideo_bridged_keys = user_oarray;

            lck_rw_done(*(IORWLock**)&this->surface_lck_mtx); // Unlock
            v4 = 0LL;
        }
        v6->release(v6);
    }
    return v4;
}
```
</details>

The addition of `lck_rw_lock_exclusive` and `lck_rw_done` ensures exclusive access to the shared resource, mitigating the race condition and preventing the use-after-free scenario.

---

## CVE Information

- **CVE Identifier**: [CVE-2024-44285](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2024-44285)
- **Affected Platforms**: iOS 18, macOS (specific versions)
- **Impact**: Exploiting this vulnerability could allow attackers to achieve a kernel panic or potentially execute arbitrary code with kernel privileges.
- **Severity**: High

---

## Credits

- [tomitokics](https://github.com/tomitokics/IOSurface_poc18): For creating the initial PoC for CVE-2024-44285.
- [slds1](https://github.com/slds1/explt): For providing the iPhone app Xcode project used in this repository.
