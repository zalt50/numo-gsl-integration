#include "integration_internal.h"

typedef struct {
  gsl_function *function;
  const gsl_integration_fixed_type *type;
  size_t order;
  double a;
  double b;
  double alpha;
  gsl_integration_fixed_workspace *workspace;
  double *result;
} integration_fixed_params;

static const gsl_integration_fixed_type *integration_fixed_type(VALUE type) {
  ID type_id;

  if (!RB_TYPE_P(type, T_SYMBOL)) {
    rb_raise(rb_eTypeError, "type must be a Symbol");
  }

  type_id = SYM2ID(type);
  if (type_id == rb_intern("legendre")) {
    return gsl_integration_fixed_legendre;
  }
  if (type_id == rb_intern("chebyshev")) {
    return gsl_integration_fixed_chebyshev;
  }
  if (type_id == rb_intern("chebyshev2")) {
    return gsl_integration_fixed_chebyshev2;
  }
  if (type_id == rb_intern("gegenbauer")) {
    return gsl_integration_fixed_gegenbauer;
  }

  rb_raise(rb_eArgError, "unsupported fixed quadrature type: %s",
           rb_id2name(type_id));
}

static VALUE integration_free_fixed_workspace(VALUE data) {
  integration_fixed_params *params = (integration_fixed_params *)data;

  if (params->workspace != NULL) {
    gsl_integration_fixed_free(params->workspace);
  }

  return Qnil;
}

static VALUE integration_run_fixed(VALUE data) {
  integration_fixed_params *params = (integration_fixed_params *)data;

  params->workspace = gsl_integration_fixed_alloc(
      params->type, params->order, params->a, params->b, params->alpha, 0.0);
  if (params->workspace == NULL) {
    rb_memerror();
  }

  (void)gsl_integration_fixed(params->function, params->result,
                              params->workspace);
  return Qnil;
}

static VALUE integration_run_fixed_with_workspace(VALUE data) {
  return rb_ensure(integration_run_fixed, data,
                   integration_free_fixed_workspace, data);
}

VALUE integration_fixed(int argc, VALUE *argv, VALUE self) {
  VALUE type;
  VALUE options;
  ID keywords[] = {rb_intern("order"), rb_intern("a"), rb_intern("b"),
                   rb_intern("alpha")};
  VALUE keyword_values[4];
  VALUE order_value;
  VALUE alpha_value;
  size_t order;
  gsl_function function;
  integration_callback_params params;
  integration_fixed_params fixed_params;
  double result;
  int state = 0;

  (void)self;
  rb_scan_args(argc, argv, "1:", &type, &options);
  fixed_params.type = integration_fixed_type(type);
  if (NIL_P(options)) {
    options = rb_hash_new();
  }
  rb_get_kwargs(options, keywords, 3, 1, keyword_values);
  order_value = keyword_values[0];
  alpha_value = keyword_values[3];

  if (!RB_INTEGER_TYPE_P(order_value)) {
    rb_raise(rb_eTypeError, "order must be an Integer");
  }
  if (RTEST(rb_funcall(order_value, rb_intern("<="), 1, INT2FIX(0)))) {
    rb_raise(rb_eArgError, "order must be positive");
  }
  order = NUM2SIZET(order_value);

  integration_function_init(&function, &params);
  fixed_params.function = &function;
  fixed_params.order = order;
  fixed_params.a = NUM2DBL(keyword_values[1]);
  fixed_params.b = NUM2DBL(keyword_values[2]);
  if (fixed_params.type == gsl_integration_fixed_gegenbauer) {
    if (alpha_value == Qundef) {
      rb_raise(rb_eArgError, "alpha is required for gegenbauer");
    }
    fixed_params.alpha = NUM2DBL(alpha_value);
  } else {
    fixed_params.alpha = 0.0;
  }
  fixed_params.workspace = NULL;
  fixed_params.result = &result;

  rb_protect(integration_run_fixed_with_workspace, (VALUE)&fixed_params,
             &state);
  RB_GC_GUARD(params.block);
  RB_GC_GUARD(params.exception);
  integration_finish_callback(&params, state);

  return DBL2NUM(result);
}
