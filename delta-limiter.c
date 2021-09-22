#include "lv2/core/lv2.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define DL_URI "https://github.com/hannesbraun/delta-limiter"

typedef enum {
	INPUT_L = 0,
	INPUT_R = 1,
	OUTPUT_L = 2,
	OUTPUT_R = 3,
	GAIN_IN = 4,
	SHAPE = 5,
	LIMIT = 6,
	GAIN_OUT = 7
} PortIndex;

typedef struct {
	// Port buffers
	const float* gain_in;
	const float* limit;
	const float* gain_out;
	const float* input_l;
	const float* input_r;
	float* output_l;
	float* output_r;

	float last_out_l;
	float last_out_r;
} DeltaLimiter;

static LV2_Handle instantiate(
	const LV2_Descriptor* descriptor,
	double rate,
	const char* bundle_path,
	const LV2_Feature* const* features)
{
	DeltaLimiter* dl = (DeltaLimiter*) calloc(1, sizeof(DeltaLimiter));
	return (LV2_Handle) dl;
}

static void connect_port(LV2_Handle instance, uint32_t port, void* data) {
	DeltaLimiter* dl = (DeltaLimiter*) instance;

	switch ((PortIndex) port) {
	case GAIN_IN:
		dl->gain_in = (const float*) data;
		break;
	case SHAPE:
		dl->shape = (const float*) data;
		break;
	case LIMIT:
		dl->limit = (const float*) data;
		break;
	case GAIN_OUT:
		dl->gain_out = (const float*) data;
		break;
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
	}
}

static void activate(LV2_Handle instance) {
	DeltaLimiter* dl = (DeltaLimiter*) instance;
	dl->last_out_l = 0.0;
	dl->last_out_r = 0.0;
}

// #define DB_CO(g) ((g) > -90.0f ? powf(10.0f, (g)*0.05f) : 0.0f)

static void run(LV2_Handle instance, uint32_t n_samples) {
	DeltaLimiter* dl = (DeltaLimiter*) instance;

	const float gain_in = *(dl->gain_in);
	const float shape = *(dl->shape);
	const float limit = *(dl->limit);
	const float gain_out = *(dl->gain_out);
	const float* const input_l = dl->input_l;
	const float* const input_r = dl->input_r;
	float* const output_l = dl->output_l;
	float* const output_r = dl->output_r;
	float last_out_l = dl->last_out_l;
	float last_out_r = dl->last_out_r;

	// const float coef = DB_CO(gain);

	if (limit < 0.00003) {
		// Prevent DC
		memset(output_l, 0.0, n_samples * sizeof(float));
		memset(output_r, 0.0, n_samples * sizeof(float));
		dl->last_out_l = 0.0;
		dl->last_out_r = 0.0;
		return;
	}

	// tanh(xy)/tanh(y)

	float l, r;
	float delta;
	for (uint32_t pos = 0; pos < n_samples; pos++) {
		l = input_l[pos] * gain_in;
		r = input_r[pos] * gain_in;

		delta = l - last_out_l;
		if (delta > limit) {
			delta = limit;
		} else if (delta < -limit) {
			delta = -limit;
		}
		l = last_out_l + delta;

		delta = r - last_out_r;
		if (delta > limit) {
			delta = limit;
		} else if (delta < -limit) {
			delta = -limit;
		}
		r = last_out_r + delta;

		last_out_l = l;
		last_out_r = r;

		output_l[pos] = l * gain_out;
		output_r[pos] = r * gain_out;
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
