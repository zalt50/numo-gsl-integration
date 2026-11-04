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

static VALUE integration_run_qagiu_with_workspace(VALUE data) {
  integration_qagiu_params *params = (integration_qagiu_params *)data;
  return rb_ensure(integration_run_qagiu, data, integration_free_workspace,
                   (VALUE)params->workspace);
}

VALUE integration_qagiu(int argc, VALUE *argv, VALUE self) {
  VALUE a;
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
  gsl_integration_workspace *workspace;
  gsl_function function;
  integration_callback_params params;
  integration_qagiu_params qagiu_params;
  double result;
  double error;
  int state = 0;
  (void)self;

  rb_scan_args(argc, argv, "1:", &a, &options);
  if (!NIL_P(options)) {
    rb_get_kwargs(options, keywords, 0, 3, keyword_values);
    epsabs_value = keyword_values[0];
    epsrel_value = keyword_values[1];
    limit_value = keyword_values[2];
  }

  epsabs = epsabs_value == Qundef ? 0.0 : NUM2DBL(epsabs_value);
  epsrel = epsrel_value == Qundef ? 1e-8 : NUM2DBL(epsrel_value);
  limit = limit_value == Qundef ? 1000 : NUM2SIZET(limit_value);

  integration_function_init(&function, &params);

  workspace = gsl_integration_workspace_alloc(limit);
  qagiu_params.function = &function;
  qagiu_params.a = NUM2DBL(a);
  qagiu_params.epsabs = epsabs;
  qagiu_params.epsrel = epsrel;
  qagiu_params.limit = limit;
  qagiu_params.workspace = workspace;
  qagiu_params.result = &result;
  qagiu_params.error = &error;

  rb_protect(integration_run_qagiu_with_workspace, (VALUE)&qagiu_params,
             &state);
  RB_GC_GUARD(params.block);
  RB_GC_GUARD(params.exception);
  integration_finish_callback(&params, state);

  return rb_ary_new_from_args(2, DBL2NUM(result), DBL2NUM(error));
}
