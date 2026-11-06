#include "integration_internal.h"

typedef struct {
  gsl_function *function;
  double a;
  double b;
  double epsabs;
  double epsrel;
  size_t limit;
  int key;
  gsl_integration_workspace *workspace;
  double *result;
  double *error;
} integration_qag_params;

static VALUE integration_run_qag(VALUE data) {
  integration_qag_params *params = (integration_qag_params *)data;

  (void)gsl_integration_qag(params->function, params->a, params->b,
                            params->epsabs, params->epsrel, params->limit,
                            params->key, params->workspace, params->result,
                            params->error);

  return Qnil;
}

VALUE integration_qag(int argc, VALUE *argv, VALUE self) {
  VALUE a;
  VALUE b;
  VALUE options;
  VALUE epsabs_value = Qundef;
  VALUE epsrel_value = Qundef;
  VALUE limit_value = Qundef;
  VALUE key_value = Qundef;
  VALUE workspace_value = Qundef;
  ID keywords[] = {rb_intern("epsabs"), rb_intern("epsrel"), rb_intern("limit"),
                   rb_intern("key"), rb_intern("workspace")};
  VALUE keyword_values[5];
  double a_double;
  double b_double;
  double epsabs;
  double epsrel;
  size_t limit;
  int limit_given;
  int key;
  gsl_function function;
  integration_callback_params params;
  integration_qag_params qag_params;
  integration_workspace_use workspace_use;
  double result;
  double error;
  int state = 0;

  (void)self;

  rb_scan_args(argc, argv, "2:", &a, &b, &options);

  if (!NIL_P(options)) {
    rb_get_kwargs(options, keywords, 0, 5, keyword_values);
    epsabs_value = keyword_values[0];
    epsrel_value = keyword_values[1];
    limit_value = keyword_values[2];
    key_value = keyword_values[3];
    workspace_value = keyword_values[4];
  }

  a_double = NUM2DBL(a);
  b_double = NUM2DBL(b);
  epsabs = epsabs_value == Qundef ? 0.0 : NUM2DBL(epsabs_value);
  epsrel = epsrel_value == Qundef ? 1e-8 : NUM2DBL(epsrel_value);
  limit = limit_value == Qundef ? 1000 : NUM2SIZET(limit_value);
  limit_given = limit_value != Qundef;
  key = key_value == Qundef ? GSL_INTEG_GAUSS61 : NUM2INT(key_value);

  integration_function_init(&function, &params);

  integration_workspace_use_begin(workspace_value, &limit, limit_given,
                                  &workspace_use);

  qag_params.function = &function;
  qag_params.a = a_double;
  qag_params.b = b_double;
  qag_params.epsabs = epsabs;
  qag_params.epsrel = epsrel;
  qag_params.limit = limit;
  qag_params.key = key;
  qag_params.workspace = workspace_use.workspace;
  qag_params.result = &result;
  qag_params.error = &error;

  rb_protect(integration_run_qag, (VALUE)&qag_params, &state);
  integration_workspace_use_end((VALUE)&workspace_use);

  RB_GC_GUARD(workspace_value);
  RB_GC_GUARD(params.block);
  RB_GC_GUARD(params.exception);

  integration_finish_callback(&params, state);

  return rb_ary_new_from_args(2, DBL2NUM(result), DBL2NUM(error));
}
