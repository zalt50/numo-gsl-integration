#include "integration_internal.h"

typedef struct {
  gsl_function *function;
  double a;
  double b;
  double omega;
  enum gsl_integration_qawo_enum sine;
  double epsabs;
  double epsrel;
  size_t limit;
  gsl_integration_workspace *workspace;
  gsl_integration_qawo_table *table;
  double *result;
  double *error;
} integration_qawo_params;

static VALUE integration_free_qawo_resources(VALUE data) {
  integration_qawo_params *params = (integration_qawo_params *)data;

  if (params->table != NULL) {
    gsl_integration_qawo_table_free(params->table);
  }
  if (params->workspace != NULL) {
    gsl_integration_workspace_free(params->workspace);
  }

  return Qnil;
}

static VALUE integration_run_qawo(VALUE data) {
  integration_qawo_params *params = (integration_qawo_params *)data;

  params->table = gsl_integration_qawo_table_alloc(
      params->omega, params->b - params->a, params->sine, params->limit);
  params->workspace = gsl_integration_workspace_alloc(params->limit);

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
  integration_options parsed_options;
  double epsabs;
  double epsrel;
  size_t limit;
  gsl_function function;
  integration_callback_params params;
  integration_qawo_params qawo_params;
  double result;
  double error;
  int state = 0;

  (void)self;

  rb_scan_args(argc, argv, "4:", &a, &b, &omega, &sine, &options);

  integration_options_parse(options, &parsed_options, 0.0, 1e-8, 1000);
  epsabs = parsed_options.epsabs;
  epsrel = parsed_options.epsrel;
  limit = parsed_options.limit;

  integration_function_init(&function, &params);

  qawo_params.function = &function;
  qawo_params.a = NUM2DBL(a);
  qawo_params.b = NUM2DBL(b);
  qawo_params.omega = NUM2DBL(omega);
  qawo_params.sine = (enum gsl_integration_qawo_enum)NUM2INT(sine);
  qawo_params.epsabs = epsabs;
  qawo_params.epsrel = epsrel;
  qawo_params.limit = limit;
  qawo_params.workspace = NULL;
  qawo_params.table = NULL;
  qawo_params.result = &result;
  qawo_params.error = &error;

  rb_protect(integration_run_qawo_with_resources, (VALUE)&qawo_params, &state);

  RB_GC_GUARD(params.block);
  RB_GC_GUARD(params.exception);

  integration_finish_callback(&params, state);

  return rb_ary_new_from_args(2, DBL2NUM(result), DBL2NUM(error));
}
