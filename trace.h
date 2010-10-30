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
#ifndef TRACE_H
#define TRACE_H

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
#include "tree-iterator.h"

#define MAX_DECL_NAME_LEN 1024
// Maximum number of nested namespace and nested classes
#define MAX_FUNCTION_SCOPE_LEN 1024 

enum trace_call_type
{
    CALLBACK = 1,
    PRINTF,
} trace_call_type;

enum branch_type
{
    IF_BRANCH = 1,
    ELSE_BRANCH,
    SWITCH_BRANCH
};

int tf_len;
char trace_functions[1024][MAX_FUNCTION_SCOPE_LEN];

char current_fn_name[MAX_DECL_NAME_LEN];
char current_sc[MAX_FUNCTION_SCOPE_LEN];

const char *get_decl_name(tree decl);
void trace_function(tree fn);
void traverse(tree t);
void traverse_stmt_list(tree stmt);
void traverse_class(tree t);
void instrument_trace_if(tree t);
const char *get_scope(tree decl, char *scope);
void instrument_trace(tree t, tree_stmt_iterator *iterator, int btype);

#endif
