cmake_minimum_required(VERSION 3.20)
project(tetros LANGUAGES C CXX ASM_NASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_ASM_NASM_COMPILER nasm)
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
set(CMAKE_ASM_NASM_COMPILE_OBJECT
        "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} -o <OBJECT> <SOURCE>")

file(GLOB_RECURSE SOURCES_ASM src/*.asm)
file(GLOB_RECURSE SOURCES_CXX src/*.cpp src/*.c)
set_source_files_properties(${SOURCES_ASM} PROPERTIES LANGUAGE ASM_NASM)

set(KERNEL_TARGET tetros)
add_executable(${KERNEL_TARGET} ${SOURCES_ASM} ${SOURCES_CXX})

target_compile_options(${KERNEL_TARGET} PRIVATE
        -ffreestanding -fno-exceptions -fno-rtti
        -Wall -Wextra -mno-mmx -mno-avx -mno-avx512f -mno-sse2
)

target_include_directories(${KERNEL_TARGET} PRIVATE include)
target_link_libraries(${KERNEL_TARGET} PRIVATE gcc)
target_link_options(${KERNEL_TARGET} PRIVATE
        -T ${CMAKE_SOURCE_DIR}/linker.ld -nostdlib -static
)

find_program(GRUB_MKRESCUE NAMES grub-mkrescue grub2-mkrescue)
if (NOT GRUB_MKRESCUE)
    message(WARNING "grub-mkrescue not found")
    return()
endif ()

set(ISO_DIR ${CMAKE_BINARY_DIR}/iso)
set(ISO_OUTPUT ${CMAKE_BINARY_DIR}/tetros.iso)
set(GRUB_CFG ${ISO_DIR}/boot/grub/grub.cfg)

file(WRITE ${CMAKE_BINARY_DIR}/grub.cfg.in
        "insmod all_video
set timeout=1
set default=0

menuentry \"kernel\" {
  multiboot2 /boot/tetros.bin
  boot
}
")

add_custom_command(TARGET ${KERNEL_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${ISO_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${ISO_DIR}/boot/grub
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${KERNEL_TARGET}> ${ISO_DIR}/boot/tetros.bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/grub.cfg.in ${GRUB_CFG}
        COMMAND ${GRUB_MKRESCUE} -o ${ISO_OUTPUT} ${ISO_DIR}
        COMMENT "Creating bootable ISO"
        VERBATIM
)