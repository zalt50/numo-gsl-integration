#include "integration_internal.h"

typedef struct {
  gsl_function *function;
  double epsabs;
  double epsrel;
  size_t limit;
  gsl_integration_workspace *workspace;
  double *result;
  double *error;
} integration_qagi_params;

static VALUE integration_run_qagi(VALUE data) {
  integration_qagi_params *params = (integration_qagi_params *)data;

  (void)gsl_integration_qagi(params->function, params->epsabs, params->epsrel,
                             params->limit, params->workspace, params->result,
                             params->error);

  return Qnil;
}

VALUE integration_qagi(int argc, VALUE *argv, VALUE self) {
  VALUE options;
  VALUE workspace_value;
  integration_options parsed_options;
  double epsabs;
  double epsrel;
  size_t limit;
  int limit_given;
  gsl_function function;
  integration_callback_params params;
  integration_qagi_params qagi_params;
  integration_workspace_use workspace_use;
  double result;
  double error;
  int state = 0;

  (void)self;

  rb_scan_args(argc, argv, "0:", &options);
  integration_options_parse_with_workspace(options, &parsed_options, 0.0, 1e-8,
                                           1000, &workspace_value,
                                           &limit_given);

  epsabs = parsed_options.epsabs;
  epsrel = parsed_options.epsrel;
  limit = parsed_options.limit;

  integration_function_init(&function, &params);

  integration_workspace_use_begin(workspace_value, &limit, limit_given,
                                  &workspace_use);

  qagi_params.function = &function;
  qagi_params.epsabs = epsabs;
  qagi_params.epsrel = epsrel;
  qagi_params.limit = limit;
  qagi_params.workspace = workspace_use.workspace;
  qagi_params.result = &result;
  qagi_params.error = &error;

  rb_protect(integration_run_qagi, (VALUE)&qagi_params, &state);
  integration_workspace_use_end((VALUE)&workspace_use);

  RB_GC_GUARD(workspace_value);
  RB_GC_GUARD(params.block);
  RB_GC_GUARD(params.exception);

  integration_finish_callback(&params, state);

  return rb_ary_new_from_args(2, DBL2NUM(result), DBL2NUM(error));
}
