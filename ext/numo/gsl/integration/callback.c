#include "integration_internal.h"

static VALUE integration_call_block(VALUE data) {
  integration_callback_params *params = (integration_callback_params *)data;
  VALUE value;

  value = rb_funcall(params->block, rb_intern("call"), 1, params->x);

  return rb_Float(value);
}

void integration_callback_params_init(integration_callback_params *params) {
  params->block = rb_block_proc();
  params->x = Qnil;
  params->exception = Qnil;
  params->state = 0;
}

double integration_callback(double x, void *data) {
  integration_callback_params *params = data;
  VALUE value;

  if (params->state != 0) {
    return 0.0;
  }

  params->x = DBL2NUM(x);
  value = rb_protect(integration_call_block, (VALUE)params, &params->state);

  if (params->state != 0) {
    params->exception = rb_errinfo();
    return 0.0;
  }

  return NUM2DBL(value);
}

void integration_raise_callback_exception(integration_callback_params *params) {
  if (params->state != 0) {
    rb_exc_raise(params->exception);
  }
}

VALUE integration_free_workspace(VALUE data) {
  gsl_integration_workspace *workspace = (gsl_integration_workspace *)data;

  gsl_integration_workspace_free(workspace);

  return Qnil;
}
