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
#include <string.h>

#include <cutils/properties.h>

static typeof(property_get) *property_get_real = NULL;

int property_get(const char *key, char *value, const char *default_value)
{
    if (!strcmp("ro.build.tags", key)) {
        strcpy(value, "release-keys");
        return 12; // string length, must be smaller than PROPERTY_VALUE_MAX-1
    }

    if (property_get_real == NULL) {
        property_get_real = dlsym(RTLD_NEXT, "property_get");
    }

    if (property_get_real != NULL) {
        return property_get_real(key, value, default_value);
    } else {
        value[0] = 0;
        return 0;
    }
}
