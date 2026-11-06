#include "integration_internal.h"

typedef struct {
  gsl_function *function;
  double a;
  double epsabs;
  double epsrel;
  size_t limit;
  gsl_integration_workspace *workspace;
  double *result;
  double *error;
} integration_qagiu_params;

static VALUE integration_run_qagiu(VALUE data) {
  integration_qagiu_params *params = (integration_qagiu_params *)data;

  (void)gsl_integration_qagiu(params->function, params->a, params->epsabs,
                              params->epsrel, params->limit, params->workspace,
                              params->result, params->error);

  return Qnil;
}

VALUE integration_qagiu(int argc, VALUE *argv, VALUE self) {
  VALUE a;
  VALUE options;
  VALUE workspace_value;
  integration_options parsed_options;
  double a_double;
  double epsabs;
  double epsrel;
  size_t limit;
  int limit_given;
  gsl_function function;
  integration_callback_params params;
  integration_qagiu_params qagiu_params;
  integration_workspace_use workspace_use;
  double result;
  double error;
  int state = 0;

  (void)self;

  rb_scan_args(argc, argv, "1:", &a, &options);
  integration_options_parse_with_workspace(options, &parsed_options, 0.0, 1e-8,
                                           1000, &workspace_value,
                                           &limit_given);

  a_double = NUM2DBL(a);
  epsabs = parsed_options.epsabs;
  epsrel = parsed_options.epsrel;
  limit = parsed_options.limit;

  integration_function_init(&function, &params);

  integration_workspace_use_begin(workspace_value, &limit, limit_given,
                                  &workspace_use);

  qagiu_params.function = &function;
  qagiu_params.a = a_double;
  qagiu_params.epsabs = epsabs;
  qagiu_params.epsrel = epsrel;
  qagiu_params.limit = limit;
  qagiu_params.workspace = workspace_use.workspace;
  qagiu_params.result = &result;
  qagiu_params.error = &error;

  rb_protect(integration_run_qagiu, (VALUE)&qagiu_params, &state);
  integration_workspace_use_end((VALUE)&workspace_use);

  RB_GC_GUARD(workspace_value);
  RB_GC_GUARD(params.block);
  RB_GC_GUARD(params.exception);

  integration_finish_callback(&params, state);

  return rb_ary_new_from_args(2, DBL2NUM(result), DBL2NUM(error));
}
