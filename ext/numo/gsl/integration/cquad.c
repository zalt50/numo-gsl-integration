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
  integration_cquad_workspace_use workspace_use;
  double *result;
  double *error;
  size_t *evaluations;
} integration_cquad_params;

static VALUE integration_run_cquad(VALUE data) {
  integration_cquad_params *params = (integration_cquad_params *)data;

  integration_cquad_workspace_use_begin(params->workspace_value, &params->limit,
                                        params->limit_given,
                                        &params->workspace_use);
  (void)gsl_integration_cquad(params->function, params->a, params->b,
                              params->epsabs, params->epsrel,
                              params->workspace_use.workspace, params->result,
                              params->error, params->evaluations);

  return Qnil;
}

static VALUE integration_run_cquad_with_workspace(VALUE data) {
  integration_cquad_params *params = (integration_cquad_params *)data;

  return rb_ensure(integration_run_cquad, data,
                   integration_cquad_workspace_use_end,
                   (VALUE)&params->workspace_use);
}

VALUE integration_cquad(int argc, VALUE *argv, VALUE self) {
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
  size_t limit = 100;
  int limit_given;
  gsl_function function;
  integration_callback_params params;
  integration_cquad_params cquad_params = {0};
  double result;
  double error;
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
    if (RTEST(rb_funcall(limit_value, rb_intern("<"), 1, INT2FIX(3)))) {
      rb_raise(rb_eArgError, "limit must be at least 3");
    }
    limit = NUM2SIZET(limit_value);
  }

  integration_function_init(&function, &params);

  cquad_params.function = &function;
  cquad_params.a = NUM2DBL(a);
  cquad_params.b = NUM2DBL(b);
  cquad_params.epsabs = epsabs;
  cquad_params.epsrel = epsrel;
  cquad_params.limit = limit;
  cquad_params.limit_given = limit_given;
  cquad_params.workspace_value = workspace_value;
  cquad_params.result = &result;
  cquad_params.error = &error;
  cquad_params.evaluations = &evaluations;

  rb_protect(integration_run_cquad_with_workspace, (VALUE)&cquad_params,
             &state);

  RB_GC_GUARD(params.block);
  RB_GC_GUARD(params.exception);
  RB_GC_GUARD(workspace_value);

  integration_finish_callback(&params, state);

  return rb_ary_new_from_args(3, DBL2NUM(result), DBL2NUM(error),
                              SIZET2NUM(evaluations));
}
