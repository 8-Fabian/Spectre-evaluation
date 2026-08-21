
#----------------------------------------------#
#-----Konfiguration des Buildsystems CMake-----#
#----------------------------------------------#

# Name des Projekts
cmake_minimum_required(VERSION 3.14)
project(Spectre_Evaluation C ASM_NASM)

#----------------------------------------------#
#-------------------Optionen-------------------#
#----------------------------------------------#

add_compile_options(-O0)
add_compile_options($<$<COMPILE_LANGUAGE:C>:-fsanitize=address>)
add_link_options(-fsanitize=address)

set(BYTES_TO_BE_READ 10000 CACHE STRING "Number of read iterations in Attacker")

#----------------------------------------------#
#---------------Erzeuge Projekt----------------#
#----------------------------------------------#

add_executable(spec_exec_leak
  speculative_exec_leak.c
  ${SRC_FILES}
)

add_executable(spectre_attacker
  spectre_attacker.c
  ${SRC_FILES}
)

add_executable(spectre_victim
  spectre_victim.c
  ${SRC_FILES}
)

add_executable(spectre_mitigated
  spectre_mitigated.c
  ${SRC_FILES}
)

add_executable(spectre_v2_poc 
  spectrev2_poc.c spectrev2_poc.asm
  ${SRC_FILES}
)

add_executable(spectre_v2_mitigated 
  spectrev2_poc.c spectrev2_mitigated.asm
  ${SRC_FILES}
)

target_compile_definitions(spectre_attacker PRIVATE BYTES_TO_BE_READ=${BYTES_TO_BE_READ})