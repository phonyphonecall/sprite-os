#include "sos.h"
#include "interrupt.h"

// HW specific registers and masks
#define PIC_BASE_ADDR (0x81800000)
#define PIC_MASTER_ENABLE_ADDR (PIC_BASE_ADDR + 0x1C)
#define PIC_INTERRUPT_ENABLE_ADDR (PIC_BASE_ADDR + 0x08)
#define PIC_INTERRUPT_ACK_ADDR (PIC_BASE_ADDR + 0x0C)
#define PIC_MASTER_ENABLE_MASK (0x03)
#define PIC_VSYNC_MASK (0x04)

#define NUM_CALLBACKS 256

typedef struct cb_node {
    bool enabled;
    sos_cb_t cb;
    void* data;
} cb_node;

int cb_count = 0;
cb_node cb_array[NUM_CALLBACKS];

extern void microblaze_enable_interrupts(void);

inline void sos_enable_mb_interrupts(void) {
    microblaze_enable_interrupts();
}

void sos_enable_vsync_interrupt(char* str) {
    sos_enable_mb_interrupts();

    SET_ADDR(PIC_MASTER_ENABLE_ADDR, PIC_MASTER_ENABLE_MASK);
    SET_ADDR(PIC_INTERRUPT_ENABLE_ADDR, PIC_VSYNC_MASK);
}

sos_cb_id_t sos_register_vsync_cb(sos_cb_t cb, void* data, bool enable) {
    if (cb_count > NUM_CALLBACKS) {
        return SOS_ERR;
    }
    sos_cb_id_t id = (sos_cb_id_t) cb_count;
    cb_node *node = &cb_array[id];
    node->cb = cb;
    node->data = data;
    node->enabled = enable;

    cb_count++;

    return id;
}

void sos_enable_vsync_cb(sos_cb_id_t id) {
    if (id < NUM_CALLBACKS && id >= 0)
        cb_array[id].enabled = true;
}

void sos_disable_vsync_cb(sos_cb_id_t id) {
    if (id < NUM_CALLBACKS && id >= 0)
        cb_array[id].enabled = false;
}

inline void clear_pic_interrupt(void) {
    SET_ADDR(PIC_INTERRUPT_ACK_ADDR, PIC_VSYNC_MASK);
}

void vsync_isr(void) {
    clear_pic_interrupt();

    cb_node *node;
    for (node = &cb_array[0]; node <= &cb_array[NUM_CALLBACKS-1]; node++) {
        if (node->enabled) {
            node->cb(node->data);
        }
    }
}
