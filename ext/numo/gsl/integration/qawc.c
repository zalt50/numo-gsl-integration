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

VALUE integration_qawc(int argc, VALUE *argv, VALUE self) {
  VALUE a;
  VALUE b;
  VALUE c;
  VALUE options;
  VALUE workspace_value;
  integration_options parsed_options;
  double a_double;
  double b_double;
  double c_double;
  double epsabs;
  double epsrel;
  size_t limit;
  int limit_given;
  gsl_function function;
  integration_callback_params params;
  integration_qawc_params qawc_params;
  integration_workspace_use workspace_use;
  double result;
  double error;
  int state = 0;

  (void)self;

  rb_scan_args(argc, argv, "3:", &a, &b, &c, &options);
  integration_options_parse_with_workspace(options, &parsed_options, 0.0, 1e-8,
                                           1000, &workspace_value,
                                           &limit_given);

  a_double = NUM2DBL(a);
  b_double = NUM2DBL(b);
  c_double = NUM2DBL(c);
  epsabs = parsed_options.epsabs;
  epsrel = parsed_options.epsrel;
  limit = parsed_options.limit;

  integration_function_init(&function, &params);

  integration_workspace_use_begin(workspace_value, &limit, limit_given,
                                  &workspace_use);

  qawc_params.function = &function;
  qawc_params.a = a_double;
  qawc_params.b = b_double;
  qawc_params.c = c_double;
  qawc_params.epsabs = epsabs;
  qawc_params.epsrel = epsrel;
  qawc_params.limit = limit;
  qawc_params.workspace = workspace_use.workspace;
  qawc_params.result = &result;
  qawc_params.error = &error;

  rb_protect(integration_run_qawc, (VALUE)&qawc_params, &state);
  integration_workspace_use_end((VALUE)&workspace_use);

  RB_GC_GUARD(workspace_value);
  RB_GC_GUARD(params.block);
  RB_GC_GUARD(params.exception);

  integration_finish_callback(&params, state);

  return rb_ary_new_from_args(2, DBL2NUM(result), DBL2NUM(error));
}
