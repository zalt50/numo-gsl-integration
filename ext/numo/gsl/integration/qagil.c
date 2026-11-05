#include "integration_internal.h"

typedef struct {
  gsl_function *function;
  double b;
  double epsabs;
  double epsrel;
  size_t limit;
  gsl_integration_workspace *workspace;
  double *result;
  double *error;
} integration_qagil_params;

static VALUE integration_run_qagil(VALUE data) {
  integration_qagil_params *params = (integration_qagil_params *)data;

  (void)gsl_integration_qagil(params->function, params->b, params->epsabs,
                              params->epsrel, params->limit, params->workspace,
                              params->result, params->error);

  return Qnil;
}

static VALUE integration_run_qagil_with_workspace(VALUE data) {
  integration_qagil_params *params = (integration_qagil_params *)data;

  return rb_ensure(integration_run_qagil, data, integration_free_workspace,
                   (VALUE)params->workspace);
}

VALUE integration_qagil(int argc, VALUE *argv, VALUE self) {
  VALUE b;
  VALUE options;
  integration_options parsed_options;
  double epsabs;
  double epsrel;
  size_t limit;
  gsl_integration_workspace *workspace;
  gsl_function function;
  integration_callback_params params;
  integration_qagil_params qagil_params;
  double result;
  double error;
  int state = 0;

  (void)self;

  rb_scan_args(argc, argv, "1:", &b, &options);

  integration_options_parse(options, &parsed_options, 0.0, 1e-8, 1000);

  epsabs = parsed_options.epsabs;
  epsrel = parsed_options.epsrel;
  limit = parsed_options.limit;

  integration_function_init(&function, &params);

  workspace = gsl_integration_workspace_alloc(limit);

  qagil_params.function = &function;
  qagil_params.b = NUM2DBL(b);
  qagil_params.epsabs = epsabs;
  qagil_params.epsrel = epsrel;
  qagil_params.limit = limit;
  qagil_params.workspace = workspace;
  qagil_params.result = &result;
  qagil_params.error = &error;

  rb_protect(integration_run_qagil_with_workspace, (VALUE)&qagil_params,
             &state);

  RB_GC_GUARD(params.block);
  RB_GC_GUARD(params.exception);

  integration_finish_callback(&params, state);

  return rb_ary_new_from_args(2, DBL2NUM(result), DBL2NUM(error));
}
