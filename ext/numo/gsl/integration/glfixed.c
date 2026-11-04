#include "integration_internal.h"

typedef struct {
  gsl_function *function;
  double a;
  double b;
  size_t order;
  gsl_integration_glfixed_table *table;
  double *result;
} integration_glfixed_params;

static VALUE integration_free_glfixed_table(VALUE data) {
  gsl_integration_glfixed_table *table = (gsl_integration_glfixed_table *)data;

  gsl_integration_glfixed_table_free(table);

  return Qnil;
}

static VALUE integration_run_glfixed(VALUE data) {
  integration_glfixed_params *params = (integration_glfixed_params *)data;

  *(params->result) = gsl_integration_glfixed(params->function, params->a,
                                              params->b, params->table);

  return Qnil;
}

static VALUE integration_run_glfixed_with_table(VALUE data) {
  integration_glfixed_params *params = (integration_glfixed_params *)data;

  return rb_ensure(integration_run_glfixed, data,
                   integration_free_glfixed_table, (VALUE)params->table);
}

VALUE integration_glfixed(int argc, VALUE *argv, VALUE self) {
  VALUE a;
  VALUE b;
  VALUE options;
  VALUE order_value = Qundef;
  ID keywords[] = {rb_intern("order")};
  VALUE keyword_values[1];
  size_t order;
  gsl_function function;
  integration_callback_params params;
  integration_glfixed_params glfixed_params;
  double result;
  int state = 0;

  (void)self;

  rb_scan_args(argc, argv, "2:", &a, &b, &options);

  if (!NIL_P(options)) {
    rb_get_kwargs(options, keywords, 0, 1, keyword_values);
    order_value = keyword_values[0];
  }

  order = order_value == Qundef ? 10 : NUM2SIZET(order_value);
  if (order == 0) {
    rb_raise(integration_error_class(), "order must be positive");
  }

  integration_function_init(&function, &params);

  glfixed_params.function = &function;
  glfixed_params.a = NUM2DBL(a);
  glfixed_params.b = NUM2DBL(b);
  glfixed_params.order = order;
  glfixed_params.table = gsl_integration_glfixed_table_alloc(order);
  glfixed_params.result = &result;

  rb_protect(integration_run_glfixed_with_table, (VALUE)&glfixed_params,
             &state);

  RB_GC_GUARD(params.block);
  RB_GC_GUARD(params.exception);

  integration_finish_callback(&params, state);

  return DBL2NUM(result);
}
