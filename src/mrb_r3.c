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
#include "mruby/hash.h"
#include "mruby/string.h"
#include "mruby/data.h"
#include "mruby/variable.h"
#include "mruby/error.h"
#include "memory.h"
#include "r3.h"

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
    mrb_sym data_attr;
    mrb_value data_ary;

    data_attr = mrb_intern_lit(mrb, "data");
    data_ary = mrb_iv_get(mrb, self, data_attr);

    mrb_ary_push(mrb, data_ary, data);
}

static mrb_value
mrb_r3_f_init(mrb_state *mrb, mrb_value self)
{
    mrb_int capa, len;
    mrb_sym data_attr;
    DATA_PTR(self) = NULL;

    len = mrb_get_args(mrb, "|i", &capa);

    if (len == 0)
        capa = 5;

    if (capa <= 0)
        mrb_raise(mrb, E_RANGE_ERROR, "Capa cannot be lower then zero.");

    data_attr = mrb_intern_lit(mrb, "data");
    mrb_iv_set(mrb, self, data_attr, mrb_ary_new_capa(mrb, capa));

    DATA_PTR(self) = r3_tree_create(capa);

    return self;
}

static mrb_value
mrb_r3_f_add(mrb_state *mrb, mrb_value self)
{
    mrb_int path_len, method;
    char *path;
    R3Node *tree = DATA_PTR(self);
    mrb_value data;
    mrb_bool data_given;

    if (mrb_get_args(mrb, "s|io?", &path, &path_len, &method, &data, &data_given) == 1) {
        method = 0;
    }

    path = strdup(path);
    mrb_r3_chomp_path(path, &path_len);

    if (data_given) {
        mrb_r3_save_data(mrb, self, data);
        r3_tree_insert_routel(tree, method, path, path_len, mrb_ptr(data));
    } else {
        r3_tree_insert_routel(tree, method, path, path_len, NULL);
    }

    return mrb_nil_value();
}

static mrb_value
mrb_r3_f_compile(mrb_state *mrb, mrb_value self)
{
    int ret;
    char *errstr = NULL;
    R3Node *tree = DATA_PTR(self);

    mrb_get_args(mrb, "");

    ret = r3_tree_compile(tree, &errstr);

    if (errstr)
        mrb_sys_fail(mrb, errstr);

    return mrb_fixnum_value(ret);
}

static mrb_value
mrb_r3_f_matches(mrb_state *mrb, mrb_value self)
{
    mrb_int path_len, method;
    char *path;
    R3Node *tree = DATA_PTR(self);
    match_entry *entry;
    R3Route *route;

    if (mrb_get_args(mrb, "s|i", &path, &path_len, &method) == 1) {
        method = 0;
    }

    path = strdup(path);
    mrb_r3_chomp_path(path, &path_len);

    entry = match_entry_createl(path, path_len);
    entry->request_method = method;

    route = r3_tree_match_route(tree, entry);
    match_entry_free(entry);

    if (route) {
        return mrb_true_value();
    }

    return mrb_false_value();
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
    mrb_int path_len, method, i, data_given = 0;
    char *path;
    R3Node *tree;
    R3Route *route;
    match_entry *entry;
    r3_iovec_t *slugs, *tokens;
    mrb_value params, data, val, key;

    if (mrb_get_args(mrb, "s|i", &path, &path_len, &method) == 1) {
        method = 0;
    }

    path = strdup(path);
    mrb_r3_chomp_path(path, &path_len);

    entry                 = match_entry_createl(path, path_len);
    entry->request_method = method;
    tree                  = DATA_PTR(self);
    route                 = r3_tree_match_route(tree, entry);

    if (!route) {
        match_entry_free(entry);
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

    if (data_given == 0)
        return params;

    return mrb_assoc_new(mrb, params, data);
}

static mrb_value
mrb_r3_f_free(mrb_state *mrb, mrb_value self)
{
    R3Node *tree;
    mrb_sym data;

    tree = DATA_PTR(self);
    data = mrb_intern_lit(mrb, "data");

    if (!tree)
        return mrb_false_value();

    mrb_iv_remove(mrb, self, data);
    r3_tree_free(tree);
    DATA_PTR(self) = NULL;

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
