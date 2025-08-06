#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <storage/storage.h>
#include <notification/notification_messages.h>
#include <furi_hal.h>

#define TAG "NRF24Scanner"  // For debugging
#define MAX_CHANNELS 126
#define LOG_PATH "/ext/nrfscan/session1.nrf"

typedef struct {
    uint8_t channel;
    bool detected;
} ChannelData;

typedef struct {
    ChannelData data[MAX_CHANNELS];
    bool running;
} AppState;

static bool nrf24_detect_signal(uint8_t ch) {
    FURI_LOG_D(TAG, "Checking channel %d", ch);
    return (ch % 8 == 0); // Simulated signal detection
}

static void render(Canvas* canvas, void* ctx) {
    AppState* state = (AppState*)ctx;
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 10, 10, "NRF24 Scanner");

    for(size_t i = 0; i < MAX_CHANNELS && i < 100; i++) {
        if(state->data[i].detected) {
            canvas_draw_box(canvas, 10 + (i % 20) * 6, 20 + (i / 20) * 10, 5, 5);
        }
    }
}

static void input_callback(InputEvent* event, void* ctx) {
    AppState* state = (AppState*)ctx;
    if(event->key == InputKeyBack) {
        state->running = false;
    }
}

int32_t nrf24_scanner_app(void* p) {
    FURI_LOG_I(TAG, "Application started");  // Important debug log
    
    // Initialize state
    AppState* state = malloc(sizeof(AppState));
    state->running = true;
    
    // Initialize channels
    FURI_LOG_I(TAG, "Initializing channels");
    for(uint8_t i = 0; i < MAX_CHANNELS; i++) {
        state->data[i].channel = i;
        state->data[i].detected = nrf24_detect_signal(i);
    }

    // Set up GUI
    Gui* gui = furi_record_open(RECORD_GUI);
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, render, state);
    view_port_input_callback_set(view_port, input_callback, state);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);
    
    FURI_LOG_I(TAG, "Entering main loop");
    
    // Main loop
    while(state->running) {
        view_port_update(view_port);
        furi_delay_ms(50);
    }
    
    FURI_LOG_I(TAG, "Exiting application");
    
    // Cleanup
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);
    free(state);
    
    return 0;
}
