/**
 * viper_bridge.cpp
 *
 * Implements the C-linkage bridge declared in viper_bridge.h.
 * Each function is a thin, exception-safe wrapper around the ViPER C++ class.
 *
 * Build: included automatically when CMakeLists.txt adds it to VIPERDSP_SOURCES.
 */

#define VIPERDSP_EXPORTS   // ensures __declspec(dllexport) in the header
#include "include/viper_bridge.h"
#include "viper/ViPER.h"

#include <cstring>   // memcpy
#include <vector>

/* ── Internal helpers ──────────────────────────────────────────────────────── */

static inline ViPER* cast(void* p) {
    return static_cast<ViPER*>(p);
}

/* ── Lifecycle ─────────────────────────────────────────────────────────────── */

extern "C" {

void* viper_create() {
    return new (std::nothrow) ViPER();
}

void viper_destroy(void* instance) {
    delete cast(instance);
}

void viper_set_sample_rate(void* instance, unsigned int rate) {
    if (!instance) return;
    cast(instance)->SetSamplingRate(rate);
    // Force a reset so all filters rebuild their coefficients at the new rate
    cast(instance)->resetAllEffects();
}

void viper_reset(void* instance) {
    if (!instance) return;
    cast(instance)->resetAllEffects();
}

/* ── Parameter dispatch ────────────────────────────────────────────────────── */

void viper_dispatch(
    void*        instance,
    int          param,
    int          val1,
    int          val2,
    int          val3,
    int          val4,
    unsigned int arr_size,
    signed char* arr
) {
    if (!instance) return;
    cast(instance)->DispatchCommand(param, val1, val2, val3, val4, arr_size, arr);
}

/* ── Audio processing ──────────────────────────────────────────────────────── */

void viper_process(void* instance, float* buffer, unsigned int frames) {
    if (!instance || !buffer || frames == 0) return;

    // ViPER::process() works on a std::vector<float>.
    // We wrap the caller's buffer to avoid an extra allocation on every call.
    const unsigned int total_samples = frames * 2; // stereo interleaved
    std::vector<float> vec(buffer, buffer + total_samples);

    cast(instance)->process(vec, frames);

    // Write results back to the caller's buffer
    std::memcpy(buffer, vec.data(), total_samples * sizeof(float));
}

/* ── Diagnostics ───────────────────────────────────────────────────────────── */

unsigned long long viper_get_process_time_ms(void* instance) {
    if (!instance) return 0;
    return cast(instance)->GetProcessTimeMs();
}

unsigned int viper_get_sample_rate(void* instance) {
    if (!instance) return 0;
    return cast(instance)->GetSamplingRate();
}

} // extern "C"
