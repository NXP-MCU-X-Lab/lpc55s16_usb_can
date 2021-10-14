include_guard(GLOBAL)
message("middleware_usb_host_common_header component is included.")


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/host
)

include(component_osa_LPC55S16)

include(middleware_usb_common_header_LPC55S16)

