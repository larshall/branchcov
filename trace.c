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
#include "trace.h"

const char *get_decl_name(tree decl)
{
    tree id = DECL_NAME (decl);
    return IDENTIFIER_POINTER(id);
}

void trace_function(tree fn)
{
    // Taken from gcc/tree.h DECL_EXTERNAL:
    // For example, for a FUNCTION_DECL, DECL_SAVED_TREE may
    // be non-NULL and DECL_EXTERNAL may be true 
    // simultaneously; that can be the case for a C99 
    // "extern inline" function.
    if (!DECL_IS_BUILTIN(fn) && (!DECL_EXTERNAL(fn)) && (DECL_SAVED_TREE(fn)))
    {
        int i;
        bool found = false;
        for (i = 0; i < tf_len; i++)
        {
            char scope[MAX_FUNCTION_SCOPE_LEN];
            const char *fn_name = get_scope(fn, scope);
            if (strcmp(trace_functions[i], scope) == 0)
            {
                memset(current_sc, 0, MAX_FUNCTION_SCOPE_LEN);
                memset(current_fn_name, 0, MAX_DECL_NAME_LEN);
                strcpy(current_fn_name, fn_name); 
                strcpy(current_sc, scope);
                found = true;
                break;
            }
        }

        if (found)
            traverse(DECL_SAVED_TREE(fn));
    }
}

// Traverses the ast/generic tree
void traverse(tree t)
{
    while (t != NULL)
    {
#if 0
        printf("\ntree code:%s",  tree_code_name[TREE_CODE(t)]);
#endif

        switch(TREE_CODE(t))
        {
            case BIND_EXPR:
                traverse(BIND_EXPR_BODY(t));
                break;
            case STATEMENT_LIST:
                traverse_stmt_list(t);
                break;
            case IF_STMT:
                instrument_trace_if(t);
                break;
            case SWITCH_STMT:
                traverse(SWITCH_BODY(t));
                break;
            case EXIT_EXPR:
                break;
            case WHILE_STMT:
                traverse(WHILE_BODY(t));
                break;
            case FOR_STMT:
                traverse(FOR_BODY(t));
                break;
            case DO_STMT:
                traverse(DO_BODY(t));
                break;
            case BLOCK:
                printf("\nTODO: Add block");
                break;
            case TRY_BLOCK:
                traverse(TRY_STMTS(t));
                break;
            case EH_SPEC_BLOCK:
                traverse(EH_SPEC_STMTS(t));
                break;
            case HANDLER:
                traverse(HANDLER_BODY(t));
                break;
            default:
                break;
        }

        t = TREE_CHAIN(t);
    }
}

void traverse_stmt_list(tree stmt)
{
    tree_stmt_iterator i = tsi_start(STATEMENT_LIST_CHECK(stmt));
    tree last_stmt = NULL;
    for (; !tsi_end_p(i); tsi_next(&i))
    {
        // If case_label_expr: instrument a trace after the case label.
        // If multiple cases is after eachother insert at the last case label
        // because of:
        // case x:
        // case y:
        // break;
        if ((last_stmt != NULL) && (TREE_CODE(last_stmt) == CASE_LABEL_EXPR))
        {
            if (TREE_CODE(*tsi_stmt_ptr(i)) != CASE_LABEL_EXPR)
                instrument_trace(*tsi_stmt_ptr(i), &i, SWITCH_BRANCH);
        }

        traverse(*tsi_stmt_ptr(i));
        last_stmt = *tsi_stmt_ptr(i);
    }
}

void traverse_class(tree t)
{
    tree type = TREE_TYPE(t);
    type = TYPE_MAIN_VARIANT(type);
    type = TYPE_FIELDS(type);

    // get nested classes
    while (type != NULL)
    {
        if (TREE_CODE(TREE_TYPE(type)) == RECORD_TYPE)
        {
            if (!DECL_SELF_REFERENCE_P(type))
                traverse_class(type);
        }
        type = TREE_CHAIN(type);
    }

    tree method = TYPE_METHODS(TREE_TYPE(t));

    while (method != NULL)
    {
        traverse(method);
        method = TREE_CHAIN(method);
    }
}

void instrument_trace_if(tree t)
{
    tree then = THEN_CLAUSE(t);
    if (then != NULL)
        instrument_trace(t, NULL, IF_BRANCH);
    
    tree el = ELSE_CLAUSE(t);
    if (el != NULL)
        instrument_trace(t, NULL, ELSE_BRANCH);
}

