#include "integration_internal.h"

typedef struct {
  gsl_function *function;
  double a;
  double omega;
  enum gsl_integration_qawo_enum sine;
  double epsabs;
  size_t limit;
  gsl_integration_workspace *workspace;
  gsl_integration_workspace *cycle_workspace;
  gsl_integration_qawo_table *table;
  double *result;
  double *error;
} integration_qawf_params;

static VALUE integration_free_qawf_resources(VALUE data) {
  integration_qawf_params *params = (integration_qawf_params *)data;

  if (params->table != NULL) {
    gsl_integration_qawo_table_free(params->table);
  }
  if (params->cycle_workspace != NULL) {
    gsl_integration_workspace_free(params->cycle_workspace);
  }
  if (params->workspace != NULL) {
    gsl_integration_workspace_free(params->workspace);
  }

  return Qnil;
}

static VALUE integration_run_qawf(VALUE data) {
  integration_qawf_params *params = (integration_qawf_params *)data;

  params->table = gsl_integration_qawo_table_alloc(params->omega, 1.0,
                                                   params->sine, params->limit);
  params->workspace = gsl_integration_workspace_alloc(params->limit);
  params->cycle_workspace = gsl_integration_workspace_alloc(params->limit);

  (void)gsl_integration_qawf(params->function, params->a, params->epsabs,
                             params->limit, params->workspace,
                             params->cycle_workspace, params->table,
                             params->result, params->error);

  return Qnil;
}

static VALUE integration_run_qawf_with_resources(VALUE data) {
  return rb_ensure(integration_run_qawf, data, integration_free_qawf_resources,
                   data);
}

VALUE integration_qawf(int argc, VALUE *argv, VALUE self) {
  VALUE a;
  VALUE omega;
  VALUE sine;
  VALUE options;
  VALUE epsabs_value = Qundef;
  VALUE limit_value = Qundef;
  ID keywords[] = {rb_intern("epsabs"), rb_intern("limit")};
  VALUE keyword_values[2];
  double epsabs;
  size_t limit;
  gsl_function function;
  integration_callback_params params;
  integration_qawf_params qawf_params;
  double result;
  double error;
  int state = 0;

  (void)self;

  rb_scan_args(argc, argv, "3:", &a, &omega, &sine, &options);

  if (!NIL_P(options)) {
    rb_get_kwargs(options, keywords, 0, 2, keyword_values);
    epsabs_value = keyword_values[0];
    limit_value = keyword_values[1];
  }

  epsabs = epsabs_value == Qundef ? 1e-10 : NUM2DBL(epsabs_value);
  limit = limit_value == Qundef ? 1000 : NUM2SIZET(limit_value);

  integration_function_init(&function, &params);

  qawf_params.function = &function;
  qawf_params.a = NUM2DBL(a);
  qawf_params.omega = NUM2DBL(omega);
  qawf_params.sine = (enum gsl_integration_qawo_enum)NUM2INT(sine);
  qawf_params.epsabs = epsabs;
  qawf_params.limit = limit;
  qawf_params.workspace = NULL;
  qawf_params.cycle_workspace = NULL;
  qawf_params.table = NULL;
  qawf_params.result = &result;
  qawf_params.error = &error;

  rb_protect(integration_run_qawf_with_resources, (VALUE)&qawf_params, &state);

  RB_GC_GUARD(params.block);
  RB_GC_GUARD(params.exception);

  integration_finish_callback(&params, state);

  return rb_ary_new_from_args(2, DBL2NUM(result), DBL2NUM(error));
}
