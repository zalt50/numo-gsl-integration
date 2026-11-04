#include "integration_internal.h"

typedef struct {
  gsl_function *function;
  double a;
  double b;
  double epsabs;
  double epsrel;
  double *result;
  double *error;
  size_t *evaluations;
} integration_qng_params;

static VALUE integration_run_qng(VALUE data) {
  integration_qng_params *params = (integration_qng_params *)data;

  (void)gsl_integration_qng(params->function, params->a, params->b,
                            params->epsabs, params->epsrel, params->result,
                            params->error, params->evaluations);

  return Qnil;
}

VALUE integration_qng(int argc, VALUE *argv, VALUE self) {
  VALUE a;
  VALUE b;
  VALUE options;
  VALUE epsabs_value = Qundef;
  VALUE epsrel_value = Qundef;
  ID keywords[] = {rb_intern("epsabs"), rb_intern("epsrel")};
  VALUE keyword_values[2];
  double epsabs;
  double epsrel;
  gsl_function function;
  integration_callback_params params;
  integration_qng_params qng_params;
  double result;
  double error;
  size_t evaluations;
  int state = 0;

  (void)self;

  rb_scan_args(argc, argv, "2:", &a, &b, &options);

  if (!NIL_P(options)) {
    rb_get_kwargs(options, keywords, 0, 2, keyword_values);
    epsabs_value = keyword_values[0];
    epsrel_value = keyword_values[1];
  }

  epsabs = epsabs_value == Qundef ? 0.0 : NUM2DBL(epsabs_value);
  epsrel = epsrel_value == Qundef ? 1e-8 : NUM2DBL(epsrel_value);

  integration_function_init(&function, &params);

  qng_params.function = &function;
  qng_params.a = NUM2DBL(a);
  qng_params.b = NUM2DBL(b);
  qng_params.epsabs = epsabs;
  qng_params.epsrel = epsrel;
  qng_params.result = &result;
  qng_params.error = &error;
  qng_params.evaluations = &evaluations;

  rb_protect(integration_run_qng, (VALUE)&qng_params, &state);

  RB_GC_GUARD(params.block);
  RB_GC_GUARD(params.exception);

  integration_finish_callback(&params, state);

  return rb_ary_new_from_args(3, DBL2NUM(result), DBL2NUM(error),
                              SIZET2NUM(evaluations));
}
