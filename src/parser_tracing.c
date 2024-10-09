#include "util.c"

#include <stdio.h>

#ifdef PARSER_TRACE
#define TRACE_PARSER_FUNC \
    const char *func_name __attribute__((cleanup(trace_function_cleanup))) = \
        __func__; \
    trace_function_start(__func__);
#else
#define TRACE_PARSER_FUNC
#endif

int trace_trace_level = 0;

const char trace_indent_tab[] = "    ";

gbString trace_alloc_indent_level_str() {
    gbString str = gb_make_string("");
    for (int i = 0; i < trace_trace_level; ++i) {
        str = gb_append_cstring(str, trace_indent_tab);
    }
    return str;
}

void trace_inc_indent() {
    trace_trace_level++;
}

void trace_dec_indent() {
    trace_trace_level--;
}

void trace_function_start(const char *func_name) {
    gbString tab = trace_alloc_indent_level_str();
    printf("%sBEGIN %s\n", tab, func_name);
    trace_inc_indent();
    gb_free_string(tab);
}

void trace_function_cleanup(const char **func_name) {
    trace_dec_indent();
    gbString tab = trace_alloc_indent_level_str();
    printf("%sEND %s\n", trace_alloc_indent_level_str(), *func_name);
    gb_free_string(tab);
}
