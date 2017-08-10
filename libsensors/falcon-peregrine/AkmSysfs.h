#ifndef _AKM_SYSFS_H
#define _AKM_SYSFS_H

int writeAkmDelay(int32_t handle, int64_t ns);
void writeAkmAccel(float x, float y, float z);

#endif // _AKM_SYSFS_H
