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

VALUE integration_qagil(int argc, VALUE *argv, VALUE self) {
  VALUE b;
  VALUE options;
  VALUE workspace_value;
  integration_options parsed_options;
  double b_double;
  double epsabs;
  double epsrel;
  size_t limit;
  int limit_given;
  gsl_function function;
  integration_callback_params params;
  integration_qagil_params qagil_params;
  integration_workspace_use workspace_use;
  double result;
  double error;
  int state = 0;

  (void)self;

  rb_scan_args(argc, argv, "1:", &b, &options);
  integration_options_parse_with_workspace(options, &parsed_options, 0.0, 1e-8,
                                           1000, &workspace_value,
                                           &limit_given);

  b_double = NUM2DBL(b);
  epsabs = parsed_options.epsabs;
  epsrel = parsed_options.epsrel;
  limit = parsed_options.limit;

  integration_function_init(&function, &params);

  integration_workspace_use_begin(workspace_value, &limit, limit_given,
                                  &workspace_use);

  qagil_params.function = &function;
  qagil_params.b = b_double;
  qagil_params.epsabs = epsabs;
  qagil_params.epsrel = epsrel;
  qagil_params.limit = limit;
  qagil_params.workspace = workspace_use.workspace;
  qagil_params.result = &result;
  qagil_params.error = &error;

  rb_protect(integration_run_qagil, (VALUE)&qagil_params, &state);
  integration_workspace_use_end((VALUE)&workspace_use);

  RB_GC_GUARD(workspace_value);
  RB_GC_GUARD(params.block);
  RB_GC_GUARD(params.exception);

  integration_finish_callback(&params, state);

  return rb_ary_new_from_args(2, DBL2NUM(result), DBL2NUM(error));
}
