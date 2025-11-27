cmake_minimum_required(VERSION 3.20)
project(tetros LANGUAGES C CXX ASM_NASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_ASM_NASM_COMPILER nasm)
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} -o <OBJECT> <SOURCE>")

file(GLOB_RECURSE SOURCES_ASM kernel/src/*.asm)
file(GLOB_RECURSE SOURCES_CXX kernel/src/*.cpp kernel/src/*.c)
set_source_files_properties(${SOURCES_ASM} PROPERTIES LANGUAGE ASM_NASM)

set(KERNEL_TARGET tetros)
add_executable(${KERNEL_TARGET} ${SOURCES_ASM} ${SOURCES_CXX})

# Use kernel code model and disable PIE/PIC so the compiler/linker generate
# full 64-bit relocations for kernel symbols
target_compile_options(${KERNEL_TARGET} PRIVATE
        -ffreestanding -fno-stack-protector -mno-red-zone
        -mno-mmx -mno-avx -mno-avx512f -mno-sse -mno-sse2
        -g -Wall -Wextra -mcmodel=kernel -fno-pic -fno-pie
)

# C++-only flags
target_compile_options(${KERNEL_TARGET} PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
)

target_include_directories(${KERNEL_TARGET} PRIVATE kernel/include)
target_link_libraries(${KERNEL_TARGET} PRIVATE gcc)
target_link_options(${KERNEL_TARGET} PRIVATE -T ${CMAKE_SOURCE_DIR}/linker.ld -nostdlib -static -no-pie -mcmodel=kernel)

find_program(XORRISO NAMES xorriso)
if (NOT XORRISO)
    message(FATAL_ERROR "xorriso not found")
endif ()

set(ISO_DIR ${CMAKE_BINARY_DIR}/iso)
set(ISO_OUTPUT ${CMAKE_BINARY_DIR}/tetros.iso)
set(LIMINE_SRC_DIR ${CMAKE_SOURCE_DIR}/limine)
set(LIMINE_DIR ${ISO_DIR}/boot/limine)

add_custom_command(TARGET ${KERNEL_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${ISO_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${LIMINE_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${KERNEL_TARGET}> ${ISO_DIR}/boot/tetros
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/limine.conf ${LIMINE_SRC_DIR}/limine-bios.sys
        ${LIMINE_SRC_DIR}/limine-bios-cd.bin ${LIMINE_SRC_DIR}/limine-uefi-cd.bin ${LIMINE_DIR}

        COMMAND ${CMAKE_COMMAND} -E make_directory ${ISO_DIR}/EFI/BOOT
        COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_SRC_DIR}/BOOTX64.EFI ${ISO_DIR}/EFI/BOOT
        COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_SRC_DIR}/BOOTIA32.EFI ${ISO_DIR}/EFI/BOOT

        COMMAND ${XORRISO}
        ARGS -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin
        -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus
        -apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin
        -efi-boot-part --efi-boot-image --protective-msdos-label
        ${ISO_DIR} -o ${ISO_OUTPUT}

        COMMAND ${LIMINE_SRC_DIR}/limine
        ARGS bios-install ${ISO_OUTPUT}
        COMMENT "Creating bootable ISO"
        VERBATIM
)