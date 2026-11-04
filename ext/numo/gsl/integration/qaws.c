#include "integration_internal.h"

typedef struct {
  gsl_function *function;
  double a;
  double b;
  double alpha;
  double beta;
  int mu;
  int nu;
  double epsabs;
  double epsrel;
  size_t limit;
  gsl_integration_workspace *workspace;
  gsl_integration_qaws_table *table;
  double *result;
  double *error;
} integration_qaws_params;

static VALUE integration_free_qaws_resources(VALUE data) {
  integration_qaws_params *params = (integration_qaws_params *)data;

  if (params->table != NULL) {
    gsl_integration_qaws_table_free(params->table);
  }
  if (params->workspace != NULL) {
    gsl_integration_workspace_free(params->workspace);
  }

  return Qnil;
}

static VALUE integration_run_qaws(VALUE data) {
  integration_qaws_params *params = (integration_qaws_params *)data;

  params->table = gsl_integration_qaws_table_alloc(params->alpha, params->beta,
                                                   params->mu, params->nu);
  params->workspace = gsl_integration_workspace_alloc(params->limit);
  (void)gsl_integration_qaws(params->function, params->a, params->b,
                             params->table, params->epsabs, params->epsrel,
                             params->limit, params->workspace, params->result,
                             params->error);

  return Qnil;
}

static VALUE integration_run_qaws_with_resources(VALUE data) {
  return rb_ensure(integration_run_qaws, data, integration_free_qaws_resources,
                   data);
}

VALUE integration_qaws(int argc, VALUE *argv, VALUE self) {
  VALUE a;
  VALUE b;
  VALUE alpha;
  VALUE beta;
  VALUE mu;
  VALUE nu;
  VALUE options;
  integration_options parsed_options;
  double epsabs;
  double epsrel;
  size_t limit;
  gsl_function function;
  integration_callback_params params;
  integration_qaws_params qaws_params;
  double result;
  double error;
  int state = 0;

  (void)self;
  rb_scan_args(argc, argv, "6:", &a, &b, &alpha, &beta, &mu, &nu, &options);

  integration_options_parse(options, &parsed_options, 0.0, 1e-8, 1000);
  epsabs = parsed_options.epsabs;
  epsrel = parsed_options.epsrel;
  limit = parsed_options.limit;

  integration_function_init(&function, &params);

  qaws_params.function = &function;
  qaws_params.a = NUM2DBL(a);
  qaws_params.b = NUM2DBL(b);
  qaws_params.alpha = NUM2DBL(alpha);
  qaws_params.beta = NUM2DBL(beta);
  qaws_params.mu = NUM2INT(mu);
  qaws_params.nu = NUM2INT(nu);
  qaws_params.epsabs = epsabs;
  qaws_params.epsrel = epsrel;
  qaws_params.limit = limit;
  qaws_params.workspace = NULL;
  qaws_params.table = NULL;
  qaws_params.result = &result;
  qaws_params.error = &error;

  rb_protect(integration_run_qaws_with_resources, (VALUE)&qaws_params, &state);
  RB_GC_GUARD(params.block);
  RB_GC_GUARD(params.exception);
  integration_finish_callback(&params, state);

  return rb_ary_new_from_args(2, DBL2NUM(result), DBL2NUM(error));
}