// Concats namespaces, classes and function name
const char *get_scope(tree decl, char *scope)
{
    const char *fn_name = get_decl_name(decl);
    memset(scope, 0, MAX_FUNCTION_SCOPE_LEN);
    const char *arr[MAX_FUNCTION_SCOPE_LEN];
    int len = 0;
    char tmp[MAX_DECL_NAME_LEN];


    if (DECL_DESTRUCTOR_P(decl))
    {
        sprintf(tmp, "%c%s", '~', fn_name);
        arr[len++] = tmp;
    }
    else
        arr[len++] = fn_name;
         
    tree t = CP_DECL_CONTEXT(decl);

    // get all the namespaces and nested classes
    // the function/method is a member of
    while (t != NULL)
    {
        if (t == global_namespace)
            break;

        tree tmp;
        if (TREE_CODE(t) == RECORD_TYPE)
            tmp = TYPE_NAME(t);
        else
            tmp = t;

        arr[len++] = get_decl_name(tmp);
        // Get nested classes/namespaces
        t = CP_DECL_CONTEXT(tmp);
    }

    int i = 0;
    int j = 0;
    for (i = len - 1; i >= 0; i--)
    {
        strcat(scope, arr[i]);
        if (i != 0)
            strcat(scope, "::");
    }

    return arr[0];
}

void instrument_trace(tree t, tree_stmt_iterator *iterator, int btype)
{
    tree bind_body = NULL;
    tree body = NULL;
    tree fn_id = NULL;
    tree fn_decl = NULL;
    tree params = NULL;

    int lineno = 0;

    if (btype == IF_BRANCH)
        body = THEN_CLAUSE(t);
    else if (btype == ELSE_BRANCH)
        body = ELSE_CLAUSE(t);
    else
        body = t;
    
    if (TREE_CODE(body) == BIND_EXPR)
    {
        // Need to remember the parent level, so when inserting,
        // the stmt list will get inserted at the BIND_EXPR code
        bind_body = body;
        body = BIND_EXPR_BODY(body);
    }
    
    if (TREE_CODE(body) == STATEMENT_LIST)
    {
        tree_stmt_iterator i = tsi_start(STATEMENT_LIST_CHECK(body));
        lineno = DECL_SOURCE_LINE(*tsi_stmt_ptr(i));
    }
    else
        lineno = DECL_SOURCE_LINE(body);

    printf("Instrumenting trace at: %s lineno: %i\n", current_sc, lineno);

    if (trace_call_type == PRINTF)
    {
        fn_id = get_identifier("printf");
        fn_decl = lookup_name(fn_id);

        if (fn_decl == NULL)
        {
            printf("\nIs printf accessible from here?");
            exit(1);
        }

        char str[MAX_FUNCTION_SCOPE_LEN + 1024];
        memset(str, 0, MAX_FUNCTION_SCOPE_LEN + 1024);
        sprintf(str, "##filename:%s;scope:%s;functionname:%s;lineno:%i\n",
            main_input_filename, current_sc, current_fn_name, lineno);
        // argh apparently the length of string needs to be +1?
        params = build_tree_list(NULL_TREE,
            build_string_literal(strlen(str), str));
    }
    else
    {
        fn_id = get_identifier("__tracer");
        fn_decl = lookup_name(fn_id);

        if (fn_decl == NULL)
        {
            printf("\nIs __tracer accessible from here?");
            exit(1);
        }

        params = build_tree_list(NULL_TREE, build_int_cst(
            long_integer_type_node, lineno));
        params = tree_cons(NULL_TREE, build_string_literal(
            strlen(current_fn_name) + 1, current_fn_name), params);
        params = tree_cons(NULL_TREE, build_string_literal(
            strlen(current_sc) + 1, current_sc), params);
        params = tree_cons(NULL_TREE, build_string_literal(
            strlen(main_input_filename) + 1, main_input_filename), params);
    }
 
    if (params != NULL)
    {
        tree fn_call = build_function_call_expr(
            DECL_SOURCE_LOCATION(body), fn_decl, params);
        tree stmt = build_stmt (DECL_SOURCE_LOCATION(body),
            EXPR_STMT, fn_call);

        if ((&iterator == NULL) && (TREE_CODE(body) == STATEMENT_LIST))
            *iterator = tsi_start(STATEMENT_LIST_CHECK(body));

        if (iterator != NULL)
            tsi_link_before(iterator, stmt, TSI_NEW_STMT); 
        else
        {
            // Push the stmt to then/else clause
            // Note: Need to go up 1 level, so this isn't possible:
            // t = push_stmt_list. Therefore we need to check the branch_type
            // again.
            tree stmt_list = push_stmt_list();

            append_to_statement_list(stmt, &stmt_list);
            // if bind_expr: add it to the parent scope (before BIND_EXPR_BODY)
            if (bind_body != NULL)
                 append_to_statement_list(bind_body, &stmt_list);
            else
                append_to_statement_list(body, &stmt_list);

            if (btype == IF_BRANCH)
                THEN_CLAUSE(t) = stmt_list;
            else if (btype == ELSE_BRANCH)
                ELSE_CLAUSE(t) = stmt_list;

            // TODO: Are these nedded?
            TREE_SIDE_EFFECTS(stmt) = 1;
            TREE_SIDE_EFFECTS(fn_decl) = 1;

            // Traverse further down the tree (the tree that just got
            // inserted in the stmt list)
            traverse(body);
        }
    }
}
