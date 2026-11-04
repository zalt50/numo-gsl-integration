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
  ID keywords[] = {rb_intern("epsabs"), rb_intern("epsrel"), rb_intern("limit"),
                   rb_intern("key")};
  VALUE keyword_values[4];
  double epsabs;
  double epsrel;
  size_t limit;
  int key;
  gsl_integration_workspace *workspace;
  gsl_function function;
  integration_callback_params params;
  integration_qag_params qag_params;
  double result;
  double error;

  (void)self;

  rb_scan_args(argc, argv, "2:", &a, &b, &options);

  if (!NIL_P(options)) {
    rb_get_kwargs(options, keywords, 0, 4, keyword_values);
    epsabs_value = keyword_values[0];
    epsrel_value = keyword_values[1];
    limit_value = keyword_values[2];
    key_value = keyword_values[3];
  }

  epsabs = epsabs_value == Qundef ? 0.0 : NUM2DBL(epsabs_value);
  epsrel = epsrel_value == Qundef ? 1e-8 : NUM2DBL(epsrel_value);
  limit = limit_value == Qundef ? 1000 : NUM2SIZET(limit_value);
  key = key_value == Qundef ? GSL_INTEG_GAUSS61 : NUM2INT(key_value);

  integration_function_init(&function, &params);

  workspace = gsl_integration_workspace_alloc(limit);

  qag_params.function = &function;
  qag_params.a = NUM2DBL(a);
  qag_params.b = NUM2DBL(b);
  qag_params.epsabs = epsabs;
  qag_params.epsrel = epsrel;
  qag_params.limit = limit;
  qag_params.key = key;
  qag_params.workspace = workspace;
  qag_params.result = &result;
  qag_params.error = &error;

  rb_ensure(integration_run_qag, (VALUE)&qag_params, integration_free_workspace,
            (VALUE)workspace);

  RB_GC_GUARD(params.block);
  RB_GC_GUARD(params.exception);

  integration_finish_callback(&params, 0);

  return rb_ary_new_from_args(2, DBL2NUM(result), DBL2NUM(error));
}
