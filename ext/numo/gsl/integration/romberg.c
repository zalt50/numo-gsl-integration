#include "integration_internal.h"

typedef struct {
  gsl_function *function;
  double a;
  double b;
  double epsabs;
  double epsrel;
  size_t limit;
  int limit_given;
  VALUE workspace_value;
  integration_romberg_workspace_use workspace_use;
  double *result;
  size_t *evaluations;
} integration_romberg_params;

static VALUE integration_run_romberg(VALUE data) {
  integration_romberg_params *params = (integration_romberg_params *)data;

  integration_romberg_workspace_use_begin(params->workspace_value,
                                          &params->limit, params->limit_given,
                                          &params->workspace_use);
  (void)gsl_integration_romberg(
      params->function, params->a, params->b, params->epsabs, params->epsrel,
      params->result, params->evaluations, params->workspace_use.workspace);

  return Qnil;
}

static VALUE integration_run_romberg_with_workspace(VALUE data) {
  integration_romberg_params *params = (integration_romberg_params *)data;

  return rb_ensure(integration_run_romberg, data,
                   integration_romberg_workspace_use_end,
                   (VALUE)&params->workspace_use);
}

VALUE integration_romberg(int argc, VALUE *argv, VALUE self) {
  VALUE a;
  VALUE b;
  VALUE options;
  VALUE epsabs_value = Qundef;
  VALUE epsrel_value = Qundef;
  VALUE limit_value = Qundef;
  VALUE workspace_value = Qundef;
  ID keywords[] = {rb_intern("epsabs"), rb_intern("epsrel"), rb_intern("limit"),
                   rb_intern("workspace")};
  VALUE keyword_values[4];
  double epsabs;
  double epsrel;
  size_t limit = 20;
  int limit_given;
  gsl_function function;
  integration_callback_params params;
  integration_romberg_params romberg_params = {0};
  double result;
  size_t evaluations;
  int state = 0;

  (void)self;

  rb_scan_args(argc, argv, "2:", &a, &b, &options);
  if (!NIL_P(options)) {
    rb_get_kwargs(options, keywords, 0, 4, keyword_values);
    epsabs_value = keyword_values[0];
    epsrel_value = keyword_values[1];
    limit_value = keyword_values[2];
    workspace_value = keyword_values[3];
  }

  epsabs = epsabs_value == Qundef ? 0.0 : NUM2DBL(epsabs_value);
  epsrel = epsrel_value == Qundef ? 1e-8 : NUM2DBL(epsrel_value);
  limit_given = limit_value != Qundef;
  if (limit_given) {
    if (!RB_INTEGER_TYPE_P(limit_value)) {
      rb_raise(rb_eTypeError, "limit must be an Integer");
    }
    if (RTEST(rb_funcall(limit_value, rb_intern("<"), 1, INT2FIX(1))) ||
        RTEST(rb_funcall(limit_value, rb_intern(">"), 1, INT2FIX(30)))) {
      rb_raise(rb_eArgError, "limit must be between 1 and 30");
    }
    limit = NUM2SIZET(limit_value);
  }

  integration_function_init(&function, &params);

  romberg_params.function = &function;
  romberg_params.a = NUM2DBL(a);
  romberg_params.b = NUM2DBL(b);
  romberg_params.epsabs = epsabs;
  romberg_params.epsrel = epsrel;
  romberg_params.limit = limit;
  romberg_params.limit_given = limit_given;
  romberg_params.workspace_value = workspace_value;
  romberg_params.result = &result;
  romberg_params.evaluations = &evaluations;

  rb_protect(integration_run_romberg_with_workspace, (VALUE)&romberg_params,
             &state);

  RB_GC_GUARD(params.block);
  RB_GC_GUARD(params.exception);
  RB_GC_GUARD(workspace_value);

  integration_finish_callback(&params, state);

  return rb_ary_new_from_args(2, DBL2NUM(result), SIZET2NUM(evaluations));
}
