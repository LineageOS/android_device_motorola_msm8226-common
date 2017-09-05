/*
 * Copyright (C) 2016 The CyanogenMod Project
 * Copyright (C) 2008 The Android Open Source Project
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

#include <string.h>
#include <cutils/log.h>

#include "AkmSysfs.h"
#include "CompOriSensor.h"

CompOriSensor::CompOriSensor()
    : SensorBase(NULL, "compass"),
      mInputReader(10),
      mPendingEventsMask(0)
{
    memset(&mPendingEvents, 0, sizeof(mPendingEvents));

    writeAkmDelay(ID_M, -1);
    mEnabled[MAG] = false;
    mPendingEvents[MAG].version = sizeof(sensors_event_t);
    mPendingEvents[MAG].sensor = ID_M;
    mPendingEvents[MAG].type = SENSOR_TYPE_MAGNETIC_FIELD;
    mPendingEventsFlushCount[MAG] = 0;
    mDelay[MAG] = 0;

    writeAkmDelay(ID_O, -1);
    mEnabled[ORI] = false;
    mPendingEvents[ORI].version = sizeof(sensors_event_t);
    mPendingEvents[ORI].sensor = ID_O;
    mPendingEvents[ORI].type = SENSOR_TYPE_ORIENTATION;
    mPendingEventsFlushCount[ORI] = 0;
    mDelay[ORI] = 0;

    // We receive events only if the associated value has changed. The
    // accuracy changes rarely, so initialize it with the current value.
    struct input_absinfo absinfo;
    if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_MAGV_STATUS), &absinfo)) {
        // Compass and orientation sensor have the same accuracy
        mPendingEvents[MAG].magnetic.status = absinfo.value;
        mPendingEvents[ORI].orientation.status = absinfo.value;
    } else {
        ALOGE("Could not get initial compass accuracy");
    }
}

CompOriSensor::~CompOriSensor()
{
    if (mEnabled[MAG]) {
        writeAkmDelay(ID_M, -1);
        enable(ID_M, 0);
    }

    if (mEnabled[ORI]) {
        writeAkmDelay(ID_O, -1);
        enable(ID_O, 0);
    }
}

int CompOriSensor::enable(int32_t handle, int en)
{
    bool enable = !!en;
    int sensor;
    int ret;

    switch (handle) {
    case ID_M:
        ALOGV("Compass: enable=%d", en);
        sensor = MAG;
        break;
    case ID_O:
        ALOGV("Orientation: enable=%d", en);
        sensor = ORI;
        break;
    default:
        ALOGE("CompOriSensor: unknown handle %d", handle);
        return -EINVAL;
    }

    if (enable == mEnabled[sensor])
        return 0;

    ret = writeAkmEnable(handle, en);
    if (ret < 0) {
        ALOGE("CompOriSensor: Could not enable handle=%d\n", handle);
        return ret;
    }

    ret = writeAkmDelay(handle, enable ? mDelay[sensor] : -1);
    if (ret < 0) {
        ALOGE("CompOriSensor: Could not set delay while enabling handle=%d\n",
              handle);
        return ret;
    }

    mEnabled[sensor] = enable;

    return 0;
}

int CompOriSensor::setDelay(int32_t handle, int64_t ns)
{
    int sensor;

    switch (handle) {
    case ID_M:
        ALOGV("Compass: delay=%lld ns", ns);
        sensor = MAG;
        break;
    case ID_O:
        ALOGV("Orientation: delay=%lld ns", ns);
        sensor = ORI;
        break;
    default:
        ALOGE("CompOriSensor: unknown handle %d", handle);
        return -EINVAL;
    }

    mDelay[sensor] = ns;

    if (mEnabled[sensor]) {
        return writeAkmDelay(handle, ns);
    }

    return 0;
}

bool CompOriSensor::hasPendingEvents() const
{
    for (int i = 0; i < NUM_SENSORS; i++) {
        if (mPendingEventsFlushCount[i] > 0) {
            return true;
        }
    }
    return false;
}

int CompOriSensor::readEvents(sensors_event_t* data, int count)
{
    if (count < 1)
        return -EINVAL;

    ssize_t n = mInputReader.fill(data_fd);
    if (n < 0)
        return n;

    int numEventReceived = 0;
    input_event const* event;

    for (int i = 0; i < NUM_SENSORS; i++) {
        if (!count)
            break;
        if (!mPendingEventsFlushCount[i])
            continue;
        sensors_meta_data_event_t flushEvent;
        flushEvent.version = META_DATA_VERSION;
        flushEvent.type = SENSOR_TYPE_META_DATA;
        flushEvent.meta_data.what = META_DATA_FLUSH_COMPLETE;
        flushEvent.meta_data.sensor = mPendingEvents[i].sensor;
        flushEvent.reserved0 = 0;
        flushEvent.timestamp = getTimestamp();
        *data++ = flushEvent;
        mPendingEventsFlushCount[i]--;
        count--;
        numEventReceived++;
    }

    while (count && mInputReader.readEvent(&event)) {
        int type = event->type;
        if (type == EV_ABS) {
            float value = event->value;
            switch (event->code) {

            /* Compass */
            case EVENT_TYPE_MAGV_X:
                mPendingEventsMask |= 1 << MAG;
                mPendingEvents[MAG].magnetic.x = value * CONVERT_M_X;
                break;
            case EVENT_TYPE_MAGV_Y:
                mPendingEventsMask |= 1 << MAG;
                mPendingEvents[MAG].magnetic.y = value * CONVERT_M_Y;
                break;
            case EVENT_TYPE_MAGV_Z:
                mPendingEventsMask |= 1 << MAG;
                mPendingEvents[MAG].magnetic.z = value * CONVERT_M_Z;
                break;
            case EVENT_TYPE_MAGV_STATUS:
                mPendingEventsMask |= 1 << MAG;
                mPendingEvents[MAG].magnetic.status = value;
                break;

            /* Orientation */
            case EVENT_TYPE_YAW:
                mPendingEventsMask |= 1 << ORI;
                mPendingEvents[ORI].orientation.azimuth = value * CONVERT_O_A;
                break;
            case EVENT_TYPE_PITCH:
                mPendingEventsMask |= 1 << ORI;
                mPendingEvents[ORI].orientation.pitch = value * CONVERT_O_P;
                break;
            case EVENT_TYPE_ROLL:
                mPendingEventsMask |= 1 << ORI;
                mPendingEvents[ORI].orientation.roll = value * CONVERT_O_R;
                break;
            case EVENT_TYPE_ORIENT_STATUS:
                mPendingEventsMask |= 1 << ORI;
                mPendingEvents[ORI].orientation.status = value;
                break;

            default:
                ALOGE("CompOriSensor: unknown event (type=%d, code=%d)",
                        type, event->code);
            }
        } else if (type == EV_SYN) {
            for (int i = 0; count && mPendingEventsMask && i < NUM_SENSORS; i++) {
                if (mPendingEventsMask & (1 << i)) {
                    mPendingEventsMask &= ~(1 << i);
                    mPendingEvents[i].timestamp = timevalToNano(event->time);
                    if (mEnabled[i]) {
                        *data++ = mPendingEvents[i];
                        count--;
                        numEventReceived++;
                    }
                }
            }
        } else {
            ALOGE("CompOriSensor: unknown event (type=%d, code=%d)",
                    type, event->code);
        }
        mInputReader.next();
    }

    return numEventReceived;
}

int CompOriSensor::flush(int handle)
{
    int id;

    switch (handle) {
    case ID_M:
        id = MAG;
        break;
    case ID_O:
        id = ORI;
        break;
    default:
        ALOGE("CompOriSensor: unknown handle %d", handle);
        return -EINVAL;
    }

    if (!mEnabled[id])
        return -EINVAL;

    mPendingEventsFlushCount[id]++;

    return 0;
}
