#include "integration_internal.h"

typedef struct {
  gsl_function *function;
  double a;
  double b;
  double epsabs;
  double epsrel;
  size_t limit;
  gsl_integration_workspace *workspace;
  double *result;
  double *error;
} integration_qags_params;

static VALUE integration_run_qags(VALUE data) {
  integration_qags_params *params = (integration_qags_params *)data;

  (void)gsl_integration_qags(params->function, params->a, params->b,
                             params->epsabs, params->epsrel, params->limit,
                             params->workspace, params->result, params->error);

  return Qnil;
}

static VALUE integration_run_qags_with_workspace(VALUE data) {
  integration_qags_params *params = (integration_qags_params *)data;

  return rb_ensure(integration_run_qags, data, integration_free_workspace,
                   (VALUE)params->workspace);
}

VALUE integration_qags(int argc, VALUE *argv, VALUE self) {
  VALUE a;
  VALUE b;
  VALUE options;
  integration_options parsed_options;
  double epsabs;
  double epsrel;
  size_t limit;
  gsl_integration_workspace *workspace;
  gsl_function function;
  integration_callback_params params;
  integration_qags_params qags_params;
  double result;
  double error;
  int state = 0;

  (void)self;

  rb_scan_args(argc, argv, "2:", &a, &b, &options);

  integration_options_parse(options, &parsed_options, 0.0, 1e-8, 1000);
  epsabs = parsed_options.epsabs;
  epsrel = parsed_options.epsrel;
  limit = parsed_options.limit;

  integration_function_init(&function, &params);

  workspace = gsl_integration_workspace_alloc(limit);

  qags_params.function = &function;
  qags_params.a = NUM2DBL(a);
  qags_params.b = NUM2DBL(b);
  qags_params.epsabs = epsabs;
  qags_params.epsrel = epsrel;
  qags_params.limit = limit;
  qags_params.workspace = workspace;
  qags_params.result = &result;
  qags_params.error = &error;

  rb_protect(integration_run_qags_with_workspace, (VALUE)&qags_params, &state);

  RB_GC_GUARD(params.block);
  RB_GC_GUARD(params.exception);

  integration_finish_callback(&params, state);

  return rb_ary_new_from_args(2, DBL2NUM(result), DBL2NUM(error));
}
