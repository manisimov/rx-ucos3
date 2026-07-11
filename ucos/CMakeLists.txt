cmake_minimum_required (VERSION 3.16)

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
