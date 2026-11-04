#include "integration_internal.h"

typedef struct {
  gsl_function *function;
  double a;
  double b;
  double epsabs;
  double epsrel;
  size_t limit;
  gsl_integration_cquad_workspace *workspace;
  double *result;
  double *error;
  size_t *evaluations;
} integration_cquad_params;

static VALUE integration_free_cquad_workspace(VALUE data) {
  gsl_integration_cquad_workspace *workspace =
      (gsl_integration_cquad_workspace *)data;

  gsl_integration_cquad_workspace_free(workspace);

  return Qnil;
}

static VALUE integration_run_cquad(VALUE data) {
  integration_cquad_params *params = (integration_cquad_params *)data;

  (void)gsl_integration_cquad(
      params->function, params->a, params->b, params->epsabs, params->epsrel,
      params->workspace, params->result, params->error, params->evaluations);

  return Qnil;
}

static VALUE integration_run_cquad_with_workspace(VALUE data) {
  integration_cquad_params *params = (integration_cquad_params *)data;

  return rb_ensure(integration_run_cquad, data,
                   integration_free_cquad_workspace, (VALUE)params->workspace);
}

VALUE integration_cquad(int argc, VALUE *argv, VALUE self) {
  VALUE a;
  VALUE b;
  VALUE options;
  integration_options parsed_options;
  double epsabs;
  double epsrel;
  size_t limit;
  gsl_integration_cquad_workspace *workspace;
  gsl_function function;
  integration_callback_params params;
  integration_cquad_params cquad_params;
  double result;
  double error;
  size_t evaluations;
  int state = 0;

  (void)self;

  rb_scan_args(argc, argv, "2:", &a, &b, &options);

  integration_options_parse(options, &parsed_options, 0.0, 1e-8, 100);
  epsabs = parsed_options.epsabs;
  epsrel = parsed_options.epsrel;
  limit = parsed_options.limit;

  integration_function_init(&function, &params);

  workspace = gsl_integration_cquad_workspace_alloc(limit);

  cquad_params.function = &function;
  cquad_params.a = NUM2DBL(a);
  cquad_params.b = NUM2DBL(b);
  cquad_params.epsabs = epsabs;
  cquad_params.epsrel = epsrel;
  cquad_params.limit = limit;
  cquad_params.workspace = workspace;
  cquad_params.result = &result;
  cquad_params.error = &error;
  cquad_params.evaluations = &evaluations;

  rb_protect(integration_run_cquad_with_workspace, (VALUE)&cquad_params,
             &state);

  RB_GC_GUARD(params.block);
  RB_GC_GUARD(params.exception);

  integration_finish_callback(&params, state);

  return rb_ary_new_from_args(3, DBL2NUM(result), DBL2NUM(error),
                              SIZET2NUM(evaluations));
}
