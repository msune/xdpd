/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _BACKGROUND_TASK_MANAGER_
#define _BACKGROUND_TASK_MANAGER_

#include <rofl_datapath.h>
#include <pthread.h>

//C++ extern C
ROFL_BEGIN_DECLS

/**
 * launches the main thread
 */
rofl_result_t launch_bgmanager(void);

rofl_result_t stop_bgmanager(void);

//C++ extern C
ROFL_END_DECLS

#endif //_BACKGROUND_TASK_MANAGER_
