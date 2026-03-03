set(PROJECT_NAME common)

################################################################################
# Source groups
################################################################################
set(no_group_source_files
    "../../app/common/vkmath.cpp"
    "../../app/helper/debug.cpp"
    "../../app/helper/resourseload.cpp"
    "../../app/struct/data.cpp"
    "../../app/struct/geometry.cpp"
    "../../app/struct/type.cpp"
    "../../include/common/base_types.h"
    "../../include/common/math_types.h"
    "../../include/common/vkmath.h"
    "../../include/helper/debug.h"
    "../../include/helper/resourseload.h"
    # "../../include/struct/configtype.h"
    "../../include/struct/data.h"
    "../../include/struct/geometry.h"
    "../../include/struct/material.h"
    "../../include/struct/object.h"
    "../../include/struct/resource.h"
    "../../include/struct/type.h"
    "../../include/struct/ubo.h"
    "../../include/struct/vkdevice.h"
)
source_group("" FILES ${no_group_source_files})

set(common
    "../../app/common/log.cpp"
    "../../app/common/macros.cpp"
#    "../../app/common/VectorCMS.cpp"
    "../../include/common/common.h"
    "../../include/common/log.h"
    "../../include/common/macros.h"
#    "../../include/common/VectorCMS.h"
#    "../../include/common/Vkbase.h"
)
source_group("common" FILES ${common})

set(external
    "../../external/include/spirv-reflect/spirv_reflect.c"
    "../../external/include/spirv-reflect/spirv_reflect.h"
)
source_group("external" FILES ${external})

set(helper
    "../../app/helper/helperDescriptor.cpp"
    "../../app/helper/helperDevice.cpp"
    "../../app/helper/helperFile.cpp"
    "../../app/helper/helperPipeline.cpp"
    "../../app/helper/helperResource.cpp"
    "../../app/helper/helperShader.cpp"
    "../../include/helper/helper.h"
    "../../include/helper/helperDescriptor.h"
    "../../include/helper/helperDevice.h"
    "../../include/helper/helperFile.h"
    "../../include/helper/helperPipeline.h"
    "../../include/helper/helperResource.h"
    "../../include/helper/helperShader.h"
)
source_group("helper" FILES ${helper})

set(ALL_FILES
    ${no_group_source_files}
    ${common}
    ${external}
    ${helper}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
################################################################################
# Includes for CMake from *.props
################################################################################
use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "$ENV{VCTargetsPath}/BuildCustomizations/CUDA 12.6.cmake")

set(ROOT_NAMESPACE common)

################################################################################
# Output directory
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/../external/${PROJECT_NAME}/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/"
    OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/../external/${PROJECT_NAME}/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/"
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
)
################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/.;"
    "$ENV{CudaToolkitDir}/include;"
    "$ENV{VULKAN_SDK}/include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../include/struct;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../include/common;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../include/engine2;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../include/helper;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../external/include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../external/ktx/include"
)

################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Debug>:"
        "DEBUG_;"
        "_DEBUG"
    ">"
    "$<$<CONFIG:Release>:"
        "NDEBUG"
    ">"
    "WIN32;"
    "WIN64;"
    "_CONSOLE;"
    "_MBCS"
)

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:
            /Od
        >
        $<$<CONFIG:Release>:
            /O2;
            /Oi;
            /Gy
        >
        /std:c++17;
        /W3;
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        /utf-8;
        ${DEFAULT_CXX_EXCEPTION_HANDLING}
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:
            /OPT:REF;
            /OPT:ICF
        >
        /SUBSYSTEM:CONSOLE
    )
endif()

################################################################################
# Dependencies
################################################################################
set(ADDITIONAL_LIBRARY_DEPENDENCIES
    "cudart_static;"
    "kernel32;"
    "user32;"
    "gdi32;"
    "winspool;"
    "comdlg32;"
    "advapi32;"
    "shell32;"
    "ole32;"
    "oleaut32;"
    "uuid;"
    "odbc32;"
    "odbccp32"
)
target_link_libraries(${PROJECT_NAME} PUBLIC "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

