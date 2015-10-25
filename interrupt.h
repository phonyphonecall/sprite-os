#ifndef INTERRUPT_H
#define INTERRUPT_H

typedef int sos_cb_id_t;
typedef void (*sos_cb_t)(void*); 

void sos_enable_vsync_interrupt();

sos_cb_id_t sos_register_vsync_cb(sos_cb_t cb, void* data, bool enable);
void sos_enable_vsync_cb(sos_cb_id_t id);
void sos_disable_vsync_cb(sos_cb_id_t id);

// PRIVATE
void vsync_isr(void) __attribute__ ((interrupt_handler));
#endif // INTERRUPT_H
