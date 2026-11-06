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

VALUE integration_qags(int argc, VALUE *argv, VALUE self) {
  VALUE a;
  VALUE b;
  VALUE options;
  VALUE workspace_value;
  integration_options parsed_options;
  double a_double;
  double b_double;
  double epsabs;
  double epsrel;
  size_t limit;
  int limit_given;
  gsl_function function;
  integration_callback_params params;
  integration_qags_params qags_params;
  integration_workspace_use workspace_use;
  double result;
  double error;
  int state = 0;

  (void)self;

  rb_scan_args(argc, argv, "2:", &a, &b, &options);
  integration_options_parse_with_workspace(options, &parsed_options, 0.0, 1e-8,
                                           1000, &workspace_value,
                                           &limit_given);

  a_double = NUM2DBL(a);
  b_double = NUM2DBL(b);
  epsabs = parsed_options.epsabs;
  epsrel = parsed_options.epsrel;
  limit = parsed_options.limit;

  integration_function_init(&function, &params);

  integration_workspace_use_begin(workspace_value, &limit, limit_given,
                                  &workspace_use);

  qags_params.function = &function;
  qags_params.a = a_double;
  qags_params.b = b_double;
  qags_params.epsabs = epsabs;
  qags_params.epsrel = epsrel;
  qags_params.limit = limit;
  qags_params.workspace = workspace_use.workspace;
  qags_params.result = &result;
  qags_params.error = &error;

  rb_protect(integration_run_qags, (VALUE)&qags_params, &state);
  integration_workspace_use_end((VALUE)&workspace_use);

  RB_GC_GUARD(workspace_value);
  RB_GC_GUARD(params.block);
  RB_GC_GUARD(params.exception);

  integration_finish_callback(&params, state);

  return rb_ary_new_from_args(2, DBL2NUM(result), DBL2NUM(error));
}
