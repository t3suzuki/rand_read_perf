#pragma once
#include <cstdint>
#include <cassert>
#include <cstdlib>
#include <cmath>



#define MYMIN(a,b) (((a)<(b))?(a):(b))

struct zipf {
	uint64_t	range;
	double		alpha;
	double		eta;
	double		theta;
	double		zetan;
	double		val1_limit;
	uint32_t	seed;
};

static inline double
zeta_increment(uint64_t n, double theta)
{
	return pow((double) 1.0 / (n + 1), theta);
}

static inline double
zeta(uint64_t range, double theta)
{
	double zetan = 0;
	double inc1, inc2;
	uint64_t i, calc, count;
	const uint32_t ZIPF_MAX_ZETA_CALC = 10 * 1000 * 1000;
	const uint32_t ZIPF_ZETA_ESTIMATE = 1 * 1000 * 1000;

	/* Cumulate zeta discretely for the first ZIPF_MAX_ZETA_CALC
	 * entries in the range.
	 */
	calc = MYMIN(ZIPF_MAX_ZETA_CALC, range);
	for (i = 0; i < calc; i++) {
		zetan += zeta_increment(i, theta);
	}

	/* For the remaining values in the range, increment zetan
	 * with an approximation for every ZIPF_ZETA_ESTIMATE
	 * entries.  We will take an average of the increment
	 * for (i) and (i + ZIPF_ZETA_ESTIMATE), and then multiply
	 * that by ZIPF_ZETA_ESTIMATE.
	 *
	 * Of course, we'll cap ZIPF_ZETA_ESTIMATE to something
	 * smaller if necessary at the end of the range.
	 */
	while (i < range) {
		count = MYMIN(ZIPF_ZETA_ESTIMATE, range - i);
		inc1 = zeta_increment(i, theta);
		inc2 = zeta_increment(i + count, theta);
		zetan += (inc1 + inc2) * count / 2;
		i += count;
	}

	return zetan;
}

static inline struct zipf *
zipf_create(uint64_t range, double theta, uint32_t seed)
{
	struct zipf *zipf;

	zipf = (struct zipf *)calloc(1, sizeof(*zipf));
	if (zipf == NULL) {
		return NULL;
	}

	zipf->range = range;
	zipf->seed = seed;

	zipf->theta = theta;
	zipf->alpha = 1.0 / (1.0 - zipf->theta);
	zipf->zetan = zeta(range, theta);
	zipf->eta = (1.0 - pow(2.0 / zipf->range, 1.0 - zipf->theta)) /
		    (1.0 - zeta(2, theta) / zipf->zetan);
	zipf->val1_limit = 1.0 + pow(0.5, zipf->theta);

	return zipf;
}

static inline void
zipf_free(struct zipf **zipfp)
{
	assert(zipfp != NULL);
	free(*zipfp);
	*zipfp = NULL;
}

static inline uint64_t
zipf_generate(struct zipf *zipf)
{
	double randu, randz;
	uint64_t val;

	randu = (double)rand_r(&zipf->seed) / RAND_MAX;
	randz = randu * zipf->zetan;

	if (randz < 1.0) {
		return 0;
	} else if (randz < zipf->val1_limit) {
		return 1;
	} else {
		val = zipf->range * pow(zipf->eta * (randu - 1.0) + 1.0, zipf->alpha);
		return val % zipf->range;
	}
}
