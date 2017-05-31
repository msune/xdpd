#include "config_rss.h"

//Set default values TODO remove
struct lcore_params lcore_params[LCORE_PARAMS_MAX] = {
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
uint16_t nb_lcore_params = 9;
