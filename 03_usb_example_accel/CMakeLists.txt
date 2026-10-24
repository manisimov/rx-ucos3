cmake_minimum_required (VERSION 3.16)

set (PROJECT_NAME rx_usb_example_accel)
project (${PROJECT_NAME} C ASM)

set (CMAKE_C_COMPILER rx-elf-gcc)
set (CMAKE_ASM_COMPILER rx-elf-gcc)
set (COMPILE_FLAGS -mcpu=rx600 -mlittle-endian-data -g -O0)
#set (COMPILE_FLAGS -mcpu=rx600 -mlittle-endian-data)

set (UCOS_DIR "${CMAKE_SOURCE_DIR}/../ucos")
set (UCOS_CONFIG_DIR "${CMAKE_SOURCE_DIR}/ucos_configs")
set (UCOS_COMPILE_FLAGS ${COMPILE_FLAGS})
add_subdirectory (${UCOS_DIR} ucos)

add_executable (${PROJECT_NAME} src/main.c
                                src/device/startup.S
                                src/device/vector_table.c
                                src/device/interrupt_handlers.c
                                src/hw/hw.c
                                src/hw/hw_os.c
                                src/leds.c
                                src/usb_device.c
                                src/buttons.c
                                src/riic.c
                                src/accel.c
                                src/uart.c)

target_include_directories (${PROJECT_NAME} PRIVATE src
                                                    src/device
                                                    src/hw
                                                    ucos_configs
                                                    )

target_compile_options (${PROJECT_NAME} PRIVATE ${COMPILE_FLAGS})
target_link_options (${PROJECT_NAME} PRIVATE "-T${CMAKE_SOURCE_DIR}/src/device/RX62N.ld"
                                             -nostartfiles
                                             -mcpu=rx600
                                             -mlittle-endian-data
                                             "-e_PowerON_Reset"
                                             -specs=nosys.specs
                                             -specs=nano.specs)

target_link_libraries(${PROJECT_NAME} "$<LINK_GROUP:RESCAN,ucos3,ucusbd,uclib>")

# Debug on HW

add_custom_target (gdbser
  COMMAND JLinkGDBServerCLExe -select USB -device RX62N -endian little -if JTAG -speed 4000 -ir -noLocalhostOnly
  DEPENDS ${PROJECT_NAME})

add_custom_target (cgdbl
                   COMMAND cgdb -d rx-elf-gdb $<TARGET_FILE:${PROJECT_NAME}> -ex 'b main'
                   -ex 'target remote 127.0.0.1:2331'
                   -ex 'monitor halt'
                   -ex 'monitor reset 2'
                   -ex 'load'
                   -x ${CMAKE_SOURCE_DIR}/.gdbinit
                   DEPENDS ${PROJECT_NAME})

add_custom_target (gdbl
                   COMMAND rx-elf-gdb $<TARGET_FILE:${PROJECT_NAME}> -ex 'b main'
                   -ex 'target remote 127.0.0.1:2331'
                   -ex 'monitor halt'
                   -ex 'monitor reset 2'
                   -ex 'load'
                   -x ${CMAKE_SOURCE_DIR}/.gdbinit
                   DEPENDS ${PROJECT_NAME})

add_custom_target (cgdbr
                   COMMAND cgdb -d rx-elf-gdb $<TARGET_FILE:${PROJECT_NAME}> -ex 'b main'
                   -ex 'target remote 127.0.0.1:2331'
                   -ex 'monitor halt'
                   -ex 'monitor reset 2'
                   -x ${CMAKE_SOURCE_DIR}/.gdbinit
                   DEPENDS ${PROJECT_NAME})

add_custom_target (gdbr
                   COMMAND rx-elf-gdb $<TARGET_FILE:${PROJECT_NAME}> -ex 'b main'
                   -ex 'target remote 127.0.0.1:2331'
                   -ex 'monitor halt'
                   -ex 'monitor reset 2'
                   -x ${CMAKE_SOURCE_DIR}/.gdbinit
                   DEPENDS ${PROJECT_NAME})
