#pragma once

/**
 * viper_bridge.h
 *
 * C-linkage wrapper for the ViPER C++ engine.
 * Go loads ViPERDSP.dll at runtime via LoadLibraryW + GetProcAddress
 * and calls these functions through syscall.SyscallN — no CGo needed.
 *
 * Parameter encoding (matches DispatchCommand internals):
 *   - Boolean flags : val1 = 0 | 1
 *   - Float values  : val1 = (int)(floatValue * 100)   → e.g. 6.5 dB = 650
 *   - Band index    : val1 = band (0-based), val2 = gain×100
 *   - Frequencies   : val1 = Hz (integer, no scaling)
 */

#ifdef _WIN32
  #ifdef VIPERDSP_EXPORTS
    #define VIPER_API __declspec(dllexport)
  #else
    #define VIPER_API __declspec(dllimport)
  #endif
#else
  #define VIPER_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ── Lifecycle ─────────────────────────────────────────────────────────────── */

/** Create a new ViPER engine instance. Returns opaque handle (never NULL). */
VIPER_API void* viper_create(void);

/** Destroy an engine instance and free all resources. */
VIPER_API void  viper_destroy(void* instance);

/** Set the sample rate (Hz). Must be called before the first process(). */
VIPER_API void  viper_set_sample_rate(void* instance, unsigned int rate);

/** Reset all internal effect state (call on stream restart). */
VIPER_API void  viper_reset(void* instance);

/* ── Parameter dispatch ────────────────────────────────────────────────────── */

/**
 * Send a parameter update to the engine.
 *
 * @param instance  Handle returned by viper_create()
 * @param param     PARAM_HP_* or PARAM_SPK_* constant from ViPERParams.h
 * @param val1      Primary integer value (see encoding table in header doc)
 * @param val2      Secondary integer value (used for band-level index)
 * @param val3      Tertiary integer value (rarely used)
 * @param val4      Quaternary integer value (rarely used)
 * @param arr_size  Length of the byte array (0 if unused)
 * @param arr       Byte array payload (NULL if arr_size == 0)
 */
VIPER_API void  viper_dispatch(
    void*        instance,
    int          param,
    int          val1,
    int          val2,
    int          val3,
    int          val4,
    unsigned int arr_size,
    signed char* arr
);

/* ── Audio processing ──────────────────────────────────────────────────────── */

/**
 * Process an interleaved stereo float32 buffer IN PLACE.
 *
 * @param instance  Handle returned by viper_create()
 * @param buffer    Interleaved L/R samples  [L0,R0, L1,R1, ...]
 * @param frames    Number of FRAMES (not samples). Total floats = frames * 2.
 */
VIPER_API void  viper_process(void* instance, float* buffer, unsigned int frames);

/* ── Diagnostics ───────────────────────────────────────────────────────────── */

/** Returns the last processing time in milliseconds (for performance monitoring). */
VIPER_API unsigned long long viper_get_process_time_ms(void* instance);

/** Returns the current sample rate set on this instance. */
VIPER_API unsigned int viper_get_sample_rate(void* instance);

#ifdef __cplusplus
} /* extern "C" */
#endif
