set(requires spi_flash)

if("${IDF_VERSION_MAJOR}.${IDF_VERSION_MINOR}" VERSION_GREATER_EQUAL "5.0")
    list(APPEND requires esp_partition)
endif()

idf_component_register(
    SRC_DIRS .
    INCLUDE_DIRS .
    REQUIRES ${requires}
)

partition_table_get_partition_info(size "--partition-name ezfiles" "size")
partition_table_get_partition_info(offset "--partition-name ezfiles" "offset")

if("${size}" AND "${offset}")
    set(image_file ${CMAKE_BINARY_DIR}/qmsd_ezfiles.bin)

    add_custom_target(generate-ezfiles
        COMMAND ${PYTHON} ${COMPONENT_PATH}/tools/generate_ezfiles.py ${PROJECT_DIR}/main/ezfiles ${size} ${image_file}
        DEPENDS ${PROJECT_DIR}/main/ezfiles
        VERBATIM
    )
    add_dependencies(${COMPONENT_LIB} generate-ezfiles)
    
    set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" APPEND PROPERTY
        ADDITIONAL_MAKE_CLEAN_FILES
        ${image_file})

    idf_component_get_property(main_args esptool_py FLASH_ARGS)
    idf_component_get_property(sub_args esptool_py FLASH_SUB_ARGS)
    esptool_py_flash_target(qmsd-files-flash "${main_args}" "${sub_args}" ALWAYS_PLAINTEXT)
    esptool_py_flash_to_partition(qmsd-files-flash "ezfiles" "${image_file}")
    add_dependencies(qmsd-files-flash generate-ezfiles)

    esptool_py_flash_to_partition(flash "ezfiles" "${image_file}")
endif()
