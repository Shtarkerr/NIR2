cmake_minimum_required(VERSION 3.16)

project(ImageAnnotator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

find_package(Qt5 5.12 REQUIRED COMPONENTS Core Gui Widgets)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
elseif(MSVC)
    # Исходники в UTF-8, иначе MSVC испортит кириллицу в строках.
    add_compile_options(/utf-8 /W4)
endif()

# Модель и работа с файлами отдельно от виджетов, чтобы тесты шли без GUI.
add_library(annotator_core STATIC
    src/model/Annotation.h            src/model/Annotation.cpp
    src/model/FillableAnnotation.h
    src/model/PointAnnotation.h       src/model/PointAnnotation.cpp
    src/model/RectAnnotation.h        src/model/RectAnnotation.cpp
    src/model/EllipseAnnotation.h     src/model/EllipseAnnotation.cpp
    src/model/ContourAnnotation.h     src/model/ContourAnnotation.cpp
    src/model/Raster.h                src/model/Raster.cpp
    src/model/AnnotationFactory.h     src/model/AnnotationFactory.cpp
    src/model/AnnotationDocument.h    src/model/AnnotationDocument.cpp
    src/io/JsonSerializer.h           src/io/JsonSerializer.cpp
    src/io/TxtExporter.h              src/io/TxtExporter.cpp
)
target_include_directories(annotator_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(annotator_core PUBLIC Qt5::Core Qt5::Gui)

add_executable(image_annotator
    src/main.cpp
    src/commands/AnnotationCommands.h src/commands/AnnotationCommands.cpp
    src/tools/Tool.h                  src/tools/Tool.cpp
    src/tools/SelectTool.h            src/tools/SelectTool.cpp
    src/tools/PointTool.h             src/tools/PointTool.cpp
    src/tools/DragShapeTool.h         src/tools/DragShapeTool.cpp
    src/tools/ContourTool.h           src/tools/ContourTool.cpp
    src/ui/Icons.h                    src/ui/Icons.cpp
    src/ui/AnnotationGraphicsItem.h   src/ui/AnnotationGraphicsItem.cpp
    src/ui/ImageCanvas.h              src/ui/ImageCanvas.cpp
    src/ui/AnnotationListDock.h       src/ui/AnnotationListDock.cpp
    src/ui/MainWindow.h               src/ui/MainWindow.cpp
)
target_link_libraries(image_annotator PRIVATE annotator_core Qt5::Widgets)

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
