/*
 * Copyright (C) 2016, The CyanogenMod Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
* @file ril-wrapper.c
*
* This file wraps a vendor RIL module.
*
*/

#define LOG_TAG "RilWrapper"

#include <telephony/ril.h>
#include <utils/Log.h>
#include <dlfcn.h>

#define REAL_RIL_NAME "libril-qc-qmi-1.so"

static RIL_RadioFunctions const *mRealRadioFuncs;
static const struct RIL_Env *mEnv;

const RIL_RadioFunctions* RIL_Init(const struct RIL_Env *env, int argc, char **argv)
{
    RIL_RadioFunctions const* (*fRealRilInit)(const struct RIL_Env *env, int argc, char **argv);
    static RIL_RadioFunctions rilInfo;
    void *realRilLibHandle;
    int i;

    // save the env;
    mEnv = env;

    // get the real RIL
    realRilLibHandle = dlopen(REAL_RIL_NAME, RTLD_LOCAL);
    if (!realRilLibHandle) {
        RLOGE("Failed to load the real RIL '" REAL_RIL_NAME  "': %s\n", dlerror());
        return NULL;
    }

    // load the real RIL
    fRealRilInit = dlsym(realRilLibHandle, "RIL_Init");
    if (!fRealRilInit) {
        RLOGE("Failed to find the real RIL's entry point\n");
        goto out_fail;
    }

    RLOGD("Calling the real RIL's entry point with %u args\n", argc);
    for (i = 0; i < argc; i++)
        RLOGD("  argv[%2d] = '%s'\n", i, argv[i]);

    // try to init the real ril
    mRealRadioFuncs = fRealRilInit(env, argc, argv);
    if (!mRealRadioFuncs) {
        RLOGE("The real RIL's entry point failed\n");
        goto out_fail;
    }

    // copy the real RIL's info struct, then replace the onRequest pointer with our own
    rilInfo = *mRealRadioFuncs;
    rilInfo.version = 12;

    RLOGD("Wrapped RIL version is '%s'\n", mRealRadioFuncs->getVersion());

    // we're all good - return to caller
    return &rilInfo;

out_fail:
    dlclose(realRilLibHandle);
    return NULL;
}
