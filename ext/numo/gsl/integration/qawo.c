#include "integration_internal.h"

typedef struct {
  gsl_function *function;
  VALUE workspace_value;
  double a;
  double b;
  double omega;

  enum gsl_integration_qawo_enum sine;
  double epsabs;
  double epsrel;
  size_t limit;
  int limit_given;
  gsl_integration_workspace *workspace;
  integration_workspace_use workspace_use;
  gsl_integration_qawo_table *table;
  double *result;
  double *error;
} integration_qawo_params;

static VALUE integration_free_qawo_resources(VALUE data) {
  integration_qawo_params *params = (integration_qawo_params *)data;

  if (params->table != NULL) {
    gsl_integration_qawo_table_free(params->table);
    params->table = NULL;
  }

  integration_workspace_use_end((VALUE)&params->workspace_use);

  return Qnil;
}

static VALUE integration_run_qawo(VALUE data) {
  integration_qawo_params *params = (integration_qawo_params *)data;

  integration_workspace_use_begin(params->workspace_value, &params->limit,
                                  params->limit_given, &params->workspace_use);
  params->workspace = params->workspace_use.workspace;
  params->table = gsl_integration_qawo_table_alloc(
      params->omega, params->b - params->a, params->sine, params->limit);

  (void)gsl_integration_qawo(params->function, params->a, params->epsabs,
                             params->epsrel, params->limit, params->workspace,
                             params->table, params->result, params->error);

  return Qnil;
}

static VALUE integration_run_qawo_with_resources(VALUE data) {
  return rb_ensure(integration_run_qawo, data, integration_free_qawo_resources,
                   data);
}

VALUE integration_qawo(int argc, VALUE *argv, VALUE self) {
  VALUE a;
  VALUE b;
  VALUE omega;
  VALUE sine;
  VALUE options;
  VALUE workspace_value;
  integration_options parsed_options;
  double a_double;
  double b_double;
  double omega_double;
  int sine_integer;
  double epsabs;
  double epsrel;
  size_t limit;
  int limit_given;
  gsl_function function;
  integration_callback_params params;
  integration_qawo_params qawo_params = {0};
  double result;
  double error;
  int state = 0;

  (void)self;

  rb_scan_args(argc, argv, "4:", &a, &b, &omega, &sine, &options);

  integration_options_parse_with_workspace(options, &parsed_options, 0.0, 1e-8,
                                           1000, &workspace_value,
                                           &limit_given);

  a_double = NUM2DBL(a);
  b_double = NUM2DBL(b);
  omega_double = NUM2DBL(omega);
  sine_integer = NUM2INT(sine);
  epsabs = parsed_options.epsabs;
  epsrel = parsed_options.epsrel;
  limit = parsed_options.limit;

  integration_function_init(&function, &params);

  qawo_params.function = &function;
  qawo_params.workspace_value = workspace_value;
  qawo_params.a = a_double;
  qawo_params.b = b_double;
  qawo_params.omega = omega_double;
  qawo_params.sine = (enum gsl_integration_qawo_enum)sine_integer;
  qawo_params.epsabs = epsabs;
  qawo_params.epsrel = epsrel;
  qawo_params.limit = limit;
  qawo_params.limit_given = limit_given;
  qawo_params.result = &result;
  qawo_params.error = &error;

  rb_protect(integration_run_qawo_with_resources, (VALUE)&qawo_params, &state);

  RB_GC_GUARD(workspace_value);
  RB_GC_GUARD(params.block);
  RB_GC_GUARD(params.exception);

  integration_finish_callback(&params, state);

  return rb_ary_new_from_args(2, DBL2NUM(result), DBL2NUM(error));
}
