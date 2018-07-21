/* MIT License
 *
 * Copyright (c) 2017 Sebastian Katzer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "mruby.h"
#include "mruby/array.h"
#include "mruby/class.h"
#include "mruby/hash.h"
#include "mruby/string.h"
#include "mruby/data.h"
#include "mruby/variable.h"
#include "mruby/error.h"
#include "memory.h"
#include "r3.h"
#include <stdio.h>

#ifdef _MSC_VER
# define strdup _strdup
#endif

static void
mrb_r3_tree_free(mrb_state *mrb, void *p)
{
    if (!p) { return; }

    r3_tree_free((R3Node*)p);
}

static mrb_data_type const mrb_r3_tree_type = { "R3::Tree", mrb_r3_tree_free };

static void
mrb_r3_chomp_path(char *path, mrb_int *len)
{
    if (*len == 1 || path[*len - 1] != '/')
        return;

    *len -= 1;
    path[*len] = '\0';
}

static void
mrb_r3_save_data(mrb_state *mrb, mrb_value self, mrb_value data)
{
    mrb_sym attr;
    mrb_value ary;

    attr = mrb_intern_lit(mrb, "data");
    ary  = mrb_iv_get(mrb, self, attr);

    mrb_ary_push(mrb, ary, data);
}

static void
mrb_r3_save_route(mrb_state *mrb, mrb_value self, mrb_int method, const char *route, int len)
{
    mrb_sym attr;
    mrb_value ary, data;
#ifdef _MSC_VER
    char buf[256];
#else
    char buf[len + 8];
#endif

    attr = mrb_intern_lit(mrb, "@routes");
    ary  = mrb_iv_get(mrb, self, attr);

    switch (method) {
        case METHOD_GET:
            sprintf(buf, "GET %s", route);
            break;
        case METHOD_POST:
            sprintf(buf, "POST %s", route);
            break;
        case METHOD_PUT:
            sprintf(buf, "PUT %s", route);
            break;
        case METHOD_PATCH:
            sprintf(buf, "PATCH %s", route);
            break;
        case METHOD_OPTIONS:
            sprintf(buf, "OPTIONS %s", route);
            break;
        case METHOD_DELETE:
            sprintf(buf, "DELETE %s", route);
            break;
        case METHOD_HEAD:
            sprintf(buf, "HEAD %s", route);
            break;
        default:
            sprintf(buf, "ANY %s", route);
            break;
    }

    data = mrb_str_new_cstr(mrb, buf);
    mrb_ary_push(mrb, ary, data);
}

static mrb_value
mrb_r3_f_init(mrb_state *mrb, mrb_value self)
{
    mrb_int capa = 5;
    mrb_sym data, routes;

    mrb_get_args(mrb, "|i", &capa);

    if (capa <= 0)
        mrb_raise(mrb, E_RANGE_ERROR, "Capa cannot be lower then zero.");

    data = mrb_intern_lit(mrb, "data");
    mrb_iv_set(mrb, self, data, mrb_ary_new_capa(mrb, capa));

    routes = mrb_intern_lit(mrb, "@routes");
    mrb_iv_set(mrb, self, routes, mrb_ary_new_capa(mrb, capa));

    mrb_data_init(self, r3_tree_create((int)capa), &mrb_r3_tree_type);

    return self;
}

static mrb_value
mrb_r3_f_add(mrb_state *mrb, mrb_value self)
{
    mrb_int path_len, method = 0;
    const char *path;
    R3Node *tree = DATA_PTR(self);
    mrb_value data = mrb_nil_value();
    mrb_bool data_given;
    mrb_value path_str;

    mrb_get_args(mrb, "s|io?", &path, &path_len, &method, &data, &data_given);

    path_str = mrb_str_new(mrb, path, path_len);
    path     = mrb_string_value_ptr(mrb, path_str);
    mrb_r3_chomp_path((char *)path, &path_len);

    if (data_given) {
        mrb_r3_save_data(mrb, self, data);
        r3_tree_insert_routel(tree, (int)method, path, (int)path_len, mrb_ptr(data));
    } else {
        r3_tree_insert_routel(tree, (int)method, path, (int)path_len, NULL);
    }

    mrb_r3_save_data(mrb, self, path_str);
    mrb_r3_save_route(mrb, self, method, path, (int)path_len);

    return mrb_nil_value();
}

static mrb_value
mrb_r3_f_compile(mrb_state *mrb, mrb_value self)
{
    int ret;
    char *err = NULL;
    R3Node *tree = DATA_PTR(self);

    mrb_get_args(mrb, "");

    ret = r3_tree_compile(tree, &err);

    if (err)
        mrb_sys_fail(mrb, err);

    return mrb_fixnum_value(ret);
}

static mrb_value
mrb_r3_f_matches(mrb_state *mrb, mrb_value self)
{
    mrb_int path_len, method = 0;
    char *path;
    R3Node *tree = DATA_PTR(self);
    match_entry *entry;
    R3Route *route;

    mrb_get_args(mrb, "s|i", &path, &path_len, &method);

    path = strdup(path);
    mrb_r3_chomp_path(path, &path_len);

    entry = match_entry_createl(path, (int)path_len);
    entry->request_method = (int)method;

    route = r3_tree_match_route(tree, entry);

    match_entry_free(entry);
    mrb_free(mrb, path);

    return mrb_bool_value(route? TRUE : FALSE);
}

static mrb_value
mrb_r3_f_mismatches(mrb_state *mrb, mrb_value self)
{
    if (mrb_type(mrb_r3_f_matches(mrb, self)) == MRB_TT_FALSE)
        return mrb_true_value();

    return mrb_false_value();
}

static mrb_value
mrb_r3_f_match(mrb_state *mrb, mrb_value self)
{
    mrb_int path_len, method = 0, i, data_given = 0;
    char *path;
    R3Node *tree;
    R3Route *route;
    match_entry *entry;
    r3_iovec_t *slugs, *tokens;
    mrb_value params, data, val, key;

    mrb_get_args(mrb, "s|i", &path, &path_len, &method);

    path = strdup(path);
    mrb_r3_chomp_path(path, &path_len);

    entry                 = match_entry_createl(path, (int)path_len);
    entry->request_method = (int)method;
    tree                  = DATA_PTR(self);
    route                 = r3_tree_match_route(tree, entry);

    if (!route) {
        match_entry_free(entry);
        mrb_free(mrb, path);
        return mrb_nil_value();
    }

    if (route->data) {
        data_given = 1;
        data = mrb_obj_value(route->data);
    }

    params = mrb_hash_new(mrb);
    slugs  = entry->vars.slugs.entries;
    tokens = entry->vars.tokens.entries;

    for (i = 0; i < entry->vars.slugs.size; i++) {
        key = mrb_str_new_static(mrb, slugs[i].base, slugs[i].len);
        val = mrb_str_new(mrb, tokens[i].base, tokens[i].len);

        mrb_hash_set(mrb, params, mrb_str_intern(mrb, key), val);
    }

    match_entry_free(entry);
    mrb_free(mrb, path);

    if (data_given == 0)
        return params;

    return mrb_assoc_new(mrb, params, data);
}

static mrb_value
mrb_r3_f_free(mrb_state *mrb, mrb_value self)
{
    R3Node *tree;
    mrb_value routes;

    tree = DATA_PTR(self);

    if (!tree)
        return mrb_false_value();

    routes = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@routes"));
    mrb_ary_clear(mrb, routes);

    mrb_iv_remove(mrb, self, mrb_intern_lit(mrb, "data"));
    r3_tree_free(tree);

    DATA_PTR(self)  = NULL;
    DATA_TYPE(self) = NULL;

    return mrb_true_value();
}

void
mrb_mruby_r3_gem_init(mrb_state *mrb)
{
    struct RClass *r3, *tr;

    r3 = mrb_define_module(mrb, "R3");
    mrb_define_const(mrb, r3, "ANY",     mrb_fixnum_value(0));
    mrb_define_const(mrb, r3, "GET",     mrb_fixnum_value(METHOD_GET));
    mrb_define_const(mrb, r3, "POST",    mrb_fixnum_value(METHOD_POST));
    mrb_define_const(mrb, r3, "PUT",     mrb_fixnum_value(METHOD_PUT));
    mrb_define_const(mrb, r3, "DELETE",  mrb_fixnum_value(METHOD_DELETE));
    mrb_define_const(mrb, r3, "PATCH",   mrb_fixnum_value(METHOD_PATCH));
    mrb_define_const(mrb, r3, "HEAD",    mrb_fixnum_value(METHOD_HEAD));
    mrb_define_const(mrb, r3, "OPTIONS", mrb_fixnum_value(METHOD_OPTIONS));

    tr = mrb_define_class_under(mrb, r3, "Tree", mrb->object_class);
    MRB_SET_INSTANCE_TT(tr, MRB_TT_DATA);
    mrb_define_method(mrb, tr, "initialize", mrb_r3_f_init, MRB_ARGS_OPT(1));
    mrb_define_method(mrb, tr, "add",        mrb_r3_f_add, MRB_ARGS_ARG(1,2));
    mrb_define_method(mrb, tr, "<<",         mrb_r3_f_add, MRB_ARGS_ARG(1,2));
    mrb_define_method(mrb, tr, "compile",    mrb_r3_f_compile, MRB_ARGS_NONE());
    mrb_define_method(mrb, tr, "match?",     mrb_r3_f_matches, MRB_ARGS_ARG(1,1));
    mrb_define_method(mrb, tr, "mismatch?",  mrb_r3_f_mismatches, MRB_ARGS_ARG(1,1));
    mrb_define_method(mrb, tr, "match",      mrb_r3_f_match, MRB_ARGS_ARG(1,1));
    mrb_define_method(mrb, tr, "free",       mrb_r3_f_free, MRB_ARGS_NONE());
}

void
mrb_mruby_r3_gem_final(mrb_state *mrb)
{

}
