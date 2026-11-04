#include "integration_internal.h"

typedef struct {
  gsl_function *function;
  double a;
  double b;
  double epsabs;
  double epsrel;
  gsl_integration_romberg_workspace *workspace;
  double *result;
  size_t *evaluations;
} integration_romberg_params;

static VALUE integration_free_romberg_workspace(VALUE data) {
  gsl_integration_romberg_workspace *workspace =
      (gsl_integration_romberg_workspace *)data;

  gsl_integration_romberg_free(workspace);

  return Qnil;
}

static VALUE integration_run_romberg(VALUE data) {
  integration_romberg_params *params = (integration_romberg_params *)data;

  (void)gsl_integration_romberg(params->function, params->a, params->b,
                                params->epsabs, params->epsrel, params->result,
                                params->evaluations, params->workspace);

  return Qnil;
}

static VALUE integration_run_romberg_with_workspace(VALUE data) {
  integration_romberg_params *params = (integration_romberg_params *)data;

  return rb_ensure(integration_run_romberg, data,
                   integration_free_romberg_workspace,
                   (VALUE)params->workspace);
}

VALUE integration_romberg(int argc, VALUE *argv, VALUE self) {
  VALUE a;
  VALUE b;
  VALUE options;
  integration_options parsed_options;
  double epsabs;
  double epsrel;
  size_t limit;
  gsl_integration_romberg_workspace *workspace;
  gsl_function function;
  integration_callback_params params;
  integration_romberg_params romberg_params;
  double result;
  size_t evaluations;
  int state = 0;

  (void)self;

  rb_scan_args(argc, argv, "2:", &a, &b, &options);

  integration_options_parse(options, &parsed_options, 0.0, 1e-8, 20);
  epsabs = parsed_options.epsabs;
  epsrel = parsed_options.epsrel;
  limit = parsed_options.limit;

  integration_function_init(&function, &params);

  workspace = gsl_integration_romberg_alloc(limit);

  romberg_params.function = &function;
  romberg_params.a = NUM2DBL(a);
  romberg_params.b = NUM2DBL(b);
  romberg_params.epsabs = epsabs;
  romberg_params.epsrel = epsrel;
  romberg_params.workspace = workspace;
  romberg_params.result = &result;
  romberg_params.evaluations = &evaluations;

  rb_protect(integration_run_romberg_with_workspace, (VALUE)&romberg_params,
             &state);

  RB_GC_GUARD(params.block);
  RB_GC_GUARD(params.exception);

  integration_finish_callback(&params, state);

  return rb_ary_new_from_args(2, DBL2NUM(result), SIZET2NUM(evaluations));
}
