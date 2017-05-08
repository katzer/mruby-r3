/*
** mrb_r3.c - R3 class
**
** Copyright (c) AMEMIYA Satoshi 2015
**
** See Copyright Notice in LICENSE
*/

#define HAVE_STRNDUP
#define HAVE_STRDUP

#include "mruby.h"
#include "mruby/array.h"
#include "mruby/string.h"
#include "mruby/data.h"
#include "mruby/hash.h"
#include "mruby/error.h"
#include "r3.h"

static mrb_value
mrb_r3_f_init(mrb_state *mrb, mrb_value self)
{
    mrb_int capa;
    DATA_PTR(self) = NULL;

    mrb_get_args(mrb, "|i", &capa);

    if (!capa)
        capa = 5;

    DATA_PTR(self) = r3_tree_create(capa);

    return self;
}

static mrb_value
mrb_r3_f_add(mrb_state *mrb, mrb_value self)
{
    mrb_int path_len, method, len;
    char *path;
    R3Node *tree = DATA_PTR(self);

    len = mrb_get_args(mrb, "s|i", &path, &path_len, &method);

    if (len == 2) {
        r3_tree_insert_routel(tree, method, path, path_len, NULL);
    } else {
        r3_tree_insert_pathl(tree, path, path_len, NULL);
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
    mrb_int path_len, method, len;
    char *path;
    R3Node *tree = DATA_PTR(self);
    match_entry *entry;
    R3Route *matched_route;

    len = mrb_get_args(mrb, "s|i", &path, &path_len, &method);

    entry         = match_entry_createl(path, path_len);
    matched_route = r3_tree_match_route(tree, entry);
    match_entry_free(entry);

    if (matched_route) {
        if (matched_route->request_method != method)
            return mrb_false_value();

        return mrb_true_value();
    }

    if (r3_tree_matchl(tree, path, path_len, NULL))
        return mrb_true_value();

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
//     int i, len;
//     mrb_value r3,
//               path,
//               data_key,
//               match,
//               params_key,
//               params_value;
//     match_entry *entry;
//     R3Route *matched_route;
//     R3Node *tree = DATA_PTR(self);
//     mrb_get_args(mrb, "iS", &r3, &path);

//     match = mrb_hash_new(mrb);
//     params_value = mrb_ary_new(mrb);
//     data_key = mrb_symbol_value(mrb_intern_cstr(mrb, "data"));
//     params_key = mrb_symbol_value(mrb_intern_cstr(mrb, "params"));
//     entry = match_entry_create(mrb_str_to_cstr(mrb, path));
//     entry->request_method = mrb_fixnum(r3);

//     matched_route = r3_tree_match_route(tree, entry);
//     if (matched_route != NULL) {
//         mrb_hash_set(mrb, match, data_key, mrb_obj_value(matched_route->data));
//     }
//     for (i = 0, len = entry->vars.tokens.size; i < len; i++) {
//         mrb_ary_push(mrb, params_value, mrb_str_new_cstr(mrb, entry->vars.tokens.entries.base[i]));
//     }
//     // match_entry_free(entry);
//     // mrb_hash_set(mrb, match, params_key, params_value);

//     return match;
}

void
mrb_mruby_r3_gem_init(mrb_state *mrb)
{
    struct RClass *r3, *tr;

    r3 = mrb_define_module(mrb, "R3");
    mrb_define_const(mrb, r3, "GET",     mrb_fixnum_value(METHOD_GET));
    mrb_define_const(mrb, r3, "POST",    mrb_fixnum_value(METHOD_POST));
    mrb_define_const(mrb, r3, "PUT",     mrb_fixnum_value(METHOD_PUT));
    mrb_define_const(mrb, r3, "DELETE",  mrb_fixnum_value(METHOD_DELETE));
    mrb_define_const(mrb, r3, "PATCH",   mrb_fixnum_value(METHOD_PATCH));
    mrb_define_const(mrb, r3, "HEAD",    mrb_fixnum_value(METHOD_HEAD));
    mrb_define_const(mrb, r3, "OPTIONS", mrb_fixnum_value(METHOD_OPTIONS));

    tr = mrb_define_class_under(mrb, r3, "Tree", mrb->object_class);
    mrb_define_method(mrb, tr, "initialize", mrb_r3_f_init, MRB_ARGS_OPT(1));
    mrb_define_method(mrb, tr, "add",        mrb_r3_f_add, MRB_ARGS_ARG(1,1));
    mrb_define_method(mrb, tr, "<<",         mrb_r3_f_add, MRB_ARGS_ARG(1,1));
    mrb_define_method(mrb, tr, "compile",    mrb_r3_f_compile, MRB_ARGS_NONE());
    mrb_define_method(mrb, tr, "match?",     mrb_r3_f_matches, MRB_ARGS_ARG(1,1));
    mrb_define_method(mrb, tr, "mismatch?",  mrb_r3_f_mismatches, MRB_ARGS_ARG(1,1));
    mrb_define_method(mrb, tr, "match",      mrb_r3_f_match, MRB_ARGS_ARG(1,1));
}

void
mrb_mruby_r3_gem_final(mrb_state *mrb)
{

}
