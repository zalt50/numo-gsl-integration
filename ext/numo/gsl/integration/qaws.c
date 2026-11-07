#include "integration_internal.h"

typedef struct {
  gsl_function *function;
  VALUE workspace_value;
  VALUE table_value;
  double a;
  double b;
  double alpha;
  double beta;
  int mu;
  int nu;
  double epsabs;
  double epsrel;
  size_t limit;
  int limit_given;
  gsl_integration_workspace *workspace;
  integration_workspace_use workspace_use;
  gsl_integration_qaws_table *table;
  integration_qaws_table_use table_use;
  double *result;
  double *error;
} integration_qaws_params;

static VALUE integration_free_qaws_resources(VALUE data) {
  integration_qaws_params *params = (integration_qaws_params *)data;

  integration_qaws_table_use_end((VALUE)&params->table_use);
  integration_workspace_use_end((VALUE)&params->workspace_use);

  return Qnil;
}

static VALUE integration_run_qaws(VALUE data) {
  integration_qaws_params *params = (integration_qaws_params *)data;

  integration_workspace_use_begin(params->workspace_value, &params->limit,
                                  params->limit_given, &params->workspace_use);
  params->workspace = params->workspace_use.workspace;
  integration_qaws_table_use_begin(params->table_value, params->alpha,
                                   params->beta, params->mu, params->nu,
                                   &params->table_use);
  params->table = params->table_use.table;

  (void)gsl_integration_qaws(params->function, params->a, params->b,
                             params->table, params->epsabs, params->epsrel,
                             params->limit, params->workspace, params->result,
                             params->error);

  return Qnil;
}

static VALUE integration_run_qaws_with_resources(VALUE data) {
  return rb_ensure(integration_run_qaws, data, integration_free_qaws_resources,
                   data);
}

VALUE integration_qaws(int argc, VALUE *argv, VALUE self) {
  VALUE a;
  VALUE b;
  VALUE alpha;
  VALUE beta;
  VALUE mu;
  VALUE nu;
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
  double alpha_double;
  double beta_double;
  int mu_integer;
  int nu_integer;
  double epsabs;
  double epsrel;
  size_t limit;
  int limit_given;
  gsl_function function;
  integration_callback_params params;
  integration_qaws_params qaws_params = {0};
  double result;
  double error;
  int state = 0;

  (void)self;

  rb_scan_args(argc, argv, "6:", &a, &b, &alpha, &beta, &mu, &nu, &options);

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
  alpha_double = NUM2DBL(alpha);
  beta_double = NUM2DBL(beta);
  mu_integer = NUM2INT(mu);
  nu_integer = NUM2INT(nu);
  epsabs = epsabs_value == Qundef ? 0.0 : NUM2DBL(epsabs_value);
  epsrel = epsrel_value == Qundef ? 1e-8 : NUM2DBL(epsrel_value);
  limit = limit_value == Qundef ? 1000 : NUM2SIZET(limit_value);
  limit_given = limit_value != Qundef;

  integration_function_init(&function, &params);

  qaws_params.function = &function;
  qaws_params.workspace_value = workspace_value;
  qaws_params.table_value = table_value;
  qaws_params.a = a_double;
  qaws_params.b = b_double;
  qaws_params.alpha = alpha_double;
  qaws_params.beta = beta_double;
  qaws_params.mu = mu_integer;
  qaws_params.nu = nu_integer;
  qaws_params.epsabs = epsabs;
  qaws_params.epsrel = epsrel;
  qaws_params.limit = limit;
  qaws_params.limit_given = limit_given;
  qaws_params.result = &result;
  qaws_params.error = &error;

  rb_protect(integration_run_qaws_with_resources, (VALUE)&qaws_params, &state);

  RB_GC_GUARD(workspace_value);
  RB_GC_GUARD(table_value);
  RB_GC_GUARD(params.block);
  RB_GC_GUARD(params.exception);

  integration_finish_callback(&params, state);

  return rb_ary_new_from_args(2, DBL2NUM(result), DBL2NUM(error));
}
