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
  VALUE epsabs_value = Qundef;
  VALUE epsrel_value = Qundef;
  VALUE limit_value = Qundef;
  ID keywords[] = {rb_intern("epsabs"), rb_intern("epsrel"),
                   rb_intern("limit")};
  VALUE keyword_values[3];
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

  if (!NIL_P(options)) {
    rb_get_kwargs(options, keywords, 0, 3, keyword_values);
    epsabs_value = keyword_values[0];
    epsrel_value = keyword_values[1];
    limit_value = keyword_values[2];
  }

  epsabs = epsabs_value == Qundef ? 0.0 : NUM2DBL(epsabs_value);
  epsrel = epsrel_value == Qundef ? 1e-8 : NUM2DBL(epsrel_value);
  limit = limit_value == Qundef ? 20 : NUM2SIZET(limit_value);

  integration_callback_params_init(&params);
  function.function = integration_callback;
  function.params = &params;

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

  integration_raise_callback_exception(&params);

  if (state != 0) {
    rb_jump_tag(state);
  }

  return rb_ary_new_from_args(2, DBL2NUM(result), SIZET2NUM(evaluations));
}
