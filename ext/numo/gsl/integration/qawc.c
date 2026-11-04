#include "integration_internal.h"

typedef struct {
  gsl_function *function;
  double a;
  double b;
  double c;
  double epsabs;
  double epsrel;
  size_t limit;
  gsl_integration_workspace *workspace;
  double *result;
  double *error;
} integration_qawc_params;

static VALUE integration_run_qawc(VALUE data) {
  integration_qawc_params *params = (integration_qawc_params *)data;
  (void)gsl_integration_qawc(params->function, params->a, params->b, params->c,
                             params->epsabs, params->epsrel, params->limit,
                             params->workspace, params->result, params->error);
  return Qnil;
}

static VALUE integration_run_qawc_with_workspace(VALUE data) {
  integration_qawc_params *params = (integration_qawc_params *)data;
  return rb_ensure(integration_run_qawc, data, integration_free_workspace,
                   (VALUE)params->workspace);
}

VALUE integration_qawc(int argc, VALUE *argv, VALUE self) {
  VALUE a;
  VALUE b;
  VALUE c;
  VALUE options;
  integration_options parsed_options;
  double epsabs;
  double epsrel;
  size_t limit;
  gsl_integration_workspace *workspace;
  gsl_function function;
  integration_callback_params params;
  integration_qawc_params qawc_params;
  double result;
  double error;
  int state = 0;
  (void)self;

  rb_scan_args(argc, argv, "3:", &a, &b, &c, &options);

  integration_options_parse(options, &parsed_options, 0.0, 1e-8, 1000);
  epsabs = parsed_options.epsabs;
  epsrel = parsed_options.epsrel;
  limit = parsed_options.limit;

  integration_function_init(&function, &params);

  workspace = gsl_integration_workspace_alloc(limit);
  qawc_params.function = &function;
  qawc_params.a = NUM2DBL(a);
  qawc_params.b = NUM2DBL(b);
  qawc_params.c = NUM2DBL(c);
  qawc_params.epsabs = epsabs;
  qawc_params.epsrel = epsrel;
  qawc_params.limit = limit;
  qawc_params.workspace = workspace;
  qawc_params.result = &result;
  qawc_params.error = &error;

  rb_protect(integration_run_qawc_with_workspace, (VALUE)&qawc_params, &state);
  RB_GC_GUARD(params.block);
  RB_GC_GUARD(params.exception);
  integration_finish_callback(&params, state);

  return rb_ary_new_from_args(2, DBL2NUM(result), DBL2NUM(error));
}
