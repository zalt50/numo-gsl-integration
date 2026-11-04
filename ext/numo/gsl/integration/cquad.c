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
  VALUE epsabs_value = Qundef;
  VALUE epsrel_value = Qundef;
  VALUE limit_value = Qundef;
  ID keywords[] = {rb_intern("epsabs"), rb_intern("epsrel"),
                   rb_intern("limit")};
  VALUE keyword_values[3];
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

  if (!NIL_P(options)) {
    rb_get_kwargs(options, keywords, 0, 3, keyword_values);
    epsabs_value = keyword_values[0];
    epsrel_value = keyword_values[1];
    limit_value = keyword_values[2];
  }

  epsabs = epsabs_value == Qundef ? 0.0 : NUM2DBL(epsabs_value);
  epsrel = epsrel_value == Qundef ? 1e-8 : NUM2DBL(epsrel_value);
  limit = limit_value == Qundef ? 100 : NUM2SIZET(limit_value);

  integration_callback_params_init(&params);
  function.function = integration_callback;
  function.params = &params;

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

  integration_raise_callback_exception(&params);

  if (state != 0) {
    rb_jump_tag(state);
  }

  return rb_ary_new_from_args(3, DBL2NUM(result), DBL2NUM(error),
                              SIZET2NUM(evaluations));
}
