# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/Oscar/esp/v5.3.1/esp-idf/components/bootloader/subproject"
  "C:/Users/Oscar/OneDrive - Universidade de Coimbra/Portugal/J1939 git hub/FedLearningNodes/FedLearning-Anomaly/FedLearning-Anomaly/node1/build/bootloader"
  "C:/Users/Oscar/OneDrive - Universidade de Coimbra/Portugal/J1939 git hub/FedLearningNodes/FedLearning-Anomaly/FedLearning-Anomaly/node1/build/bootloader-prefix"
  "C:/Users/Oscar/OneDrive - Universidade de Coimbra/Portugal/J1939 git hub/FedLearningNodes/FedLearning-Anomaly/FedLearning-Anomaly/node1/build/bootloader-prefix/tmp"
  "C:/Users/Oscar/OneDrive - Universidade de Coimbra/Portugal/J1939 git hub/FedLearningNodes/FedLearning-Anomaly/FedLearning-Anomaly/node1/build/bootloader-prefix/src/bootloader-stamp"
  "C:/Users/Oscar/OneDrive - Universidade de Coimbra/Portugal/J1939 git hub/FedLearningNodes/FedLearning-Anomaly/FedLearning-Anomaly/node1/build/bootloader-prefix/src"
  "C:/Users/Oscar/OneDrive - Universidade de Coimbra/Portugal/J1939 git hub/FedLearningNodes/FedLearning-Anomaly/FedLearning-Anomaly/node1/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/Oscar/OneDrive - Universidade de Coimbra/Portugal/J1939 git hub/FedLearningNodes/FedLearning-Anomaly/FedLearning-Anomaly/node1/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/Oscar/OneDrive - Universidade de Coimbra/Portugal/J1939 git hub/FedLearningNodes/FedLearning-Anomaly/FedLearning-Anomaly/node1/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
