include_guard(GLOBAL)
message("driver_codec component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/fsl_codec_common.c
)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/.
)


#OR Logic component
if(CONFIG_USE_component_wm8904_adapter_LPC55S16)
     include(component_wm8904_adapter_LPC55S16)
endif()
if(CONFIG_USE_component_wm8960_adapter_LPC55S16)
     include(component_wm8960_adapter_LPC55S16)
endif()
if(CONFIG_USE_component_wm8524_adapter_LPC55S16)
     include(component_wm8524_adapter_LPC55S16)
endif()
if(CONFIG_USE_component_sgtl_adapter_LPC55S16)
     include(component_sgtl_adapter_LPC55S16)
endif()
if(CONFIG_USE_component_da7212_adapter_LPC55S16)
     include(component_da7212_adapter_LPC55S16)
endif()
if(CONFIG_USE_component_ak4497_adapter_LPC55S16)
     include(component_ak4497_adapter_LPC55S16)
endif()
if(CONFIG_USE_component_tfa9xxx_adapter_LPC55S16)
     include(component_tfa9xxx_adapter_LPC55S16)
endif()
if(CONFIG_USE_component_tfa9896_adapter_LPC55S16)
     include(component_tfa9896_adapter_LPC55S16)
endif()
if(CONFIG_USE_component_cs42888_adapter_LPC55S16)
     include(component_cs42888_adapter_LPC55S16)
endif()
if(CONFIG_USE_component_cs42448_adapter_LPC55S16)
     include(component_cs42448_adapter_LPC55S16)
endif()
if(CONFIG_USE_component_codec_adapters_LPC55S16)
     include(component_codec_adapters_LPC55S16)
endif()
if(NOT (CONFIG_USE_component_wm8904_adapter_LPC55S16 OR CONFIG_USE_component_wm8960_adapter_LPC55S16 OR CONFIG_USE_component_wm8524_adapter_LPC55S16 OR CONFIG_USE_component_sgtl_adapter_LPC55S16 OR CONFIG_USE_component_da7212_adapter_LPC55S16 OR CONFIG_USE_component_ak4497_adapter_LPC55S16 OR CONFIG_USE_component_tfa9xxx_adapter_LPC55S16 OR CONFIG_USE_component_tfa9896_adapter_LPC55S16 OR CONFIG_USE_component_cs42888_adapter_LPC55S16 OR CONFIG_USE_component_cs42448_adapter_LPC55S16 OR CONFIG_USE_component_codec_adapters_LPC55S16))
    message(WARNING "Since component_wm8904_adapter_LPC55S16/component_wm8960_adapter_LPC55S16/component_wm8524_adapter_LPC55S16/component_sgtl_adapter_LPC55S16/component_da7212_adapter_LPC55S16/component_ak4497_adapter_LPC55S16/component_tfa9xxx_adapter_LPC55S16/component_tfa9896_adapter_LPC55S16/component_cs42888_adapter_LPC55S16/component_cs42448_adapter_LPC55S16/component_codec_adapters_LPC55S16 is not included at first or config in config.cmake file, use component_wm8904_adapter_LPC55S16 by default.")
    include(component_wm8904_adapter_LPC55S16)
endif()

include(driver_common_LPC55S16)

