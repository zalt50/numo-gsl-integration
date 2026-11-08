#include "integration_internal.h"

typedef struct {
  gsl_function *function;
  double a;
  double b;
  size_t order;
  int order_given;
  VALUE table_value;
  integration_glfixed_table_use table_use;
  double *result;
} integration_glfixed_params;

static VALUE integration_run_glfixed(VALUE data) {
  integration_glfixed_params *params = (integration_glfixed_params *)data;

  integration_glfixed_table_use_begin(params->table_value, &params->order,
                                      params->order_given, &params->table_use);
  *(params->result) = gsl_integration_glfixed(
      params->function, params->a, params->b, params->table_use.table);

  return Qnil;
}

static VALUE integration_run_glfixed_with_table(VALUE data) {
  integration_glfixed_params *params = (integration_glfixed_params *)data;

  return rb_ensure(integration_run_glfixed, data,
                   integration_glfixed_table_use_end,
                   (VALUE)&params->table_use);
}

VALUE integration_glfixed(int argc, VALUE *argv, VALUE self) {
  VALUE a;
  VALUE b;
  VALUE options;
  VALUE order_value = Qundef;
  VALUE table_value = Qundef;
  ID keywords[] = {rb_intern("order"), rb_intern("table")};
  VALUE keyword_values[2];
  size_t order = 10;
  int order_given;
  gsl_function function;
  integration_callback_params params;
  integration_glfixed_params glfixed_params = {0};
  double result;
  int state = 0;

  (void)self;

  rb_scan_args(argc, argv, "2:", &a, &b, &options);
  if (!NIL_P(options)) {
    rb_get_kwargs(options, keywords, 0, 2, keyword_values);
    order_value = keyword_values[0];
    table_value = keyword_values[1];
  }

  order_given = order_value != Qundef;
  if (order_given) {
    if (!RB_INTEGER_TYPE_P(order_value)) {
      rb_raise(rb_eTypeError, "order must be an Integer");
    }
    if (RTEST(rb_funcall(order_value, rb_intern("<="), 1, INT2FIX(0)))) {
      rb_raise(rb_eArgError, "order must be positive");
    }
    order = NUM2SIZET(order_value);
  }

  integration_function_init(&function, &params);

  glfixed_params.function = &function;
  glfixed_params.a = NUM2DBL(a);
  glfixed_params.b = NUM2DBL(b);
  glfixed_params.order = order;
  glfixed_params.order_given = order_given;
  glfixed_params.table_value = table_value;
  glfixed_params.result = &result;

  rb_protect(integration_run_glfixed_with_table, (VALUE)&glfixed_params,
             &state);

  RB_GC_GUARD(params.block);
  RB_GC_GUARD(params.exception);
  RB_GC_GUARD(table_value);

  integration_finish_callback(&params, state);

  return DBL2NUM(result);
}
