/*
 * Copyright (C) 2017 The LineageOS Project
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
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "libshims_camera"
#include <cutils/log.h>
#include <cutils/properties.h>

static typeof(property_get) *property_get_real = NULL;

int property_get(const char *key, char *value, const char *default_value)
{
    if (!strcmp("ro.build.tags", key)) {
        ALOGV("Returning release-keys for ro.build.tags");
        return strlen(strcpy(value, "release-keys"));
    }

    if (!property_get_real) {
        property_get_real = dlsym(RTLD_NEXT, "property_get");
    }

    if (property_get_real) {
        return property_get_real(key, value, default_value);
    }

    /*
     * If we could not find property_get, print an error and bail out,
     * we should not pretend everything's fine and return default_value.
     */
    ALOGE("Could not find property_get, aborting");
    abort();

    return 0;
}
