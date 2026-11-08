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
  const integration_fixed_type_info *type_info;
  size_t order;
  double a;
  double b;
  double alpha;
  double beta;
} integration_fixed_config;

typedef struct {
  gsl_integration_fixed_workspace *workspace;
  const integration_fixed_type_info *type_info;
  size_t order;
  double a;
  double b;
  double alpha;
  double beta;
  int busy;
} integration_fixed_workspace;

typedef struct {
  gsl_integration_fixed_workspace *workspace;
  integration_fixed_workspace *owner;
} integration_fixed_workspace_use;

typedef struct {
  gsl_function *function;
  gsl_integration_fixed_workspace *workspace;
  double *result;
} integration_fixed_params;

static VALUE integration_cFixedWorkspace;

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
static ID integration_fixed_id_workspace;
static ID integration_fixed_id_less_than_or_equal;

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

static void integration_fixed_config_parse(VALUE type, VALUE order_value,
                                           VALUE a_value, VALUE b_value,
                                           VALUE alpha_value, VALUE beta_value,
                                           integration_fixed_config *config) {
  const integration_fixed_type_info *type_info = integration_fixed_type(type);

  if (order_value == Qundef) {
    rb_raise(rb_eArgError, "missing keyword: :order");
  }
  if (a_value == Qundef) {
    rb_raise(rb_eArgError, "missing keyword: :a");
  }
  if (b_value == Qundef) {
    rb_raise(rb_eArgError, "missing keyword: :b");
  }
  if (!RB_INTEGER_TYPE_P(order_value)) {
    rb_raise(rb_eTypeError, "order must be an Integer");
  }
  if (RTEST(rb_funcall(order_value, integration_fixed_id_less_than_or_equal, 1,
                       INT2FIX(0)))) {
    rb_raise(rb_eArgError, "order must be positive");
  }
  if ((type_info->requirements & INTEGRATION_FIXED_REQUIRES_ALPHA) != 0 &&
      alpha_value == Qundef) {
    rb_raise(rb_eArgError, "alpha is required for %s", type_info->name);
  }
  if ((type_info->requirements & INTEGRATION_FIXED_REQUIRES_BETA) != 0 &&
      beta_value == Qundef) {
    rb_raise(rb_eArgError, "beta is required for %s", type_info->name);
  }

  config->type_info = type_info;
  config->order = NUM2SIZET(order_value);
  config->a = NUM2DBL(a_value);
  config->b = NUM2DBL(b_value);
  config->alpha =
      (type_info->requirements & INTEGRATION_FIXED_REQUIRES_ALPHA) != 0
          ? NUM2DBL(alpha_value)
          : 0.0;
  config->beta =
      (type_info->requirements & INTEGRATION_FIXED_REQUIRES_BETA) != 0
          ? NUM2DBL(beta_value)
          : 0.0;
}

static void integration_fixed_workspace_free(void *data) {
  integration_fixed_workspace *workspace = data;

  if (workspace->workspace != NULL) {
    gsl_integration_fixed_free(workspace->workspace);
  }
  xfree(workspace);
}

static size_t integration_fixed_workspace_memsize(const void *data) {
  return data == NULL ? 0 : sizeof(integration_fixed_workspace);
}

