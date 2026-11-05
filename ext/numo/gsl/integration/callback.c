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
void integration_function_init(gsl_function *function,
                               integration_callback_params *params) {
  integration_callback_params_init(params);
  function->function = integration_callback;
  function->params = params;
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
void integration_finish_callback(integration_callback_params *params,
                                 int state) {
  integration_raise_callback_exception(params);
  if (state != 0) {
    rb_jump_tag(state);
  }
}
