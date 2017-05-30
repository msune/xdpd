#include "config_rss.h"

//static struct lcore_params lcore_params_array[MAX_LCORE_PARAMS];
struct lcore_params lcore_params_array_default[] = {
	{0, 0, 2},
	{0, 1, 2},
	{0, 2, 2},
	{1, 0, 2},
	{1, 1, 2},
	{1, 2, 2},
	{2, 0, 2},
	{3, 0, 3},
	{3, 1, 3},
};

struct lcore_params * lcore_params = lcore_params_array_default;
uint16_t nb_lcore_params = sizeof(lcore_params_array_default) /
						sizeof(struct lcore_params);
