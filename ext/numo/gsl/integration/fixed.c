#include "integration_internal.h"

enum {
  INTEGRATION_FIXED_REQUIRES_ALPHA = 1 << 0,
  INTEGRATION_FIXED_REQUIRES_BETA = 1 << 1
};

typedef struct {
  ID id;
  const char *name;
  const gsl_integration_fixed_type **type;
  unsigned int requirements;
} integration_fixed_type_info;

typedef struct {
  gsl_function *function;
  const gsl_integration_fixed_type *type;
  size_t order;
  double a;
  double b;
  double alpha;
  double beta;
  gsl_integration_fixed_workspace *workspace;
  double *result;
} integration_fixed_params;

static integration_fixed_type_info integration_fixed_types[] = {
    {0, "legendre", &gsl_integration_fixed_legendre, 0},
    {0, "chebyshev", &gsl_integration_fixed_chebyshev, 0},
    {0, "chebyshev2", &gsl_integration_fixed_chebyshev2, 0},
    {0, "gegenbauer", &gsl_integration_fixed_gegenbauer,
     INTEGRATION_FIXED_REQUIRES_ALPHA},
    {0, "jacobi", &gsl_integration_fixed_jacobi,
     INTEGRATION_FIXED_REQUIRES_ALPHA | INTEGRATION_FIXED_REQUIRES_BETA},
    {0, "laguerre", &gsl_integration_fixed_laguerre,
     INTEGRATION_FIXED_REQUIRES_ALPHA},
    {0, "hermite", &gsl_integration_fixed_hermite,
     INTEGRATION_FIXED_REQUIRES_ALPHA},
    {0, "exponential", &gsl_integration_fixed_exponential,
     INTEGRATION_FIXED_REQUIRES_ALPHA},
    {0, "rational", &gsl_integration_fixed_rational,
     INTEGRATION_FIXED_REQUIRES_ALPHA | INTEGRATION_FIXED_REQUIRES_BETA},
};

static ID integration_fixed_id_order;
static ID integration_fixed_id_a;
static ID integration_fixed_id_b;
static ID integration_fixed_id_alpha;
static ID integration_fixed_id_beta;
static ID integration_fixed_id_less_than_or_equal;

void integration_fixed_init(void) {
  size_t i;

  for (i = 0;
       i < sizeof(integration_fixed_types) / sizeof(integration_fixed_types[0]);
       ++i) {
    integration_fixed_types[i].id = rb_intern(integration_fixed_types[i].name);
  }

  integration_fixed_id_order = rb_intern("order");
  integration_fixed_id_a = rb_intern("a");
  integration_fixed_id_b = rb_intern("b");
  integration_fixed_id_alpha = rb_intern("alpha");
  integration_fixed_id_beta = rb_intern("beta");
  integration_fixed_id_less_than_or_equal = rb_intern("<=");
}

static const integration_fixed_type_info *integration_fixed_type(VALUE type) {
  ID type_id;
  size_t i;

  if (!RB_TYPE_P(type, T_SYMBOL)) {
    rb_raise(rb_eTypeError, "type must be a Symbol");
  }

  type_id = SYM2ID(type);
  for (i = 0;
       i < sizeof(integration_fixed_types) / sizeof(integration_fixed_types[0]);
       ++i) {
    if (integration_fixed_types[i].id == type_id) {
      return &integration_fixed_types[i];
    }
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

  params->workspace =
      gsl_integration_fixed_alloc(params->type, params->order, params->a,
                                  params->b, params->alpha, params->beta);
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
  ID keywords[] = {integration_fixed_id_order, integration_fixed_id_a,
                   integration_fixed_id_b, integration_fixed_id_alpha,
                   integration_fixed_id_beta};
  VALUE keyword_values[5];
  VALUE order_value;
  VALUE alpha_value;
  VALUE beta_value;
  const integration_fixed_type_info *type_info;
  size_t order;
  gsl_function function;
  integration_callback_params params;
  integration_fixed_params fixed_params = {0};
  double result;
  int state = 0;

  (void)self;
  rb_scan_args(argc, argv, "1:", &type, &options);
  type_info = integration_fixed_type(type);

  if (NIL_P(options)) {
    options = rb_hash_new();
  }
  rb_get_kwargs(options, keywords, 3, 2, keyword_values);

  order_value = keyword_values[0];
  alpha_value = keyword_values[3];
  beta_value = keyword_values[4];

  if (!RB_INTEGER_TYPE_P(order_value)) {
    rb_raise(rb_eTypeError, "order must be an Integer");
  }
  if (RTEST(rb_funcall(order_value, integration_fixed_id_less_than_or_equal, 1,
                       INT2FIX(0)))) {
    rb_raise(rb_eArgError, "order must be positive");
  }
  order = NUM2SIZET(order_value);

  if ((type_info->requirements & INTEGRATION_FIXED_REQUIRES_ALPHA) != 0 &&
      alpha_value == Qundef) {
    rb_raise(rb_eArgError, "alpha is required for %s", type_info->name);
  }
  if ((type_info->requirements & INTEGRATION_FIXED_REQUIRES_BETA) != 0 &&
      beta_value == Qundef) {
    rb_raise(rb_eArgError, "beta is required for %s", type_info->name);
  }

  integration_function_init(&function, &params);
  fixed_params.function = &function;
  fixed_params.type = *type_info->type;
  fixed_params.order = order;
  fixed_params.a = NUM2DBL(keyword_values[1]);
  fixed_params.b = NUM2DBL(keyword_values[2]);
  fixed_params.alpha =
      (type_info->requirements & INTEGRATION_FIXED_REQUIRES_ALPHA) != 0
          ? NUM2DBL(alpha_value)
          : 0.0;
  fixed_params.beta =
      (type_info->requirements & INTEGRATION_FIXED_REQUIRES_BETA) != 0
          ? NUM2DBL(beta_value)
          : 0.0;
  fixed_params.result = &result;

  rb_protect(integration_run_fixed_with_workspace, (VALUE)&fixed_params,
             &state);
  RB_GC_GUARD(params.block);
  RB_GC_GUARD(params.exception);
  integration_finish_callback(&params, state);
  return DBL2NUM(result);
}
