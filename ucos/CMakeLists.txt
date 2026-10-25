cmake_minimum_required (VERSION 3.16)

######### OS ###########

add_library (ucos3
  #### uCOS-III CPU specific ####
  uC-OS3/Ports/Renesas/RX/GNURX/os_cpu_c.c
  uC-OS3/Ports/Renesas/RX/GNURX/os_cpu_a.S
  #### CPU independent ####
  uC-OS3/Source/__dbg_uCOS-III.c
  uC-OS3/Source/os_cfg_app.c
  uC-OS3/Source/os_core.c
  uC-OS3/Source/os_dbg.c
  uC-OS3/Source/os_flag.c
  uC-OS3/Source/os_mem.c
  uC-OS3/Source/os_msg.c
  uC-OS3/Source/os_mutex.c
  uC-OS3/Source/os_prio.c
  uC-OS3/Source/os_q.c
  uC-OS3/Source/os_sem.c
  uC-OS3/Source/os_stat.c
  uC-OS3/Source/os_task.c
  uC-OS3/Source/os_tick.c
  uC-OS3/Source/os_time.c
  uC-OS3/Source/os_tmr.c
  uC-OS3/Source/os_var.c
  #### uC-CPU ####
  uC-CPU/RX/GNURX/cpu_a.s
  uC-CPU/cpu_core.c
)

set_target_properties (ucos3 PROPERTIES EXCLUDE_FROM_ALL TRUE)
target_compile_options (ucos3 PRIVATE ${UCOS_COMPILE_FLAGS})

target_include_directories (ucos3 PUBLIC
  #### CPU independent ####
  uC-OS3/Source
  #### uC-CPU ####
  uC-CPU
  uC-CPU/RX/GNURX
  #### uC-LIB ####
  uC-LIB
  #### uCOS-III CPU specific ####
  uC-OS3/Ports/Renesas/RX/GNURX
  #### configuration ####
  ${UCOS_CONFIG_DIR}
)

######## uC-LIB ###############################

add_library (uclib
  uC-LIB/lib_ascii.c
  uC-LIB/lib_math.c
  uC-LIB/lib_mem.c
  uC-LIB/lib_str.c
)

set_target_properties (uclib PROPERTIES EXCLUDE_FROM_ALL TRUE)
target_compile_options (uclib PRIVATE ${UCOS_COMPILE_FLAGS})

target_include_directories (uclib PUBLIC
  uC-LIB
  uC-OS3/Source
  uC-CPU
  uC-CPU/RX/GNURX
  ${UCOS_CONFIG_DIR}
)

######### USB Device stack (uC-USBD) ###########

add_library (ucusbd
  uC-USBD/Source/usbd_core.c
  uC-USBD/Source/usbd_ep.c
  uC-USBD/OS/uCOS-III/usbd_os.c
  uC-USBD/Drivers/RX600/usbd_drv_rx600.c
  uC-USBD/Class/HID/usbd_hid.c
  uC-USBD/Class/HID/usbd_hid_report.c
  uC-USBD/Class/HID/OS/uCOS-III/usbd_hid_os.c
  uC-USBD/Class/Video/usbd_uvc.c
### usb app example ###
  #uC-USBD/App/Device/app_usbd.c
  #uC-USBD/App/Device/app_usbd_hid.c
)

set_target_properties (ucusbd PROPERTIES EXCLUDE_FROM_ALL TRUE)
target_compile_options (ucusbd PRIVATE ${UCOS_COMPILE_FLAGS})

target_include_directories (ucusbd PUBLIC
  #### CPU independent ####
  uC-OS3/Source
  uC-OS3
  #### uC-CPU ####
  uC-CPU
  uC-CPU/RX/GNURX
  #### uC-LIB ####
  uC-LIB
  #### uCOS-III CPU specific ####
  uC-OS3/Ports/Renesas/RX/GNURX
  #### configuration ####
  ${UCOS_CONFIG_DIR}
  ######
  uC-USBD/Drivers/RX600
  ######
  uC-USBD/Class/HID
  uC-USBD/Class/Video
  ######
  uC-USBD/App/Device
  uC-USBD
  uC-USBD/Source
)

#add_library (ucusbd
