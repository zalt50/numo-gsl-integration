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

static VALUE integration_run_qagil_with_workspace(VALUE data) {
  integration_qagil_params *params = (integration_qagil_params *)data;
  return rb_ensure(integration_run_qagil, data, integration_free_workspace,
                   (VALUE)params->workspace);
}

VALUE integration_qagil(int argc, VALUE *argv, VALUE self) {
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
  gsl_integration_workspace *workspace;
  gsl_function function;
  integration_callback_params params;
  integration_qagil_params qagil_params;
  double result;
  double error;
  int state = 0;
  (void)self;

  rb_scan_args(argc, argv, "1:", &b, &options);
  if (!NIL_P(options)) {
    rb_get_kwargs(options, keywords, 0, 3, keyword_values);
    epsabs_value = keyword_values[0];
    epsrel_value = keyword_values[1];
    limit_value = keyword_values[2];
  }

  epsabs = epsabs_value == Qundef ? 0.0 : NUM2DBL(epsabs_value);
  epsrel = epsrel_value == Qundef ? 1e-8 : NUM2DBL(epsrel_value);
  limit = limit_value == Qundef ? 1000 : NUM2SIZET(limit_value);

  integration_callback_params_init(&params);
  function.function = integration_callback;
  function.params = &params;

  workspace = gsl_integration_workspace_alloc(limit);
  qagil_params.function = &function;
  qagil_params.b = NUM2DBL(b);
  qagil_params.epsabs = epsabs;
  qagil_params.epsrel = epsrel;
  qagil_params.limit = limit;
  qagil_params.workspace = workspace;
  qagil_params.result = &result;
  qagil_params.error = &error;

  rb_protect(integration_run_qagil_with_workspace, (VALUE)&qagil_params,
             &state);
  RB_GC_GUARD(params.block);
  RB_GC_GUARD(params.exception);
  integration_raise_callback_exception(&params);
  if (state != 0) {
    rb_jump_tag(state);
  }

  return rb_ary_new_from_args(2, DBL2NUM(result), DBL2NUM(error));
}
