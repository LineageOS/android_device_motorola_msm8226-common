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

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <cutils/log.h>

#include "sensors.h"

int writeAkmDelay(int32_t handle, int64_t ns)
{
    char delay_path[PATH_MAX];
    int fd;

    switch (handle) {
    case ID_A:
        ALOGV("AkmSysfs: Accelerometer: delay=%lld ns", ns);
        strcpy(delay_path, "/sys/class/compass/akm8963/delay_acc");
        break;
    case ID_M:
        ALOGV("AkmSysfs: Compass: delay=%lld ns", ns);
        strcpy(delay_path, "/sys/class/compass/akm8963/delay_mag");
        break;
    case ID_O:
        ALOGV("AkmSysfs: Orientation: delay=%lld ns", ns);
        strcpy(delay_path, "/sys/class/compass/akm8963/delay_ori");
        break;
    default:
        ALOGE("AkmSysfs: unknown handle %d", handle);
        return -EINVAL;
    }

    if (ns < MAG_MIN_DELAY_NS) {
        ns = MAG_MIN_DELAY_NS;
    }

    fd = open(delay_path, O_WRONLY);
    if (fd >= 0) {
        char buf[80];
        int ret;
        sprintf(buf, "%lld", ns);
        ret = write(fd, buf, strlen(buf)+1);
        if (ret < 0) {
            ALOGE("AkmSysfs: could not set delay of handle %d to %lld",
                  handle, ns);
            return ret;
        }
        close(fd);
        return 0;
    }
    return fd;
}

void writeAkmAccel(float x, float y, float z)
{
    int16_t buf[3];
    int ret;
    int fd;

    buf[0] = (int16_t) (x * CONVERT_ACC);
    buf[1] = (int16_t) (y * CONVERT_ACC);
    buf[2] = (int16_t) (z * CONVERT_ACC);

    fd = open("/sys/devices/virtual/compass/akm8963/accel", O_WRONLY);
    if (fd < 0) {
        ALOGE("AkmSysfs: could not open accel file");
        return;
    }

    ret = write(fd, buf, sizeof(buf));
    if (ret < 0)
        ALOGE("AkmSysfs: could not write accel data");

    close(fd);
}
