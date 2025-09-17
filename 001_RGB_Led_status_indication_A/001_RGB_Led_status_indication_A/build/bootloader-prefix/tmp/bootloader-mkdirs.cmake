# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Espressif/frameworks/esp-idf-v5.3.1/components/bootloader/subproject"
  "D:/Embedded System Volume/Esp_IDF_workspace/001_RGB_Led_status_indication_A/build/bootloader"
  "D:/Embedded System Volume/Esp_IDF_workspace/001_RGB_Led_status_indication_A/build/bootloader-prefix"
  "D:/Embedded System Volume/Esp_IDF_workspace/001_RGB_Led_status_indication_A/build/bootloader-prefix/tmp"
  "D:/Embedded System Volume/Esp_IDF_workspace/001_RGB_Led_status_indication_A/build/bootloader-prefix/src/bootloader-stamp"
  "D:/Embedded System Volume/Esp_IDF_workspace/001_RGB_Led_status_indication_A/build/bootloader-prefix/src"
  "D:/Embedded System Volume/Esp_IDF_workspace/001_RGB_Led_status_indication_A/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/Embedded System Volume/Esp_IDF_workspace/001_RGB_Led_status_indication_A/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/Embedded System Volume/Esp_IDF_workspace/001_RGB_Led_status_indication_A/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
