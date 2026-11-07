#include "integration_internal.h"

typedef struct {
  gsl_function *function;
  VALUE workspace_value;
  VALUE table_value;
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
  integration_qawo_table_use table_use;
  double *result;
  double *error;
} integration_qawo_params;

static VALUE integration_free_qawo_resources(VALUE data) {
  integration_qawo_params *params = (integration_qawo_params *)data;

  integration_qawo_table_use_end((VALUE)&params->table_use);
  integration_workspace_use_end((VALUE)&params->workspace_use);

  return Qnil;
}

static VALUE integration_run_qawo(VALUE data) {
  integration_qawo_params *params = (integration_qawo_params *)data;

  integration_workspace_use_begin(params->workspace_value, &params->limit,
                                  params->limit_given, &params->workspace_use);
  params->workspace = params->workspace_use.workspace;
  integration_qawo_table_use_begin(params->table_value, params->omega,
                                   params->b - params->a, params->sine,
                                   params->limit, &params->table_use);
  params->table = params->table_use.table;

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
  VALUE epsabs_value = Qundef;
  VALUE epsrel_value = Qundef;
  VALUE limit_value = Qundef;
  VALUE workspace_value = Qundef;
  VALUE table_value = Qundef;
  ID keywords[] = {rb_intern("epsabs"), rb_intern("epsrel"), rb_intern("limit"),
                   rb_intern("workspace"), rb_intern("table")};
  VALUE keyword_values[5];
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

  if (!NIL_P(options)) {
    rb_get_kwargs(options, keywords, 0, 5, keyword_values);
    epsabs_value = keyword_values[0];
    epsrel_value = keyword_values[1];
    limit_value = keyword_values[2];
    workspace_value = keyword_values[3];
    table_value = keyword_values[4];
  }

  a_double = NUM2DBL(a);
  b_double = NUM2DBL(b);
  omega_double = NUM2DBL(omega);
  sine_integer = NUM2INT(sine);
  epsabs = epsabs_value == Qundef ? 0.0 : NUM2DBL(epsabs_value);
  epsrel = epsrel_value == Qundef ? 1e-8 : NUM2DBL(epsrel_value);
  limit = limit_value == Qundef ? 1000 : NUM2SIZET(limit_value);
  limit_given = limit_value != Qundef;

  integration_function_init(&function, &params);

  qawo_params.function = &function;
  qawo_params.workspace_value = workspace_value;
  qawo_params.table_value = table_value;
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
  RB_GC_GUARD(table_value);
  RB_GC_GUARD(params.block);
  RB_GC_GUARD(params.exception);

  integration_finish_callback(&params, state);

  return rb_ary_new_from_args(2, DBL2NUM(result), DBL2NUM(error));
}
