#include <lv2/core/lv2.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#define DL_URI "https://hannesbraun.net/ns/lv2/delta-limiter"

typedef enum {
	INPUT_L = 0,
	INPUT_R = 1,
	OUTPUT_L = 2,
	OUTPUT_R = 3,
	GAIN_IN = 4,
	LIMIT = 5,
	GAIN_OUT = 6,
	DRY_WET = 7
} PortIndex;

typedef struct {
	// Port buffers
	const float* gain_in;
	const float* limit;
	const float* gain_out;
	const float* dry_wet;
	const float* input_l;
	const float* input_r;
	float* output_l;
	float* output_r;

	// Last output value (before amplification)
	float last_out_l;
	float last_out_r;
} Delta_Limiter;

static LV2_Handle instantiate(
	const LV2_Descriptor* descriptor,
	double rate,
	const char* bundle_path,
	const LV2_Feature* const* features)
{
	Delta_Limiter* dl = (Delta_Limiter*) calloc(1, sizeof(Delta_Limiter));
	return (LV2_Handle) dl;
}

static void connect_port(LV2_Handle instance, uint32_t port, void* data) {
	Delta_Limiter* dl = (Delta_Limiter*) instance;

	switch ((PortIndex) port) {
	case INPUT_L:
		dl->input_l = (const float*) data;
		break;
	case INPUT_R:
		dl->input_r = (const float*) data;
		break;
	case OUTPUT_L:
		dl->output_l = (float*) data;
		break;
	case OUTPUT_R:
		dl->output_r = (float*) data;
		break;
	case GAIN_IN:
		dl->gain_in = (const float*) data;
		break;
	case LIMIT:
		dl->limit = (const float*) data;
		break;
	case GAIN_OUT:
		dl->gain_out = (const float*) data;
		break;
	case DRY_WET:
		dl->dry_wet = (const float*) data;
	}
}

static void activate(LV2_Handle instance) {
	Delta_Limiter* dl = (Delta_Limiter*) instance;
	dl->last_out_l = 0.0;
	dl->last_out_r = 0.0;
}

static void run(LV2_Handle instance, uint32_t n_samples) {
	Delta_Limiter* dl = (Delta_Limiter*) instance;

	const float* const input_l = dl->input_l;
	const float* const input_r = dl->input_r;
	float* const output_l = dl->output_l;
	float* const output_r = dl->output_r;
	const float gain_in = *(dl->gain_in);
	const float limit = 0.2 * powf(*(dl->limit), 2.0);
	const float gain_out = *(dl->gain_out);
	const float dry_wet = *(dl->dry_wet);
	float last_out_l = dl->last_out_l;
	float last_out_r = dl->last_out_r;

	float l, r;
	float delta;
	float fade_out = (limit < 0.0001f) ? limit / 0.0001 : 1.0f;
	for (uint32_t pos = 0; pos < n_samples; pos++) {
		// Apply input gain
		l = input_l[pos] * gain_in;
		r = input_r[pos] * gain_in;

		// Manual loop unrolling instead of using an array for left/right

		// Determine delta
		delta = l - last_out_l;

		// Clip delta
		if (delta > limit) {
			delta = limit;
		} else if (delta < -limit) {
			delta = -limit;
		}

		// Calculate resulting signal for left channel
		l = last_out_l + delta;

		// Determine delta
		delta = r - last_out_r;

		// Clip delta
		if (delta > limit) {
			delta = limit;
		} else if (delta < -limit) {
			delta = -limit;
		}

		// Calculate resulting signal for right channel
		r = last_out_r + delta;

		// Remember last output value
		last_out_l = l;
		last_out_r = r;

		// Apply output gain, fadeout and dry/wet
		output_l[pos] = dry_wet * l * gain_out * fade_out + (1 - dry_wet) * input_l[pos];
		output_r[pos] = dry_wet * r * gain_out * fade_out + (1 - dry_wet) * input_r[pos];
	}

	dl->last_out_l = last_out_l;
	dl->last_out_r = last_out_r;
}

static void deactivate(LV_Handle) {}

static void cleanup(LV2_Handle instance) {
	free(instance);
}

static const void* extension_data(const char* uri) {
	return NULL;
}

static const LV2_Descriptor descriptor = {
	DL_URI,
	instantiate,
	connect_port,
	activate,
	run,
	deactivate,
	cleanup,
	extension_data
};

LV2_SYMBOL_EXPORT const LV2_Descriptor* lv2_descriptor(uint32_t index) {
	return index == 0 ? &descriptor : NULL;
}
