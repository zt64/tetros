#include "driver/keyboard.hpp"
#include "driver/screen.hpp"
#include "kernel/irq.hpp"
#include "kernel/system.hpp"
#include <cstddef>

static key_listener_t g_listener = nullptr;

static volatile uint8_t kb_buf[64];
static volatile uint8_t kb_buf_head = 0;
static volatile uint8_t kb_buf_tail = 0;

static void kb_enqueue(const uint8_t scancode) {
    if (const size_t next = (kb_buf_head + 1) % sizeof(kb_buf); next != kb_buf_tail) {
        kb_buf[next] = scancode;
        kb_buf_head = next;
    }
}

static bool kb_dequeue(uint8_t* out) {
    if (kb_buf_tail == kb_buf_head) return false;
    *out = kb_buf[kb_buf_tail];
    kb_buf_tail = (kb_buf_tail + 1) % sizeof(kb_buf);
    return true;
}

void kb_register_listener(const key_listener_t listener) { g_listener = listener; }
void kb_unregister_listener() { g_listener = nullptr; }

void kb_process_queue() {
    uint8_t sc;
    while (kb_dequeue(&sc)) {
        if (g_listener) g_listener(sc);
    }
}

void kb_handler([[maybe_unused]] regs* r) {
    const unsigned char scancode = inb(0x60);

    kb_enqueue(scancode);
}

void kb_init() {
    irq_install_handler(1, kb_handler);
}
