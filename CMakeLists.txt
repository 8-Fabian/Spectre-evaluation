
#----------------------------------------------#
#-----Konfiguration des Buildsystems CMake-----#
#----------------------------------------------#

# Name des Projekts
cmake_minimum_required(VERSION 3.14)
project(Spectre_Evaluation C)

#----------------------------------------------#
#-------------------Optionen-------------------#
#----------------------------------------------#

add_compile_options(-O0)
add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

#----------------------------------------------#
#---------------Erzeuge Projekt----------------#
#----------------------------------------------#

add_executable(spec_exec_leak
  speculative_exec_leak.c
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

add_executable(spectre_attacker
  spectre_attacker.c
  ${SRC_FILES}
)