static const rb_data_type_t integration_fixed_workspace_type = {
    .wrap_struct_name = "Numo::GSL::Integration::FixedWorkspace",
    .function =
        {
            .dmark = NULL,
            .dfree = integration_fixed_workspace_free,
            .dsize = integration_fixed_workspace_memsize,
        },
    .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

static VALUE integration_fixed_workspace_allocate(VALUE klass) {
  integration_fixed_workspace *workspace;
  VALUE object =
      TypedData_Make_Struct(klass, integration_fixed_workspace,
                            &integration_fixed_workspace_type, workspace);

  workspace->workspace = NULL;
  workspace->type_info = NULL;
  workspace->order = 0;
  workspace->a = 0.0;
  workspace->b = 0.0;
  workspace->alpha = 0.0;
  workspace->beta = 0.0;
  workspace->busy = 0;
  return object;
}

static VALUE integration_fixed_workspace_initialize(int argc, VALUE *argv,
                                                    VALUE self) {
  VALUE type;
  VALUE options;
  ID keywords[] = {integration_fixed_id_order, integration_fixed_id_a,
                   integration_fixed_id_b, integration_fixed_id_alpha,
                   integration_fixed_id_beta};
  VALUE keyword_values[5];
  integration_fixed_config config;
  gsl_integration_fixed_workspace *native_workspace;
  integration_fixed_workspace *workspace;

  rb_scan_args(argc, argv, "1:", &type, &options);
  if (NIL_P(options)) {
    options = rb_hash_new();
  }
  rb_get_kwargs(options, keywords, 0, 5, keyword_values);
  integration_fixed_config_parse(type, keyword_values[0], keyword_values[1],
                                 keyword_values[2], keyword_values[3],
                                 keyword_values[4], &config);

  TypedData_Get_Struct(self, integration_fixed_workspace,
                       &integration_fixed_workspace_type, workspace);
  if (workspace->workspace != NULL) {
    rb_raise(rb_eRuntimeError, "workspace is already initialized");
  }

  native_workspace = gsl_integration_fixed_alloc(
      *config.type_info->type, config.order, config.a, config.b, config.alpha,
      config.beta);
  if (native_workspace == NULL) {
    rb_memerror();
  }

  workspace->workspace = native_workspace;
  workspace->type_info = config.type_info;
  workspace->order = config.order;
  workspace->a = config.a;
  workspace->b = config.b;
  workspace->alpha = config.alpha;
  workspace->beta = config.beta;
  return self;
}

static VALUE integration_fixed_workspace_type_value(VALUE self) {
  integration_fixed_workspace *workspace;

  TypedData_Get_Struct(self, integration_fixed_workspace,
                       &integration_fixed_workspace_type, workspace);
  return ID2SYM(workspace->type_info->id);
}

static VALUE integration_fixed_workspace_order(VALUE self) {
  integration_fixed_workspace *workspace;

  TypedData_Get_Struct(self, integration_fixed_workspace,
                       &integration_fixed_workspace_type, workspace);
  return SIZET2NUM(workspace->order);
}

static VALUE integration_fixed_workspace_a(VALUE self) {
  integration_fixed_workspace *workspace;

  TypedData_Get_Struct(self, integration_fixed_workspace,
                       &integration_fixed_workspace_type, workspace);
  return DBL2NUM(workspace->a);
}

static VALUE integration_fixed_workspace_b(VALUE self) {
  integration_fixed_workspace *workspace;

  TypedData_Get_Struct(self, integration_fixed_workspace,
                       &integration_fixed_workspace_type, workspace);
  return DBL2NUM(workspace->b);
}

static VALUE integration_fixed_workspace_alpha(VALUE self) {
  integration_fixed_workspace *workspace;

  TypedData_Get_Struct(self, integration_fixed_workspace,
                       &integration_fixed_workspace_type, workspace);
  if ((workspace->type_info->requirements & INTEGRATION_FIXED_REQUIRES_ALPHA) ==
      0) {
    return Qnil;
  }
  return DBL2NUM(workspace->alpha);
}

static VALUE integration_fixed_workspace_beta(VALUE self) {
  integration_fixed_workspace *workspace;

  TypedData_Get_Struct(self, integration_fixed_workspace,
                       &integration_fixed_workspace_type, workspace);
  if ((workspace->type_info->requirements & INTEGRATION_FIXED_REQUIRES_BETA) ==
      0) {
    return Qnil;
  }
  return DBL2NUM(workspace->beta);
}

static VALUE integration_fixed_workspace_closed_p(VALUE self) {
  integration_fixed_workspace *workspace;

  TypedData_Get_Struct(self, integration_fixed_workspace,
                       &integration_fixed_workspace_type, workspace);
  return workspace->workspace == NULL ? Qtrue : Qfalse;
}

static VALUE integration_fixed_workspace_close(VALUE self) {
  integration_fixed_workspace *workspace;

  TypedData_Get_Struct(self, integration_fixed_workspace,
                       &integration_fixed_workspace_type, workspace);
  if (workspace->busy) {
    rb_raise(rb_eRuntimeError, "workspace is already in use");
  }
  if (workspace->workspace != NULL) {
    gsl_integration_fixed_free(workspace->workspace);
    workspace->workspace = NULL;
  }
  return Qnil;
}

static VALUE integration_fixed_workspace_yield(VALUE workspace) {
  return rb_yield(workspace);
}

static VALUE integration_fixed_workspace_open(int argc, VALUE *argv,
                                              VALUE klass) {
  VALUE workspace =
      rb_funcallv_kw(klass, rb_intern("new"), argc, argv, RB_PASS_KEYWORDS);
  return rb_ensure(integration_fixed_workspace_yield, workspace,
                   integration_fixed_workspace_close, workspace);
}

static integration_fixed_workspace *
integration_fixed_workspace_get(VALUE workspace_value) {
  integration_fixed_workspace *workspace;

  if (!rb_obj_is_kind_of(workspace_value, integration_cFixedWorkspace)) {
    rb_raise(rb_eTypeError,
             "workspace must be a Numo::GSL::Integration::FixedWorkspace");
  }
  TypedData_Get_Struct(workspace_value, integration_fixed_workspace,
                       &integration_fixed_workspace_type, workspace);
  if (workspace->workspace == NULL) {
    rb_raise(rb_eRuntimeError, "workspace is closed");
  }
  if (workspace->busy) {
    rb_raise(rb_eRuntimeError, "workspace is already in use");
  }
  return workspace;
}

static void
integration_fixed_workspace_use_begin(VALUE workspace_value,
                                      const integration_fixed_config *config,
                                      integration_fixed_workspace_use *use) {
  integration_fixed_workspace *workspace;

  use->workspace = NULL;
  use->owner = NULL;

  if (workspace_value == Qundef) {
    use->workspace = gsl_integration_fixed_alloc(
        *config->type_info->type, config->order, config->a, config->b,
        config->alpha, config->beta);
    if (use->workspace == NULL) {
      rb_memerror();
    }
    return;
  }

  workspace = integration_fixed_workspace_get(workspace_value);
  workspace->busy = 1;
  use->workspace = workspace->workspace;
  use->owner = workspace;
}

static VALUE integration_fixed_workspace_use_end(VALUE data) {
  integration_fixed_workspace_use *use =
      (integration_fixed_workspace_use *)data;

  if (use->owner != NULL) {
    use->owner->busy = 0;
  } else if (use->workspace != NULL) {
    gsl_integration_fixed_free(use->workspace);
  }
  use->workspace = NULL;
  use->owner = NULL;
  return Qnil;
}

static VALUE integration_run_fixed(VALUE data) {
  integration_fixed_params *params = (integration_fixed_params *)data;

  (void)gsl_integration_fixed(params->function, params->result,
                              params->workspace);
  return Qnil;
}

VALUE integration_fixed(int argc, VALUE *argv, VALUE self) {
  VALUE type = Qundef;
  VALUE options;
  VALUE workspace_value = Qundef;
  ID keywords[] = {integration_fixed_id_order, integration_fixed_id_a,
                   integration_fixed_id_b,     integration_fixed_id_alpha,
                   integration_fixed_id_beta,  integration_fixed_id_workspace};
  VALUE keyword_values[6];
  integration_fixed_config config = {0};
  int positional_count;
  int config_given;
  gsl_function function;
  integration_callback_params params;
  integration_fixed_params fixed_params;
  integration_fixed_workspace_use workspace_use;
  double result;
  int state = 0;

  (void)self;
  positional_count = rb_scan_args(argc, argv, "01:", &type, &options);
  if (NIL_P(options)) {
    options = rb_hash_new();
  }
  rb_get_kwargs(options, keywords, 0, 6, keyword_values);
  workspace_value = keyword_values[5];
  config_given = positional_count == 1;

  if (!config_given && workspace_value == Qundef) {
    rb_raise(rb_eArgError, "wrong number of arguments (given 0, expected 1)");
  }
  if (config_given && workspace_value != Qundef) {
    rb_raise(rb_eArgError,
             "workspace cannot be specified with fixed quadrature parameters");
  }
  if (!config_given &&
      (keyword_values[0] != Qundef || keyword_values[1] != Qundef ||
       keyword_values[2] != Qundef || keyword_values[3] != Qundef ||
       keyword_values[4] != Qundef)) {
    rb_raise(rb_eArgError,
             "type is required when fixed quadrature parameters are specified");
  }
  if (config_given) {
    integration_fixed_config_parse(type, keyword_values[0], keyword_values[1],
                                   keyword_values[2], keyword_values[3],
                                   keyword_values[4], &config);
  }

  integration_function_init(&function, &params);
  integration_fixed_workspace_use_begin(workspace_value, &config,
                                        &workspace_use);
  fixed_params.function = &function;
  fixed_params.workspace = workspace_use.workspace;
  fixed_params.result = &result;
  rb_protect(integration_run_fixed, (VALUE)&fixed_params, &state);
  integration_fixed_workspace_use_end((VALUE)&workspace_use);
  RB_GC_GUARD(workspace_value);
  RB_GC_GUARD(params.block);
  RB_GC_GUARD(params.exception);
  integration_finish_callback(&params, state);
  return DBL2NUM(result);
}

void integration_fixed_init(VALUE integration_module) {
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
  integration_fixed_id_workspace = rb_intern("workspace");
  integration_fixed_id_less_than_or_equal = rb_intern("<=");

  integration_cFixedWorkspace =
      rb_define_class_under(integration_module, "FixedWorkspace", rb_cObject);
  rb_define_alloc_func(integration_cFixedWorkspace,
                       integration_fixed_workspace_allocate);
  rb_define_method(integration_cFixedWorkspace, "initialize",
                   integration_fixed_workspace_initialize, -1);
  rb_define_method(integration_cFixedWorkspace, "type",
                   integration_fixed_workspace_type_value, 0);
  rb_define_method(integration_cFixedWorkspace, "order",
                   integration_fixed_workspace_order, 0);
  rb_define_method(integration_cFixedWorkspace, "a",
                   integration_fixed_workspace_a, 0);
  rb_define_method(integration_cFixedWorkspace, "b",
                   integration_fixed_workspace_b, 0);
  rb_define_method(integration_cFixedWorkspace, "alpha",
                   integration_fixed_workspace_alpha, 0);
  rb_define_method(integration_cFixedWorkspace, "beta",
                   integration_fixed_workspace_beta, 0);
  rb_define_method(integration_cFixedWorkspace, "closed?",
                   integration_fixed_workspace_closed_p, 0);
  rb_define_method(integration_cFixedWorkspace, "close",
                   integration_fixed_workspace_close, 0);
  rb_define_singleton_method(integration_cFixedWorkspace, "open",
                             integration_fixed_workspace_open, -1);
}
