#include "integration_internal.h"

typedef struct {
  gsl_function *function;
  double a;
  double omega;

  enum gsl_integration_qawo_enum sine;
  double epsabs;
  size_t limit;
  VALUE workspace_value;
  VALUE cycle_workspace_value;
  VALUE table_value;
  integration_workspace_use workspace_use;
  integration_workspace_use cycle_workspace_use;
  integration_qawo_table_use table_use;
  size_t cycle_workspace_native_limit;
  int cycle_workspace_limit_changed;
  double *result;
  double *error;
} integration_qawf_params;

static VALUE integration_free_qawf_resources(VALUE data) {
  integration_qawf_params *params = (integration_qawf_params *)data;

  integration_qawo_table_use_end((VALUE)&params->table_use);

  if (params->cycle_workspace_limit_changed) {
    params->cycle_workspace_use.workspace->limit =
        params->cycle_workspace_native_limit;
    params->cycle_workspace_limit_changed = 0;
  }

  integration_workspace_use_end((VALUE)&params->cycle_workspace_use);
  integration_workspace_use_end((VALUE)&params->workspace_use);

  return Qnil;
}

static VALUE integration_run_qawf(VALUE data) {
  integration_qawf_params *params = (integration_qawf_params *)data;

  integration_workspace_use_begin(params->workspace_value, &params->limit, 1,
                                  &params->workspace_use);
  integration_workspace_use_begin_named(
      params->cycle_workspace_value, &params->limit, 1,
      &params->cycle_workspace_use, "cycle_workspace");

  params->cycle_workspace_native_limit =
      params->cycle_workspace_use.workspace->limit;
  params->cycle_workspace_use.workspace->limit = params->limit;
  params->cycle_workspace_limit_changed = 1;

  integration_qawf_table_use_begin(params->table_value, params->omega,
                                   params->sine, params->limit,
                                   &params->table_use);

  (void)gsl_integration_qawf(
      params->function, params->a, params->epsabs, params->limit,
      params->workspace_use.workspace, params->cycle_workspace_use.workspace,
      params->table_use.table, params->result, params->error);

  return Qnil;
}

static VALUE integration_run_qawf_with_resources(VALUE data) {
  return rb_ensure(integration_run_qawf, data, integration_free_qawf_resources,
                   data);
}

VALUE integration_qawf(int argc, VALUE *argv, VALUE self) {
  VALUE a;
  VALUE omega;
  VALUE sine;
  VALUE options;
  VALUE epsabs_value = Qundef;
  VALUE limit_value = Qundef;
  VALUE workspace_value = Qundef;
  VALUE cycle_workspace_value = Qundef;
  VALUE table_value = Qundef;
  ID keywords[] = {rb_intern("epsabs"), rb_intern("limit"),
                   rb_intern("workspace"), rb_intern("cycle_workspace"),
                   rb_intern("table")};
  VALUE keyword_values[5];
  double epsabs;
  size_t limit = 1000;
  size_t workspace_capacity = 0;
  size_t cycle_workspace_capacity = 0;
  int limit_given;
  int workspace_given;
  int cycle_workspace_given;
  gsl_function function;
  integration_callback_params params;
  integration_qawf_params qawf_params = {0};
  double result;
  double error;
  int state = 0;

  (void)self;

  rb_scan_args(argc, argv, "3:", &a, &omega, &sine, &options);

  if (!NIL_P(options)) {
    rb_get_kwargs(options, keywords, 0, 5, keyword_values);
    epsabs_value = keyword_values[0];
    limit_value = keyword_values[1];
    workspace_value = keyword_values[2];
    cycle_workspace_value = keyword_values[3];
    table_value = keyword_values[4];
  }

  qawf_params.a = NUM2DBL(a);
  qawf_params.omega = NUM2DBL(omega);
  if (!RB_INTEGER_TYPE_P(sine)) {
    rb_raise(rb_eTypeError, "sine must be an Integer");
  }
  qawf_params.sine = (enum gsl_integration_qawo_enum)NUM2INT(sine);
  if (qawf_params.sine != GSL_INTEG_SINE &&
      qawf_params.sine != GSL_INTEG_COSINE) {
    rb_raise(rb_eArgError, "sine must be SINE or COSINE");
  }
  epsabs = epsabs_value == Qundef ? 1e-10 : NUM2DBL(epsabs_value);
  limit_given = limit_value != Qundef;
  if (limit_given) {
    limit = NUM2SIZET(limit_value);
  }

  workspace_given = workspace_value != Qundef;
  cycle_workspace_given = cycle_workspace_value != Qundef;

  if (workspace_given) {
    workspace_capacity =
        integration_workspace_capacity(workspace_value, "workspace");
  }
  if (cycle_workspace_given) {
    cycle_workspace_capacity = integration_workspace_capacity(
        cycle_workspace_value, "cycle_workspace");
  }
  if (workspace_given && cycle_workspace_given &&
      workspace_value == cycle_workspace_value) {
    rb_raise(rb_eArgError,
             "workspace and cycle_workspace must be different objects");
  }

  if (!limit_given) {
    if (workspace_given && cycle_workspace_given) {
      limit = workspace_capacity < cycle_workspace_capacity
                  ? workspace_capacity
                  : cycle_workspace_capacity;
    } else if (workspace_given) {
      limit = workspace_capacity;
    } else if (cycle_workspace_given) {
      limit = cycle_workspace_capacity;
    }
  }

  if (workspace_given && limit > workspace_capacity) {
    rb_raise(rb_eArgError, "limit exceeds workspace capacity");
  }
  if (cycle_workspace_given && limit > cycle_workspace_capacity) {
    rb_raise(rb_eArgError, "limit exceeds cycle_workspace capacity");
  }

  integration_function_init(&function, &params);

  qawf_params.function = &function;
  qawf_params.epsabs = epsabs;
  qawf_params.limit = limit;
  qawf_params.workspace_value = workspace_value;
  qawf_params.cycle_workspace_value = cycle_workspace_value;
  qawf_params.table_value = table_value;
  qawf_params.result = &result;
  qawf_params.error = &error;

  rb_protect(integration_run_qawf_with_resources, (VALUE)&qawf_params, &state);

  RB_GC_GUARD(params.block);
  RB_GC_GUARD(params.exception);
  RB_GC_GUARD(workspace_value);
  RB_GC_GUARD(cycle_workspace_value);
  RB_GC_GUARD(table_value);

  integration_finish_callback(&params, state);

  return rb_ary_new_from_args(2, DBL2NUM(result), DBL2NUM(error));
}
