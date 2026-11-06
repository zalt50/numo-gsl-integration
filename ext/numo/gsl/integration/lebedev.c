#include "integration_internal.h"

typedef struct {
  VALUE block;
  VALUE x;
  VALUE y;
  VALUE z;
  VALUE exception;
  gsl_integration_lebedev_workspace *workspace;
  double result;
  int state;
} integration_lebedev_params;

static VALUE integration_call_lebedev_block(VALUE data) {
  integration_lebedev_params *params = (integration_lebedev_params *)data;
  VALUE arguments[3] = {params->x, params->y, params->z};
  VALUE value;

  value = rb_funcallv(params->block, rb_intern("call"), 3, arguments);
  return rb_Float(value);
}

static VALUE integration_free_lebedev_workspace(VALUE data) {
  integration_lebedev_params *params = (integration_lebedev_params *)data;

  if (params->workspace != NULL) {
    gsl_integration_lebedev_free(params->workspace);
  }

  return Qnil;
}

static VALUE integration_run_lebedev(VALUE data) {
  integration_lebedev_params *params = (integration_lebedev_params *)data;
  double result = 0.0;
  size_t i;

  for (i = 0; i < params->workspace->n; ++i) {
    VALUE value;

    params->x = DBL2NUM(params->workspace->x[i]);
    params->y = DBL2NUM(params->workspace->y[i]);
    params->z = DBL2NUM(params->workspace->z[i]);
    value = rb_protect(integration_call_lebedev_block, (VALUE)params,
                       &params->state);

    if (params->state != 0) {
      params->exception = rb_errinfo();
      break;
    }

    result += params->workspace->weights[i] * NUM2DBL(value);
  }

  params->result = 4.0 * M_PI * result;
  return Qnil;
}

static VALUE integration_run_lebedev_with_workspace(VALUE data) {
  integration_lebedev_params *params = (integration_lebedev_params *)data;

  return rb_ensure(integration_run_lebedev, data,
                   integration_free_lebedev_workspace, data);
}

VALUE integration_lebedev(VALUE self, VALUE n_value) {
  integration_lebedev_params params = {0};
  size_t n;
  int state = 0;

  (void)self;

  if (!RB_INTEGER_TYPE_P(n_value)) {
    rb_raise(rb_eTypeError, "n must be an Integer");
  }
  if (RTEST(rb_funcall(n_value, rb_intern("<="), 1, INT2FIX(0)))) {
    rb_raise(rb_eArgError, "n must be positive");
  }

  n = NUM2SIZET(n_value);
  params.block = rb_block_proc();
  params.workspace = gsl_integration_lebedev_alloc(n);
  if (params.workspace == NULL) {
    rb_memerror();
  }

  rb_protect(integration_run_lebedev_with_workspace, (VALUE)&params, &state);

  RB_GC_GUARD(params.block);
  RB_GC_GUARD(params.x);
  RB_GC_GUARD(params.y);
  RB_GC_GUARD(params.z);
  RB_GC_GUARD(params.exception);

  if (params.state != 0) {
    rb_exc_raise(params.exception);
  }
  if (state != 0) {
    rb_jump_tag(state);
  }

  return DBL2NUM(params.result);
}
