# Copyright (C) 2016 The CyanogenMod Project
# Copyright (C) 2017 The LineageOS Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import re

def FullOTA_Assertions(info):
  AddBootloaderAssertion(info, info.input_zip)


def IncrementalOTA_Assertions(info):
  AddBootloaderAssertion(info, info.target_zip)


def FullOTA_PostValidate(info):
  ResizeSystem(info)


def AddBootloaderAssertion(info, input_zip):
  android_info = input_zip.read("OTA/android-info.txt")
  m = re.search(r"require\s+version-bootloader\s*=\s*(\S+)", android_info)
  if m:
    bootloaders = m.group(1).split("|")
    if "*" not in bootloaders:
      info.script.AssertSomeBootloader(*bootloaders)
    info.metadata["pre-bootloader"] = m.group(1)


def ResizeSystem(info):
  if info.metadata["ota-type"] == "BLOCK":
    fstab = info.info_dict.get("fstab", None)
    system_block = fstab["/system"].device
    e2fsck = "/tmp/install/bin/e2fsck_static"
    resize2fs = "/tmp/install/bin/resize2fs_static"
    resize_error = "Error: could not resize /system"

    # We copied verbatim an image that we already checked with e2fsck,
    # so this shouldn't be required.
    info.script.AppendExtra('run_program("%s", "-fy", "%s");'
                            % (e2fsck, system_block))
    info.script.AppendExtra('run_program("%s", "%s") == 0 || ui_print("%s");'
                            % (resize2fs, system_block, resize_error))
    # However, there might be errors caused by resize2fs
    info.script.AppendExtra('run_program("%s", "-fy", "%s");'
                            % (e2fsck, system_block))
