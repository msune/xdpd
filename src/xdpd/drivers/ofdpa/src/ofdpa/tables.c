#include "tables.h"

#include "../config.h"

bool ofdpa_is_valid_table_id(const unsigned int tid){
	unsigned int i;

	for(i=0; i<OFDPA_MAX_TABLE_LOOP; i++)
		if(ofdpa_table_set[i] == tid)
			return true;

	return false;
}
