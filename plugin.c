/*
 * Copyright (C) 2010 Lars Hall
 *
 * This file is part of BranchCov.

 * BranchCov is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * BranchCov is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with BranchCov.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "gcc-plugin.h"
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "cp/cp-tree.h"
#include "toplev.h"
#include "basic-block.h"
#include "gimple.h"
#include "tree.h"
#include "tree-pass.h"
#include "intl.h"
#include "diagnostic.h"
#include "c-common.h"
#include "c-pragma.h"

#include "trace.h"

int plugin_is_GPL_compatible;

void read_trace_file(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Cannot read trace file\n");
        exit(1);
    }
    else
    {
        tf_len = 0;
        char line[MAX_FUNCTION_SCOPE_LEN];
        memset(line, 0, MAX_FUNCTION_SCOPE_LEN);
        while (fgets(line, MAX_FUNCTION_SCOPE_LEN, file) != NULL)
        {
            int len = strlen(line);
            if (line[len-1] == '\n')
                len --;

            strncpy(trace_functions[tf_len], line, len);
            trace_functions[tf_len++][len] = '\0';
        }
        fclose(file);
    }
}

void handle_pre_generic (void *event_data, void *data)
{
    if (errorcount || sorrycount)
        return;

    tree fndecl = (tree) event_data;
    printf("BranchCover: processing:%s\n", input_filename);
    trace_function(fndecl);
}

int plugin_init (struct plugin_name_args *plugin_info,
    struct plugin_gcc_version *version)
{
    // default
    trace_call_type = PRINTF; 

    if (plugin_info->argc > 0)
    {
        int i;
        for (i = 0; i < plugin_info->argc; i++)
        {
            if (strcmp(plugin_info->argv[i].key, "calltype") == 0)
            {
                if (strcmp(plugin_info->argv[i].value, "callback") == 0)
                    trace_call_type = CALLBACK;
            }
            if (strcmp(plugin_info->argv[i].key, "tracefile") == 0)
            {
                read_trace_file(plugin_info->argv[i].value);
            }
        }
    }

    printf("\nAnalyzing branches..\n");

    // Note: PLUGIN_PRE_GENERICIZE has more options for traversing modifying
    // and will call handle_pre_generic for each function
    register_callback (plugin_info->base_name,
        PLUGIN_PRE_GENERICIZE, &handle_pre_generic, NULL);
   
    return 0;
}
